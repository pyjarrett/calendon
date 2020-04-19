"""
A Hammer context in which commands can be dispatched and executed.
"""
import argparse
import cmd
import multiprocessing
import os
import shutil
import subprocess
import sys
from typing import Optional

from context import ProjectContext
from multiplatform import root_to_executable
from parsers import *
from run import run_program


class Terminal(cmd.Cmd):
    pass


def generator_settings_for_compiler(cmake_path: str, compiler_path: Optional[str]):
    """Makes settings to give the generator for a specific compiler."""
    settings = []
    if compiler_path is not None:
        settings = [f'-DCMAKE_C_COMPILER={compiler_path}']

    # https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2015%202017.html
    if sys.platform == 'win32':
        if compiler_path is None:
            arch = 'x64'
            help_output = subprocess.check_output([cmake_path, '--help'])
            generator = None
            for line in help_output.decode().splitlines():
                if line.startswith('*'):
                    print(line)
                    generator = line[1:line.index('=')]
                    if '[arch]' in generator:
                        generator = generator.replace('[arch]', '')
                    generator = generator.strip()
                    print(f'"{generator}"')
                    break
            if generator is not None:
                settings.extend(['-G', generator, '-A', arch])
        else:
            settings.extend(['-G', 'Unix Makefiles'])

    return settings


def verify_executable_exists(ctx: ProjectContext, alias: str) -> bool:
    if not ctx.has_registered_program(alias):
        print(f'No alias exists for {alias}')
        return False
    program_path = ctx.path_for_program(alias)
    if not os.path.isfile(program_path):
        print(f'Executable path for {alias} does not exist at {program_path}')
        return False
    return True


def verify_build_dir_exists(build_dir: str) -> bool:
    if os.path.isfile(build_dir):
        print(f'Build directory {build_dir} exists as something other than a directory')
        return False

    if not os.path.isdir(build_dir):
        print(f'Build directory {build_dir} does not exist')
        return False
    return True


def verify_venv_dir_exists(venv: str) -> bool:
    if os.path.isfile(venv):
        print(f'Virtual environment {venv} exists as something other than a directory')
        return False

    if not os.path.isdir(venv):
        print(f'Virtual environment {venv} does not exist')
        return False
    return True


def cmd_clean(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    build_dir: str = ctx.build_dir()
    if not os.path.exists(build_dir):
        return 0

    if not os.path.isdir(build_dir):
        print(f'Build directory {build_dir} is not a directory')
        return 1

    if ctx.is_dry_run():
        print(f'Dry run')
        print(f'Would have removed: {build_dir}')
    else:
        print(f'Removing: {build_dir}')
        try:
            shutil.rmtree(build_dir)
        except OSError as err:
            print(f'Error removing {build_dir} {str(err)}')
    return 0


def cmd_gen(ctx: ProjectContext, args: argparse.Namespace) -> int:
    if not verify_executable_exists(ctx, 'cmake'):
        return 1

    if ctx.compiler() is not None and not verify_executable_exists(ctx, ctx.compiler()):
        return 1

    build_dir = ctx.build_dir()
    if os.path.isfile(build_dir):
        print(f'Build directory {build_dir} exists as something other than a directory')
        return 1
    if os.path.isdir(build_dir):
        if args.force:
            if ctx.is_dry_run():
                print(f'Would have removed previously existing directory {build_dir}')
            else:
                shutil.rmtree(build_dir)
        else:
            print(f'{build_dir} exists.  Use --force to wipe and recreate the build dir.')
            return 1

    if ctx.is_dry_run():
        print(f'Would have created {build_dir}')
    else:
        print(f'Creating build directory {build_dir}')
        os.mkdir(build_dir)

    cmake_path: str = ctx.path_for_program('cmake')
    cmake_args = [cmake_path, '..']
    if args.enable_ccache:
        cmake_args.append('-DKN_ENABLE_CCACHE=1')

    compiler = None
    if ctx.compiler() is not None:
        compiler = ctx.path_for_program(ctx.compiler())
    cmake_args.extend(generator_settings_for_compiler(cmake_path, compiler))

    if ctx.is_dry_run():
        print(f'Would have run {cmake_args} in {build_dir}')
        return 0
    else:
        return run_program(cmake_args, cwd=build_dir)


def cmd_build(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Build using the current project configuration."""
    if not verify_executable_exists(ctx, 'cmake'):
        return 1

    if not verify_build_dir_exists(ctx.build_dir()):
        return 1

    cmake_args = [ctx.path_for_program('cmake'), '--build', '.',
                  '--parallel', str(multiprocessing.cpu_count()),
                  '--config', ctx.build_config()]

    if ctx.is_dry_run():
        print(f'Would have run {cmake_args} in {ctx.build_dir()}')
    else:
        return run_program(cmake_args, cwd=(ctx.build_dir()))


def cmd_check(ctx: ProjectContext, args: argparse.Namespace) -> int:
    if not verify_executable_exists(ctx, 'cmake'):
        return 1

    if not verify_build_dir_exists(ctx.build_dir()):
        return 1

    check_target: str = 'check'
    if args.iterate:
        check_target += '-iterate'

    cmake_args = [ctx.path_for_program('cmake'), '--build', '.',
                  '--target', check_target,
                  '--config', ctx.build_config()]
    if ctx.is_dry_run():
        print(f'Would have run {cmake_args} in {ctx.build_dir()}')
    else:
        return run_program(cmake_args, cwd=(ctx.build_dir()))


def cmd_demo(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def cmd_run(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def cmd_env(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    json.dump(ctx.dump(), sys.stdout, indent=4)
    return 0


def cmd_register(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Registers a new program for use in the given context."""
    if os.path.isfile(args.path) or args.force:
        if ctx.is_dry_run():
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

    print(f'No program exists at {args.path}')
    return 1


def cmd_default(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def cmd_pysetup(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Creates a virtual environment for helper module installs."""
    if not verify_executable_exists(ctx, 'python3'):
        return 1

    if os.path.isfile(ctx.venv_dir()):
        print(f'Cannot create venv, {ctx.venv_dir()} is a file.')
        return 1

    if  os.path.isdir(ctx.venv_dir()) and args.clean:
        shutil.rmtree(ctx.venv_dir())

    if not os.path.isdir(ctx.venv_dir()):
        venv_args = [(ctx.path_for_program('python3')), '-m', 'venv', ctx.venv_dir()]
        venv_setup = run_program(venv_args, cwd=ctx.knell_home())
        if venv_setup != 0:
            print(f'Could not create virtual environment at {ctx.venv_dir()}')
            return venv_setup
        ctx.register_program('localpython3', os.path.join(ctx.venv_dir(), 'Scripts', root_to_executable('python')),
                             override=True)

    pip_upgrade_result = run_program(
        [(ctx.path_for_program('localpython3')), '-m', 'pip', 'install', '--upgrade', 'pip'], cwd=ctx.knell_home())
    if pip_upgrade_result != 0:
        print('Could not upgrade pip.')

    requirements_file = os.path.join(ctx.py_dir(), 'requirements.txt')
    if os.path.isfile(requirements_file):
        pip_install_args = [ctx.path_for_program('localpython3'), '-m', 'pip', 'install', '-r', requirements_file]
    else:
        required_dev_packages = ['mypy', 'pylint', 'pydocstyle', 'pycodestyle', 'bandit', 'colorama']
        pip_install_args = [ctx.path_for_program('localpython3'), '-m', 'pip', 'install']
        pip_install_args.extend(required_dev_packages)
    return run_program(pip_install_args, cwd=ctx.knell_home())


def py_files():
    return ['knell']


def cmd_pycheck(ctx: ProjectContext, args: argparse.Namespace) -> int:
    """Run all the checks on the Hammer Python source."""
    if not verify_executable_exists(ctx, 'localpython3'):
        return 1

    if not verify_venv_dir_exists(ctx.venv_dir()):
        return 1

    python_path = ctx.path_for_program('localpython3')
    checks = [['mypy'],
              ['pylint'],
              ['pycodestyle', '--max-line-length=120', '--show-source', '--statistics', '--count'],
              ['pydocstyle', '--ignore=D200,D203,D204,D212,D401'],
              ]

    for program in checks:
        cmd_line = [python_path, '-m']
        cmd_line.extend(program)
        cmd_line.extend(py_files())
        if run_program(cmd_line, cwd=ctx.py_dir()) != 0:
            return False

    return True

    return 1


def cmd_source(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def cmd_save(ctx: ProjectContext, args: argparse.Namespace) -> int:
    ctx.save()
    return 0


def cmd_load(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


COMMAND_PARSERS = {
    # Build and test
    'clean': (parser_clean, cmd_clean),
    'gen': (parser_gen, cmd_gen),
    'build': (parser_build, cmd_build),
    'check': (parser_check, cmd_check),

    # Run
    'demo': (parser_demo, cmd_demo),
    'run': (parser_run, cmd_run),

    # Environment
    'env': (parser_env, cmd_env),
    'register': (parser_register, cmd_register),
    'default': (parser_default, cmd_default),

    # Command history and automation
    'source': (parser_source, cmd_source),
    'save': (parser_save, cmd_save),
    'load': (parser_load, cmd_load),

    # Development
    'pysetup': (parser_pysetup, cmd_pysetup),
    'pycheck': (parser_pycheck, cmd_pycheck),
}


def parse_args() -> argparse.Namespace:
    """
    Parse command arguments and return a namespace for creating a project
    context.
    """
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest='command')
    for command in COMMAND_PARSERS:
        COMMAND_PARSERS[command][0](commands)
    parser_add_hammer_args(parser)

    args = parser.parse_args()
    if args.command is None:
        parser.print_help()
        sys.exit(0)

    return args


def run_interactive_loop():
    """Starts an interactive terminal."""
    print('Running interactively.')


# Hammer is runnable as a script, providing the appropriate command and options
# to use.  Running commands individually should be indistinguishable from
# using an interactive environment.
if __name__ == '__main__':
    if len(sys.argv) == 1:
        run_interactive_loop()
        sys.exit(0)

    # `args` is a namespace of all of our possible arguments, appropriate for
    # whatever command is going to be executed.  This isn't going to be a
    # suitable thing for passing around to each command though.
    args = parse_args()

    # Establish the target environment for the script.
    knell_home: str = os.environ.get('KNELL_HOME', os.getcwd())
    if args.knell_home:
        knell_home = args.knell_home

    # Build the context using the given home directory.
    ctx: ProjectContext = ProjectContext(knell_home)
    ctx = ctx.copy_with_overrides(vars(args))

    # Running in non-interactive mode.
    # Dispatch to the appropriate handling function.
    retval = COMMAND_PARSERS[args.command][1](ctx, args)

    if args.command in ['register', 'pysetup']:
        ctx.save()

    sys.exit(retval)
