"""
Support for running Crank commands as a script.
"""
import argparse
import sys
import time

from calendon.command_map import COMMANDS
from calendon.context import default_calendon_home, ProjectContext
from calendon.parsers import parser_add_top_level_args
from calendon import stats

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


def args_as_string(args: argparse.Namespace) -> str:
    keys = [key for key in args.__dict__.keys() if key != 'command']
    return " ".join([f'{key}={args.__dict__[key]}' for key in keys])


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

    start_time = time.time()
    exit_code: int = command(ctx, args)
    end_time = time.time()

    elapsed_time_ms: int = max(0, round((end_time - start_time) * 1000))

    stats.command_add_time(ctx, args.command, args_as_string(args), elapsed_time_ms)
    time_min, time_avg, time_max = stats.command_get_times(ctx, args.command, args_as_string(args))

    print(f'Command Runtime (ms): {elapsed_time_ms} vs. Avg: {time_avg:.1f} of [{time_min:.1f}, {time_max:.1f}]')

    # stats.command_stats(ctx)
    if args.command in COMMANDS_WHICH_SAVE:
        ctx.save()

    return exit_code
