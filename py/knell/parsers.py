"""
Parsers for various commands.
"""
import argparse

DEFAULT_NAMES = ['compiler', 'build-config', 'build-dir', 'game', 'asset-dir']


def parser_add_top_level_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """
    Adds arguments for the helper tool itself to a parser.

    Adds properties `knell_home` and `config`.
    """
    top_level = parser.add_argument_group('top-level')
    top_level.add_argument('--knell-home',
                           type=str,
                           help="Root directory of the Knell project, which will "
                                "override KNELL_HOME")
    top_level.add_argument('--config', type=str,
                           help="Specify a different configuration file to use "
                                "rather than searching the current directory and "
                                "user's home directory.")
    return parser


def parser_add_general_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """
    Adds general arguments to a parser.

    Adds properties `dry_run` and `verbose`.
    """
    general = parser.add_argument_group('general')
    general.add_argument('--dry-run',
                         action='store_true',
                         help="Show what commands would have been run.")
    general.add_argument('--verbose',
                         action='store_true',
                         help="Log more details than normal.")
    return parser


def parser_add_build_dir(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser.add_argument('--build-dir',
                        type=str)
    return parser


def parser_add_build_config(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser.add_argument('--build-config',
                        type=str,
                        choices=['Debug', 'Release'],
                        help="Changes applied compiler/linker flags and "
                             "preprocessor defines.")
    return parser


def parser_add_driver_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """
    Adds arguments for running the driver.

    Adds properties `ticks`, `runtime`, `headless`, `game`, `asset_dir`,
    `from_replay`, and `to_replay`.
    """
    driver = parser.add_argument_group('driver')
    driver.add_argument('--ticks',
                        type=int,
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


def parser_clean(parser) -> argparse.ArgumentParser:
    return parser_add_general_args(parser_add_build_dir(parser))


def parser_gen(parser) -> argparse.ArgumentParser:
    parser_add_general_args(parser_add_build_dir(parser))
    parser.add_argument('--compiler',
                        type=str,
                        help='Use a registered program as the compiler.')
    parser.add_argument('--force',
                        action='store_true',
                        help='Delete any preexisting build directory')
    parser.add_argument('--enable-ccache',
                        action='store_true',
                        help='Use ccache (if available)')
    return parser


def parser_build(parser) -> argparse.ArgumentParser:
    parser_add_general_args(parser)
    parser_add_build_config(parser)
    parser_add_build_dir(parser)
    return parser


def parser_doc(parser) -> argparse.ArgumentParser:
    return parser


def parser_check(parser) -> argparse.ArgumentParser:
    parser_add_general_args(parser)
    parser_add_build_config(parser)
    parser_add_build_dir(parser)
    parser.add_argument('--iterate',
                        action='store_true',
                        help='Only rerun failed tests.')
    return parser


def parser_demo(parser) -> argparse.ArgumentParser:
    return parser


def parser_run(parser) -> argparse.ArgumentParser:
    parser.add_argument('demo')
    parser_add_general_args(parser_add_driver_args(parser))
    return parser


def parser_new(parser) -> argparse.ArgumentParser:
    parser_add_general_args(parser)
    parser.add_argument('type',
                        choices=['test', 'demo'],
                        help='Creates new content from a template.')
    parser.add_argument('name')
    return parser


def parser_env(parser) -> argparse.ArgumentParser:
    return parser


def parser_register(parser) -> argparse.ArgumentParser:
    parser_add_general_args(parser)
    parser.add_argument('alias',
                        metavar='ALIAS',
                        help='Short name to use for the program')
    parser.add_argument('path',
                        metavar='PATH',
                        help='Existing path for the program executable.')
    parser.add_argument('--force',
                        action='store_true',
                        help='Add, even if the path does not exist.')
    parser.add_argument('--override',
                        action='store_true',
                        help='Override an existing alias.')
    return parser


def parser_default(parser) -> argparse.ArgumentParser:
    parser.add_argument('name',
                        metavar='NAME',
                        choices=DEFAULT_NAMES)
    parser.add_argument('value', metavar='VALUE')
    return parser

def parser_reset(parser) -> argparse.ArgumentParser:
    parser.add_argument('name')
    return parser

def parser_pysetup(parser) -> argparse.ArgumentParser:
    parser.add_argument('--clean',
                        action='store_true',
                        help='Remove any existing virtual environment.')
    return parser


def parser_pycheck(parser) -> argparse.ArgumentParser:
    parser.add_argument('--incremental',
                        action='store_true',
                        help='Stop on first failure in linting and testing.')
    return parser


def parser_source(parser) -> argparse.ArgumentParser:
    parser.add_argument('file', metavar='FILE')
    return parser


def parser_save(parser) -> argparse.ArgumentParser:
    parser.add_argument('--file', metavar='FILE')
    return parser


def parser_load(parser) -> argparse.ArgumentParser:
    parser.add_argument('file', metavar='FILE')
    return parser
