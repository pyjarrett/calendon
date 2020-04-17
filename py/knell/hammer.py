"""
A Hammer context in which commands can be dispatched and executed.
"""
from __future__ import annotations  # See PEP 484 and PEP 563.
import argparse
import cmd
import copy
from dataclasses import dataclass
import os
from parsers import *
import shutil
import sys
from typing import Dict, Optional


class Terminal(cmd.Cmd):
    pass


def override_flavor_from_namespace(flavor: object, kv: Dict):
    """Overrides values in a flavor if they assigned in the namespace."""
    for k in kv:
        if kv[k] is not None and hasattr(flavor, k):
            setattr(flavor, k, kv[k])


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
        self.knell_home = os.path.abspath(knell_home)
        self._script_flavor = ScriptFlavor()
        self._build_flavor = BuildFlavor()
        self._run_flavor = RunFlavor()

    def copy_with_overrides(self, kv: Dict) -> ProjectContext:
        """Creates a new context with the given overrides applied."""
        ctx = copy.deepcopy(self)
        override_flavor_from_namespace(ctx._script_flavor, kv)
        override_flavor_from_namespace(ctx._build_flavor, kv)
        override_flavor_from_namespace(ctx._run_flavor, kv)

        if self._script_flavor.verbose:
            print(f'Overrode: {self.__dict__}')
            print(f'Using:    {kv}')
            print(f'Result:   {self.__dict__}')
        return ctx

    def is_dry_run(self) -> bool:
        return self._script_flavor.dry_run

    def build_dir(self) -> str:
        return os.path.abspath(os.path.join(self.knell_home, self._build_flavor.build_dir))


# Atomic operations which produce a status and a new context.
def do_clean(ctx: ProjectContext) -> int:
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


def do_gen(ctx: ProjectContext) -> int:
    return 1


def do_build(ctx: ProjectContext) -> int:
    return 1


def do_check(ctx: ProjectContext) -> int:
    return 1


def do_demo(ctx: ProjectContext) -> int:
    return 1


def do_run(ctx: ProjectContext) -> int:
    return 1


def do_env(ctx: ProjectContext) -> int:
    return 1


def do_register(ctx: ProjectContext) -> int:
    return 1


def do_default(ctx: ProjectContext) -> int:
    return 1


def do_pycheck(ctx: ProjectContext) -> int:
    return 1


def do_source(ctx: ProjectContext) -> int:
    return 1


def do_save(ctx: ProjectContext) -> int:
    return 1


def do_load(ctx: ProjectContext) -> int:
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
    COMMAND_PARSERS[args.command][1](ctx)
