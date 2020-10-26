import argparse
import glob
import json
import multiprocessing
import os
import re
import shutil
import sys
from typing import List, Optional

from calendon.cmake import generator_settings_for_compiler
from calendon.context import ProjectContext
import calendon.multiplatform as mp
from calendon.run import run_program


def py_files():
    return ['calendon']


def _verify_executable_exists(ctx: ProjectContext, alias: Optional[str]) -> bool:
    """Return true if an alias maps to a file in a context and provide user messaging."""
    if alias is None:
        print('Cannot find an executable for a null alias.')
        return False

    if not ctx.has_registered_program(alias):
        print(f'No alias exists for {alias}')
        recommended: Optional[str] = mp.recommended_executable(mp.root_to_executable(alias))
        if recommended is not None:
            choice: str = input(f'Use recommended path for alias "{alias}": {recommended} [y/N]? ')
            if choice == 'y':
                print(f'Setting alias {alias} -> {recommended}')
                return ctx.register_program(alias, recommended)

        print('Use `crank register ALIAS PATH` to register a program for use.')
        print('Example: crank register cmake "C:/Program Files/CMake/bin/cmake.exe"')
        return False
    program_path = ctx.path_for_program(alias)
    if not os.path.isfile(program_path):
        print(f'Executable path for {alias} does not exist at {program_path}')
        return False
    return True


def _verify_build_dir_exists(build_dir: str) -> bool:
    """Return true if the build dir exists while providing user messaging."""
    if os.path.isfile(build_dir):
        print(f'Build directory {build_dir} exists as something other than a directory')
        return False

    if not os.path.isdir(build_dir):
        print(f'Build directory {build_dir} does not exist')
        return False
    return True


def _verify_venv_dir_exists(venv: str) -> bool:
    """Return true if the virtual environment dir exists while providing user messaging."""
    if os.path.isfile(venv):
        print(f'Virtual environment {venv} exists as something other than a directory')
        return False

    if not os.path.isdir(venv):
        print(f'Virtual environment {venv} does not exist')
        return False
    return True


def cmd_clean(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Clean up the build directory and build files."""
    build_dir: str = ctx.build_dir()
    if not os.path.exists(build_dir):
        return 0

    if not os.path.isdir(build_dir):
        print(f'Build directory {build_dir} is not a directory')
        return 1

    if args.dry_run:
        print('Dry run')
        print(f'Would have removed: {build_dir}')
    else:
        print(f'Removing: {build_dir}')
        try:
            shutil.rmtree(build_dir)
        except OSError as err:
            print(f'Error removing {build_dir} {str(err)}')
    return 0


def cmd_gen(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Runs the generator to make build files like Visual Studio solutions."""
    if not _verify_executable_exists(ctx, 'cmake'):
        return 1

    if ctx.compiler() is not None and not _verify_executable_exists(ctx, ctx.compiler()):
        return 1

    build_dir = ctx.build_dir()
    if os.path.isfile(build_dir):
        print(f'Build directory {build_dir} exists as something other than a directory')
        return 1
    if os.path.isdir(build_dir):
        if args.force:
            if args.dry_run:
                print(f'Would have removed previously existing directory {build_dir}')
            else:
                shutil.rmtree(build_dir)
        else:
            print(f'{build_dir} exists.  Use --force to wipe and recreate the build dir.')
            return 1

    if args.dry_run:
        print(f'Would have created {build_dir}')
    else:
        print(f'Creating build directory {build_dir}')
        os.mkdir(build_dir)

    cmake_path: str = ctx.path_for_program('cmake')
    cmake_args = [cmake_path, '..']
    if args.enable_ccache:
        cmake_args.append('-DCN_ENABLE_CCACHE=1')

    compiler = ctx.compiler()
    if compiler is not None:
        compiler = ctx.path_for_program(compiler)
    cmake_args.extend(generator_settings_for_compiler(cmake_path, compiler))

    if args.dry_run:
        print(f'Would have run {cmake_args} in {build_dir}')
        return 0

    return run_program(cmake_args, cwd=build_dir)


def cmd_build(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Build using the current project configuration."""
    if not _verify_executable_exists(ctx, 'cmake'):
        return 1

    if not _verify_build_dir_exists(ctx.build_dir()):
        return 1

    cmake_args = [ctx.path_for_program('cmake'), '--build', '.',
                  '--parallel', str(multiprocessing.cpu_count()),
                  '--config', ctx.build_config()]

    if args.dry_run:
        print(f'Would have run {cmake_args} in {ctx.build_dir()}')
        return 0

    return run_program(cmake_args, cwd=(ctx.build_dir()))


def cmd_doc(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Generate local project documentation."""
    if not _verify_executable_exists(ctx, 'sphinx-build'):
        return 1

    if args.doxygen:
        if not _verify_executable_exists(ctx, 'doxygen'):
            return 1

        doxygen_args: List[str] = [ctx.path_for_program('doxygen'),
                                   os.path.join(ctx.calendon_home(), 'Doxyfile')]
        doxygen_exit_code: int = run_program(doxygen_args, cwd=(ctx.calendon_home()))
        if doxygen_exit_code != 0:
            return doxygen_exit_code

    source_dir: str = os.path.join(ctx.sphinx_dir(), 'source')
    build_dir: str = 'build'
    sphinx_args: List[str] = [ctx.path_for_program('sphinx-build'),
                              '-M', 'html', source_dir, build_dir]
    sphinx_exit_code: int = run_program(sphinx_args, cwd=ctx.sphinx_dir())
    if sphinx_exit_code != 0:
        return sphinx_exit_code

    if args.no_open:
        return sphinx_exit_code

    index: str = os.path.join(ctx.sphinx_dir(), 'build', 'html', 'index.html')
    return mp.open_file(index)


def cmd_check(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Runs tests."""
    if not _verify_executable_exists(ctx, 'cmake'):
        return 1

    if not _verify_build_dir_exists(ctx.build_dir()):
        return 1

    check_target: str = 'check'
    if args.iterate:
        check_target += '-iterate'

    cmake_args = [ctx.path_for_program('cmake'), '--build', '.',
                  '--target', check_target,
                  '--config', ctx.build_config()]
    if args.dry_run:
        print(f'Would have run {cmake_args} in {ctx.build_dir()}')
        return 0
    else:
        return run_program(cmake_args, cwd=(ctx.build_dir()))


def cmd_demo(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Prints all currently build demos which can be run."""
    print('Demos:')
    demo_glob: str = os.path.join(ctx.demo_dir(), mp.shared_lib_glob())
    for demo_shared_lib in glob.glob(demo_glob):
        demo_name = mp.shared_lib_to_root(os.path.basename(demo_shared_lib))
        print(demo_name)
    return 0


def cmd_export(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Exports the currently built shared libraries and headers."""
    top_level_dir_name: str = 'calendon-0.0.1'
    if args.output_dir:
        export_dir = os.path.join(args.output_dir, top_level_dir_name)
    else:
        export_dir = os.path.join(ctx.export_dir(), top_level_dir_name)

    header_dir: str = os.path.join(export_dir, 'include', 'calendon')
    base_dir: str = ctx.source_dir()
    print(f'Exporting from {ctx.build_dir()} to {export_dir}')

    if os.path.isdir(export_dir):
        print(f'Removing export directory: {export_dir}')
        shutil.rmtree(export_dir)

    shutil.copytree(base_dir, header_dir, ignore=shutil.ignore_patterns('*.c', 'CMakeLists.txt'))
    print('Exported Headers:')
    print(f' - {header_dir}')

    # TODO: Allow export as other architectures other than x64.
    lib_dir = os.path.join(export_dir, 'lib', 'x64')
    os.makedirs(lib_dir, exist_ok=True)

    libraries = glob.glob(os.path.join(ctx.build_dir(), mp.shared_lib_glob()))
    libraries.extend(glob.glob(os.path.join(ctx.build_dir(), mp.static_lib_glob())))
    print('Exporting Libraries:')
    for lib in libraries:
        exported_lib: str = os.path.basename(lib)
        shutil.copyfile(lib, os.path.join(lib_dir, exported_lib))
        print(f'- {exported_lib}')

    print(f'Exported Calendon to: {export_dir}')
    return 0


def cmd_run(ctx: ProjectContext, args: argparse.Namespace) -> int:
    ovr_ctx = ctx.copy_with_overrides(vars(args))

    # Ensure an up-to-date build.
    build_status = cmd_build(ovr_ctx, args)
    if build_status != 0:
        return build_status

    ovr_ctx.set_game(os.path.join(ctx.demo_dir(), mp.root_to_shared_lib(args.demo)))
    return ovr_ctx.run_driver()


def write_demo_template(ctx: ProjectContext, name: str, dry_run: bool) -> bool:
    realname: str = sanitize_for_file_name(name)
    demo_src_dir: str = os.path.join(ctx.calendon_home(), 'src', 'demos')
    demo_source_file: str = os.path.join(demo_src_dir, realname + '.c')
    if os.path.exists(demo_source_file):
        print(f'Demo source file already exists: {demo_source_file}')
        return False

    if dry_run:
        print(f'Would have written demo to {demo_source_file}')
        return True

    with open(demo_source_file, 'w') as file:
        file.write(f'''#include <calendon/cn.h>
#include <calendon/log.h>

CnLogHandle LogSysSample;

CN_GAME_API bool Game_Init(void)
{{
    cnLog_RegisterSystem(&LogSysSample, "{realname}", CnLogVerbosityTrace);
    CN_TRACE(LogSysSample, "{realname} loaded");
    return true;
}}

CN_GAME_API void Game_Draw(void)
{{
}}

CN_GAME_API void Game_Tick(uint64_t dt)
{{
    CN_UNUSED(dt);
}}

CN_GAME_API void Game_Shutdown(void)
{{
}}
''')
    return True


def sanitize_for_file_name(name: str) -> str:
    return re.sub(r'[ \t\r\n%:<>{}\[\]"/\\?*-]', '', name.lower())


def cmd_new(ctx: ProjectContext, args: argparse.Namespace) -> int:
    # Some sort of template mapping between types and outputs.
    # Some templates might eventually generate multiple output files.
    # Types to eventually support:
    # test -> src/tests/unit/test-{name}.c
    # plugin -> src/plugins/{name}/{name}.c
    # demo -> src/demos/{name}/demo-{name}.c
    if args.type == 'demo':
        if not write_demo_template(ctx, args.name, args.dry_run):
            print('Unable to create content')
            return 1
    else:
        print(f'Unrecognized template name {args.type}')
        return 1

    print('Regenerate the project to use your new demo')
    return 0


def cmd_env(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Prints the current Crank environment."""
    json.dump(ctx.dump(), sys.stdout, indent=4)
    print()
    return 0


def cmd_register(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Registers a new program for use in the given context."""
    if os.path.isfile(args.path) or args.force:
        if args.dry_run:
            if ctx.has_registered_program(args.alias) and not args.override:
                print(f'Trying to override {ctx.path_for_program(args.alias)} of {args.alias} with {args.path}')
                return 1
            else:
                print(f'Would have added alias {args.alias} -> {args.path}')
                return 0
        else:
            if ctx.register_program(args.alias, args.path, args.override):
                return 0
            return 1

    print(f'No program exists at {args.path}.')
    return 1


def cmd_default(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Sets a default compiler, build-dir, build-config, game, or asset-dir."""
    return ctx.set_default(args.name, args.value)


def cmd_reset(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Reverts a defaulted compiler, build-dir, build-config, game, or asset-dir."""
    return ctx.clear_default(args.name)


def cmd_pysetup(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Creates a virtual environment for helper module installs."""
    if not _verify_executable_exists(ctx, 'python3'):
        return 1

    if os.path.isfile(ctx.venv_dir()):
        print(f'Cannot create venv, {ctx.venv_dir()} is a file.')
        return 1

    if os.path.isdir(ctx.venv_dir()) and args.clean:
        shutil.rmtree(ctx.venv_dir())

    if not os.path.isdir(ctx.venv_dir()):
        venv_args = [(ctx.path_for_program('python3')), '-m', 'venv', ctx.venv_dir()]
        venv_setup = run_program(venv_args, cwd=ctx.calendon_home())
        if venv_setup != 0:
            print(f'Could not create virtual environment at {ctx.venv_dir()}')
            return venv_setup
        ctx.register_program('localpython3', os.path.join(ctx.venv_bin_dir(),
                                                          mp.root_to_executable('python')),
                             override=True)

    pip_upgrade_result = run_program(
        [(ctx.path_for_program('localpython3')), '-m', 'pip', 'install',
         '--upgrade', 'pip', 'setuptools', 'wheel'], cwd=ctx.calendon_home())
    if pip_upgrade_result != 0:
        print('Could not upgrade pip.')

    requirements_file = os.path.join(ctx.py_dir(), 'requirements.txt')
    if os.path.isfile(requirements_file):
        pip_install_args = [ctx.path_for_program('localpython3'), '-m', 'pip', 'install', '-r', requirements_file]
    else:
        required_dev_packages = ['mypy', 'pylint', 'pydocstyle', 'pycodestyle', 'bandit', 'colorama']
        required_dev_packages.extend(['sphinx', 'sphinx_rtd_theme', 'breathe'])
        pip_install_args = [ctx.path_for_program('localpython3'), '-m', 'pip', 'install']
        pip_install_args.extend(required_dev_packages)

    sphinx_build_path: str = os.path.join(ctx.venv_bin_dir(), mp.root_to_executable('sphinx-build'))
    ctx.register_program('sphinx-build', sphinx_build_path, override=True)
    return run_program(pip_install_args, cwd=ctx.calendon_home())


def cmd_pycheck(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Run all the checks on Python source."""
    if not _verify_executable_exists(ctx, 'localpython3'):
        return 1

    if not _verify_venv_dir_exists(ctx.venv_dir()):
        return 1

    python_path = ctx.path_for_program('localpython3')
    checks = [['mypy'],
              ['pylint'],
              ['pycodestyle', '--max-line-length=120', '--show-source',
               '--statistics', '--count', '--ignore=E731'],
              ['pydocstyle', '--ignore=D100,D102,D103,D104,D200,D203,D204,D212,D401'],
              ]

    failures = 0
    for program in checks:
        cmd_line = [python_path, '-m']
        cmd_line.extend(program)
        cmd_line.extend(py_files())
        if run_program(cmd_line, cwd=ctx.py_dir()) != 0:
            failures += 1
            if args.incremental:
                return failures

    return failures


def cmd_source(_ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Does nothing.  Will run a script of commands at some point."""
    return 1


def cmd_save(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Saves the current Crank context to file."""
    ctx.save()
    return 0


def cmd_load(_ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Does nothing.  Will load the context from file."""
    return 1
