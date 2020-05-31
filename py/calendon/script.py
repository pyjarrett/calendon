"""
Support for running Crank commands as a script.
"""
import argparse
import sys

from calendon.command_map import COMMANDS
from calendon.context import default_calendon_home, ProjectContext
from calendon.parsers import parser_add_top_level_args

# Commands which should save settings when run non-interactively.
COMMANDS_WHICH_SAVE = ['register', 'pysetup', 'default', 'reset']


def parse_args() -> argparse.Namespace:
    """
    Parse command arguments and return a namespace for creating a project context.
    """
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest='command')
    for command in COMMANDS:
        subparser = commands.add_parser(COMMANDS[command].name, help=COMMANDS[command].help)
        parser_builder = COMMANDS[command].parser
        parser_builder(subparser)
    parser_add_top_level_args(parser)

    args = parser.parse_args()
    if args.command is None:
        parser.print_help()
        sys.exit(0)

    return args


def run_as_script():
    """Runs Crank as a simple command using the current command line."""
    args = parse_args()

    # Establish the target environment for the script.
    calendon_home: str = default_calendon_home()
    if args.calendon_home:
        calendon_home = args.calendon_home

    ctx: ProjectContext = ProjectContext(calendon_home)
    ctx = ctx.copy_with_overrides(vars(args))

    # Dispatch to the appropriate handling function.
    command = COMMANDS[args.command].command
    exit_code: int = command(ctx, args)
    if args.command in COMMANDS_WHICH_SAVE:
        ctx.save()

    return exit_code
