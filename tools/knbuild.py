#!/usr/bin/env python3
#
# Helper cross-platform script for building Knell.
#
# Assists with the creation of out-of-tree builds across multiple platforms.
# 
import argparse
import logging
import os
import shutil
import subprocess
import sys


# TODO: Should support building for clang, gcc and MSVC, wherever each one is available.
def supported_compilers():
    return ['default', 'clang', 'gcc']


def build_dir_for_compiler(compiler):
    if compiler == 'default':
        return 'build'
    else:
        return f'build-{compiler}'


def cmake_compiler_settings(compiler):
    return [f'-DCMAKE_C_COMPILER={compiler}']


def cmd_gen(args):
    compiler = args.compiler
    build_dir = build_dir_for_compiler(compiler)
    if os.path.exists(build_dir):
        if not os.path.isdir(build_dir):
            logging.error(f'Build directory {build_dir} exists as something other than a directory')
            sys.exit(1)
        elif args.force:
            logging.info(f'Wiping the build directory {build_dir}')
            shutil.rmtree(build_dir)
        else:
            logging.info(f'{build_dir} exists.  Rerun with --force to remove the existing directory')
            sys.exit(0)

    logging.info(f'Creating build directory {build_dir}')
    os.mkdir(build_dir)
    cmake_args = ['cmake', '..']
    if compiler is not None:
        cmake_args.extend(cmake_compiler_settings(compiler))
    print(cmake_args)
    subprocess.check_call(cmake_args, cwd=build_dir)


def main():
    logging.getLogger().setLevel(logging.DEBUG)

    parser = argparse.ArgumentParser(description='Knell build tool')
    parser.add_argument('--compiler', type=str, default='default', choices=supported_compilers())

    subparser = parser.add_subparsers()
    subparser.required = True

    parser_gen = subparser.add_parser('gen', help='gen help')
    parser_gen.add_argument('--force', help='Remove current build directory if one exists')
    parser_gen.set_defaults(func=cmd_gen)

    args = parser.parse_args()
    args.func(args)



if __name__ == '__main__':
    main()
