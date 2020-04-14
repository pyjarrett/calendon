"""
A Hammer context in which commands can be dispatched and executed.
"""
import argparse
import os
import sys


class Hammer:
    pass


def parser_add_hammer_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """Adds arguments for hammer to a parser."""
    hammer = parser.add_argument_group('hammer')
    hammer.add_argument('--knell-home',
                        type=str,
                        default=os.environ['KNELL_HOME'],
                        help="Root directory of the Knell project, which will"
                             "override KNELL_HOME")
    hammer.add_argument('--config', type=str,
                        help="Specify a different configuration file to use"
                             "rather than searching the current directory and"
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
                        help="Changes applied compiler/linker flags and"
                             "preprocessor defines.")
    build.add_argument('--compiler',
                        type=str,
                        default='default',
                        help='Use a registered program as the compiler.')
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

    parser_add_hammer_args(
        parser_add_general_args(
            parser_add_build_args(parser)))
    args = parser.parse_args()
