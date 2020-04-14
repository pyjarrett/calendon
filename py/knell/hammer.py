"""
A Hammer context in which commands can be dispatched and executed.
"""
import argparse
import os
import sys


class Hammer:
    pass


DEFAULT_NAMES = ['compiler', 'build-config', 'build-dir', 'demo']


def parser_add_hammer_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """Adds arguments for hammer to a parser."""
    hammer = parser.add_argument_group('hammer')
    hammer.add_argument('--knell-home',
                        type=str,
                        default=os.environ['KNELL_HOME'],
                        help="Root directory of the Knell project, which will "
                             "override KNELL_HOME")
    hammer.add_argument('--config', type=str,
                        help="Specify a different configuration file to use "
                             "rather than searching the current directory and "
                             "user's home directory.")
    return parser


def parser_add_general_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """Adds general arguments to a parser."""
    general = parser.add_argument_group('general')
    general.add_argument('--dry-run',
                         action='store_true',
                         help="Show what commands would have been run.")
    general.add_argument('--verbose',
                         action='store_true',
                         help="Log more details than normal.")
    return parser


def parser_add_build_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """Adds common build arguments to a parser."""
    build = parser.add_argument_group('build')
    build.add_argument('--build-dir',
                       type=str,
                       default='build')
    build.add_argument('--build-config',
                       type=str,
                       default='Debug',
                       choices=['Debug', 'Release'],
                       help="Changes applied compiler/linker flags and "
                            "preprocessor defines.")
    build.add_argument('--compiler',
                       type=str,
                       default='default',
                       help='Use a registered program as the compiler.')
    return parser


def parser_add_driver_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    driver = parser.add_argument_group('driver')
    driver.add_argument('--ticks',
                        type=int,
                        default=0,
                        help='Specifies the number of game ticks to run the demo.')
    driver.add_argument('--runtime',
                        type=int,
                        metavar='SECONDS',
                        help='Only run the demo for a specific amount of time.')
    driver.add_argument('--headless',
                        action='store_true',
                        help='Runs the demo without the UI starting up.')
    driver.add_argument('--game',
                        type=str,
                        help='Specify the game (or demo) to load at driver startup.')
    driver.add_argument('--asset-dir',
                        type=str,
                        help='Sets the directory from which to load assets.')
    driver.add_argument('--from-replay',
                        type=str,
                        metavar='REPLAY_FILE',
                        help='Replays inputs as specified by a replay file.')
    driver.add_argument('--to-replay',
                        type=str,
                        metavar='REPLAY_FILE',
                        help='Generates a replay file of all inputs, ticks, and frame draws.')
    return parser


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers()

    clean_parser = commands.add_parser('clean', help='Remove build directories')
    parser_add_general_args(parser_add_build_args(clean_parser))

    gen_parser = commands.add_parser('gen', help='Generate build types')
    parser_add_general_args(parser_add_build_args(gen_parser))
    gen_parser.add_argument('--force',
                            action='store_true',
                            help='Delete any preexisting build directory')

    build_parser = commands.add_parser('build', help='Do a build')
    parser_add_general_args(parser_add_build_args(build_parser))
    build_parser.add_argument('--clean',
                              action='store_true',
                              help='Delete any preexisting build directory')

    check_parser = commands.add_parser('check', help='Run tests')
    parser_add_general_args(parser_add_build_args(check_parser))
    check_parser.add_argument('--iterate',
                              action='store_true',
                              help='Only rerun failed tests.')

    commands.add_parser('demo', help='List built demos available to be run')

    run_parser = commands.add_parser('run', help='Run a demo or a game.')
    run_parser.add_argument('demo')
    parser_add_general_args(parser_add_driver_args(run_parser))

    commands.add_parser('env', help="Prints the current Hammer configuration, "
                                    "environment variables and the list of "
                                    "registered programs")

    register_parser = commands.add_parser('register')
    register_parser.add_argument('alias',
                                 metavar='ALIAS',
                                 help='Short name to use for the program')
    register_parser.add_argument('path',
                                 metavar='PATH',
                                 help='Existing path for the program executable.')
    register_parser.add_argument('--force',
                                 action='store_true',
                                 help='Add, even if the path does not exist.')

    default_parser = commands.add_parser('default')
    default_parser.add_argument('name',
                                metavar='NAME',
                                choices=DEFAULT_NAMES)
    default_parser.add_argument('value', metavar='VALUE')

    pycheck_parser = commands.add_parser('pycheck',
                                         help='Run Python linting and testing.')

    source_parser = commands.add_parser('source',
                                        help="Run commands from a given file as if "
                                             "they had been typed in line by line.")
    source_parser.add_argument('file', metavar='FILE')

    save_parser = commands.add_parser('save', help="Saves the current configuration. "
                                                   "If no file is specified, it "
                                                   "will save to the configuration "
                                                   "file originally loaded.")
    save_parser.add_argument('file', metavar='FILE')

    load_parser = commands.add_parser('load', help="Loads a configuration from "
                                                   "file.  This also sets the "
                                                   "current configuration file "
                                                   "which will be saved.")
    load_parser.add_argument('file', metavar='FILE')


    parser_add_hammer_args(
        parser_add_general_args(
            parser_add_build_args(parser)))
    args = parser.parse_args()
