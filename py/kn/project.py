"""
Module to deal with project-related options and configurations.
"""
import argparse
import contextlib
import copy
import json
import os
import shlex
from typing import Optional

import kn.multiplatform as mp


def add_build_args(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    """Adds argument parsing of build-specific arguments to a parser."""
    usage = """\nfor build overrides
--build-dir DIR
--build-config Debug|Release
--compiler COMPILER_ALIAS"""
    if parser.usage is None:
        parser.usage = usage
    else:
        parser.usage = parser.usage + usage
    parser.add_argument('--build-dir', type=str, default=None)
    parser.add_argument('--build-config', type=str, default=None)
    parser.add_argument('--compiler', type=str, default=None)
    return parser


class BuildAndRunContext:
    """A description of the current build and run environment."""
    def __init__(self):
        """Create an empty config."""
        self.config = self._empty_config()

    @staticmethod
    def _empty_config():
        return {'compilers': {}, 'registered-programs': {}}

    def save(self):
        """Save current configuration."""
        print(f'Saving to {self.save_path()}')
        with open(self.save_path(), 'w') as file:
            json.dump(self.config, file)

    def load(self):
        """Load configuration from file."""
        if os.path.isfile(self.save_path()):
            print(f'Loading from {self.save_path()}')
            with open(self.save_path(), 'r') as file:
                self.config = json.load(file)
                if 'compilers' not in self.config:
                    self.config['compilers'] = {}
                if 'registered-programs' not in self.config:
                    self.config['registered-programs'] = {}

        print(f'Config file {self.save_path()} does not exist.')

    def save_path(self):
        """The currently set configuration save path."""
        return self.config.get('save-path', '.hammer')

    def values(self):
        """A copy of all configuration values."""
        return self.config

    def driver_executable(self):
        """Absolute path to the Knell driver executable."""
        return os.path.join(self.build_dir(), 'src', 'driver', mp.root_to_executable('knell-driver'))

    def set_home_dir(self, home):
        """Sets the Knell home directory, which all other paths are relative to."""
        self.config['knell-home'] = home

    def home_dir(self):
        """The specified root directory for the Knell project."""
        return self.config.get('knell-home', os.environ.get('KNELL_HOME'))

    def lib_path(self):
        """Path to the Knell lib itself."""
        return os.path.join(self.build_dir(), 'src', 'knell', mp.root_to_shared_lib('knell'))

    def current_demo_path(self):
        """Absolute path to current the demo."""
        return os.path.join(self.demo_dir(), mp.root_to_shared_lib(self.demo()))

    def demo_dir(self):
        """Absolute path to directory containing demos."""
        return os.path.join(self.build_dir(), 'src', 'demos')

    def set_build_dir(self, build_dir: str):
        """Sets a specific build directory to prevent from inferring it."""
        self.config['build-dir'] = build_dir

    def build_dir(self):
        """The location of the out-of-tree build."""
        build_dir = self.config.get('build-dir')

        if build_dir is None:
            compiler = self.config.get('compiler')
            build_dir = 'build'
            if compiler is not None and compiler != 'default':
                build_dir = f'build-{compiler}'

            build_dir += '-' + self.build_config()
        return os.path.abspath(os.path.join(self.home_dir(), build_dir))

    def build_config(self):
        """A particular version of the build, such as Debug, or Release."""
        return self.config.get('config', 'Debug')

    def register_program(self, alias, path, force=False) -> bool:
        """Adds programs to a list of "known good" programs which should be allowed to be run."""
        if os.path.isfile(path) or force:
            self.config['registered-programs'][alias] = path
            return True
        return False

    def is_registered_program(self, program_name: str) -> bool:
        return program_name in self.config['registered-programs'].keys()

    def has_default_compiler(self):
        """Check to see if CMake's compiler choice been overridden."""
        return self.config.get('compiler') is None and self.config.get('compiler') != 'default'

    def compiler(self):
        """The type of the compiler, independent of the path."""
        return self.config.get('compiler')

    def compiler_alias(self) -> Optional[str]:
        """The pseudoname for the compiler, independent of the path."""
        return self.config.get('compiler')

    def set_compiler_alias(self, alias) -> bool:
        """Sets which registered program alias it should run."""
        if alias in self.config['compilers'].keys():
            return False

        if not self.is_registered_program(alias):
            return False

        self.config['compiler'] = alias
        return True

    def set_build_config(self, config: str) -> bool:
        """Sets the build config version to use, such as Debug or Release."""
        if config not in ['Debug', 'Release']:
            return False
        self.config['config'] = config
        return True

    def compiler_path(self):
        """Return the current compiler path or None if none set."""
        compiler_alias = self.config.get('compiler')
        if compiler_alias is None:
            return None

        if compiler_alias in self.config['registered-programs'].keys():
            return self.config['registered-programs'][compiler_alias]

        return None

    def demo(self):
        """The generic name of the current demo without a prefix or suffix."""
        return self.config.get('demo')

    def add(self, args):
        """Add to the environment settings."""
        parser = argparse.ArgumentParser(usage='key value\n    Sets a key equal to a value.\n')
        parser.add_argument('key', choices=['compiler'])
        parser.add_argument('alias')
        parser.add_argument('path')

        try:
            args = parser.parse_args(shlex.split(args))
            if args.key == 'compiler':
                if os.path.isfile(args.path):
                    self.config['compilers'][args.alias] = args.path
                    return 0

                print(f'Compiler "{args.alias}" does not exist at {args.path}')
            return 1
        except SystemExit:
            return 1

    def parse_config_value(self, args):
        """Parse a config key value pair and set it."""
        parser = argparse.ArgumentParser(usage='key value\n    Sets a key equal to a value.\n')
        parser.add_argument('key', choices=['compiler', 'config', 'demo'])
        parser.add_argument('value')

        try:
            args = parser.parse_args(shlex.split(args))
            if args.key == 'compiler' and args.value not in self.config['compilers'].keys():
                print(f'Unknown compiler: {args.key}.  Add compiler aliases first.')
                return 1

            self.config[args.key] = args.value
            return 0
        except SystemExit:
            return 1


@contextlib.contextmanager
def parse_build_context_with_overrides(original_context: BuildAndRunContext, overrides: argparse.Namespace):
    """Takes a context and provides an context with the given overrides."""
    context = copy.deepcopy(original_context)
    if overrides.build_dir:
        context.set_build_dir(overrides.build_dir)

    if overrides.compiler:
        context.set_compiler_alias(overrides.compiler)

    if overrides.build_config:
        context.set_build_config(overrides.build_config)

    yield context
