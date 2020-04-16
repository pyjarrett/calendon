"""
A Hammer context in which commands can be dispatched and executed.
"""
import argparse
from parsers import *
import sys


class Hammer:
    pass


class ProjectContext:
    pass


def do_clean(ctx: ProjectContext) -> int:
    pass


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
    'clean': (parser_clean, do_clean),
    'gen': (parser_gen, do_gen),
    'build': (parser_build, do_build),
    'check': (parser_check, do_check),
    'demo': (parser_demo, do_demo),
    'run': (parser_run, do_run),
    'env': (parser_env, do_env),
    'register': (parser_register, do_register),
    'default': (parser_default, do_default),
    'pycheck': (parser_pycheck, do_pycheck),
    'source': (parser_source, do_source),
    'save': (parser_save, do_save),
    'load': (parser_load, do_load)
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


if __name__ == '__main__':
    args = parse_args()
    print('COMMAND: ' + args.command)
    print(args)
    COMMAND_PARSERS[args.command][1](args)
