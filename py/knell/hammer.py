"""
A Hammer context in which commands can be dispatched and executed.
"""
from __future__ import annotations  # See PEP 484 and PEP 563.
import argparse
import cmd
import copy
from dataclasses import dataclass
import dataclasses
import json
import multiprocessing
import os
from parsers import *
import queue
import shutil
import subprocess
import sys
import threading
from typing import Dict, IO, List, Optional


class Terminal(cmd.Cmd):
    pass


def read_stream(stream: IO, queued_lines: queue.Queue):
    """Reads a stream into a queue."""
    for line in stream:
        try:
            queued_lines.put(line.decode().strip())
        except UnicodeDecodeError:
            queued_lines.put(line.strip())


def run_program(command_line: List[str], **kwargs):
    """Runs another process while streaming its stdout and stderr."""
    print(f'Running: {" ".join(command_line)} {kwargs}')
    process = subprocess.Popen(command_line,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               **kwargs)

    out_queue: queue.Queue = queue.Queue()
    err_queue: queue.Queue = queue.Queue()

    out_thread = threading.Thread(target=read_stream, args=(process.stdout, out_queue))
    err_thread = threading.Thread(target=read_stream, args=(process.stderr, err_queue))

    out_thread.start()
    err_thread.start()

    while out_thread.is_alive() or err_thread.is_alive() or not out_queue.empty() or not err_queue.empty():
        try:
            line = out_queue.get_nowait()
            print(line)
        except queue.Empty:
            pass

        try:
            line = err_queue.get_nowait()
            print(f'err:{line}')
        except queue.Empty:
            pass

    out_thread.join()
    err_thread.join()

    return process.wait()


def override_flavor_from_dict(flavor: object, kv: Dict):
    """Overrides values in a flavor if they assigned in the namespace."""
    for k in kv:
        if kv[k] is not None and hasattr(flavor, k):
            setattr(flavor, k, kv[k])


def generator_settings_for_compiler(cmake_path: str, compiler_path: Optional[str]):
    """Makes settings to give the generator for a specific compiler."""
    settings = []
    if compiler_path is not None:
        settings = [f'-DCMAKE_C_COMPILER="{compiler_path}"']

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


@dataclass
class ScriptFlavor:
    knell_home: str = None
    verbose: bool = False
    dry_run: bool = False


@dataclass
class BuildFlavor:
    """Define enough information to do a build."""
    build_dir: str = 'build'
    build_config: str = 'Debug'
    compiler: str = None


@dataclass
class RunFlavor:
    """Environment specification of how to do a 'run'."""
    game: str = None
    asset_dir: str = None
    ticks: Optional[int] = 0
    run_time_seconds: Optional[int] = 0
    headless: bool = False


class ProjectContext:
    """A concise description of the environment in which the script will run."""

    def __init__(self, knell_home: str):
        """Creates a default context from a home directory."""
        self._script_flavor = ScriptFlavor()
        self._script_flavor.knell_home = os.path.abspath(knell_home)
        self._build_flavor = BuildFlavor()
        self._run_flavor = RunFlavor()
        self._registered_programs = {}
        self._load_config(os.path.join(self.knell_home(), '.hammer'))

    def _override_from_dict(self, kv: Dict):
        override_flavor_from_dict(self._script_flavor, kv)
        override_flavor_from_dict(self._build_flavor, kv)
        override_flavor_from_dict(self._run_flavor, kv)

    def _load_config(self, config_path: str):
        """Loads a config from a path."""
        if not os.path.isfile(config_path):
            print(f'No config file found at {config_path}')
            return

        with open(config_path, 'r') as file:
            kv = json.load(file)
            self._registered_programs = kv.get('registered_programs', {})
            self._override_from_dict(kv)

    def _save_config(self, config_path: str):
        with open(config_path, 'w') as file:
            combined = {}
            for flavor in [self._script_flavor, self._build_flavor, self._run_flavor]:
                for field in dataclasses.fields(flavor):
                    combined[field.name] = getattr(flavor, field.name)
            combined['registered_programs'] = self._registered_programs

            if self.is_verbose():
                print(f'Saving to {config_path}')
                print(combined)
            json.dump(combined, file, indent=4)

    def save(self):
        self._save_config(os.path.join(self.knell_home(), '.hammer'))

    def copy_with_overrides(self, kv: Dict) -> ProjectContext:
        """Creates a new context with the given overrides applied."""
        ctx = copy.deepcopy(self)
        ctx._override_from_dict(kv)

        if self._script_flavor.verbose:
            print(f'Overrode: {self.__dict__}')
            print(f'Using:    {kv}')
            print(f'Result:   {self.__dict__}')
        return ctx

    def is_dry_run(self) -> bool:
        return self._script_flavor.dry_run

    def is_verbose(self) -> bool:
        return self._script_flavor.verbose

    def knell_home(self) -> str:
        """Project root directory."""
        return self._script_flavor.knell_home

    def build_dir(self) -> str:
        return os.path.abspath(os.path.join(self._script_flavor.knell_home, self._build_flavor.build_dir))

    def build_config(self) -> str:
        return self._build_flavor.build_config

    def compiler(self) -> Optional[str]:
        return self._build_flavor.compiler

    def register_program(self, alias: str, path: str, override: bool = False) -> bool:
        if alias in self._registered_programs and not override:
            existing: str = self._registered_programs[alias]
            print(f'Trying to override {existing} of {alias} with {path}')
            return False

        self._registered_programs[alias] = path
        return True

    def has_registered_program(self, alias: str) -> bool:
        return alias in self._registered_programs

    def path_for_program(self, alias: str) -> str:
        if not self.has_registered_program(alias):
            raise ValueError(f'No registered path for {alias}')
        return self._registered_programs[alias]


# Atomic operations which produce a status and a new context.
def do_clean(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    build_dir: str = ctx.build_dir()
    if not os.path.exists(build_dir):
        print(f'Build directory {build_dir} does not exist')
        return 1

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


def do_gen(ctx: ProjectContext, args: argparse.Namespace) -> int:
    if not ctx.has_registered_program('cmake'):
        print('No alias exists for `cmake`')
        return 1

    cmake_path: str = ctx.path_for_program('cmake')
    if not os.path.isfile(cmake_path):
        print(f'CMake does not exist at {cmake_path}')

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

    print(f'Creating build directory {build_dir}')
    if ctx.is_dry_run():
        print(f'Would have created {build_dir}')
    else:
        os.mkdir(build_dir)
    cmake_args = [cmake_path, '..']

    if args.enable_ccache:
        cmake_args.append('-DKN_ENABLE_CCACHE=1')

    compiler_path: Optional[str] = None
    if ctx.compiler():
        if not ctx.has_registered_program(ctx.compiler()):
            print(f'Compiler alias "{ctx.compiler()}" is not registered.')
            return 1
        compiler_path = ctx.path_for_program(ctx.compiler())
    cmake_args.extend(generator_settings_for_compiler(cmake_path, compiler_path))

    if ctx.is_dry_run():
        print(f'Would have run {cmake_args} in {build_dir}')
        return 0
    else:
        return run_program(cmake_args, cwd=build_dir)


def do_build(ctx: ProjectContext, _args: argparse.Namespace) -> int:
    """Build using the current project configuration."""
    build_dir = ctx.build_dir()
    if not os.path.isdir(build_dir):
        print(f'Build dir does not exist at {build_dir}')
        return 1

    if not ctx.has_registered_program('cmake'):
        print('No alias exists for `cmake`')
        return 1

    cmake_path: str = ctx.path_for_program('cmake')
    if not os.path.isfile(cmake_path):
        print(f'CMake does not exist at {cmake_path}')
    cmake_args = [cmake_path, '--build', '.',
                  '--parallel', str(multiprocessing.cpu_count()),
                  '--config', ctx.build_config()]

    if ctx.compiler():
        if not ctx.has_registered_program(ctx.compiler()):
            print(f'Compiler alias "{ctx.compiler()}" is not registered.')
            return 1
        cmake_args.append(f'-DCMAKE_C_COMPILER={ctx.path_for_program(ctx.compiler())}')

    if ctx.is_dry_run():
        print(f'Would have run {cmake_args} in {build_dir}')
    else:
        return run_program(cmake_args, cwd=build_dir)


def do_check(ctx: ProjectContext, args: argparse.Namespace) -> int:
    if not ctx.has_registered_program('cmake'):
        print('No alias exists for `cmake`')
        return 1

    cmake_path: str = ctx.path_for_program('cmake')
    if not os.path.isfile(cmake_path):
        print(f'CMake does not exist at {cmake_path}')

    build_dir = ctx.build_dir()
    if not os.path.isdir(build_dir):
        print(f'Build dir does not exist at {build_dir}')
        return 1

    check_target: str = 'check'
    if args.iterate:
        check_target += '-iterate'

    cmake_args = [cmake_path, '--build', '.',
                  '--target', check_target,
                  '--config', ctx.build_config()]
    if ctx.is_dry_run():
        print(f'Would have run {cmake_args} in {build_dir}')
    else:
        return run_program(cmake_args, cwd=build_dir)


def do_demo(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def do_run(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def do_env(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def do_register(ctx: ProjectContext, args: argparse.Namespace) -> int:
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


def do_default(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def do_pycheck(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def do_source(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


def do_save(ctx: ProjectContext, args: argparse.Namespace) -> int:
    ctx.save()
    return 0


def do_load(ctx: ProjectContext, args: argparse.Namespace) -> int:
    return 1


COMMAND_PARSERS = {
    # Build and test
    'clean': (parser_clean, do_clean),
    'gen': (parser_gen, do_gen),
    'build': (parser_build, do_build),
    'check': (parser_check, do_check),

    # Run
    'demo': (parser_demo, do_demo),
    'run': (parser_run, do_run),

    # Environment
    'env': (parser_env, do_env),
    'register': (parser_register, do_register),
    'default': (parser_default, do_default),

    # Command history and automation
    'source': (parser_source, do_source),
    'save': (parser_save, do_save),
    'load': (parser_load, do_load),

    # Development
    'pycheck': (parser_pycheck, do_pycheck),
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
    COMMAND_PARSERS[args.command][1](ctx, args)

    if args.command in ['register']:
        ctx.save()
