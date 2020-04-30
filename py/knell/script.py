"""
Support for running Crank commands as a script.
"""
import argparse
import sys

from knell.command_map import COMMANDS
from knell.context import default_knell_home, ProjectContext
from knell.parsers import parser_add_top_level_args

# Commands which should save settings when run non-interactively.
COMMANDS_WHICH_SAVE = ['register', 'pysetup']


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
    knell_home: str = default_knell_home()
    if args.knell_home:
        knell_home = args.knell_home

    ctx: ProjectContext = ProjectContext(knell_home)
    ctx = ctx.copy_with_overrides(vars(args))

    # Dispatch to the appropriate handling function.
    command = COMMANDS[args.command].command
    exit_code: int = command(ctx, args)
    if args.command in COMMANDS_WHICH_SAVE:
        ctx.save()

    return exit_code
