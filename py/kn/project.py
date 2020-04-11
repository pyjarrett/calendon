import argparse
import json
import os

import kn.multiplatform as mp


class BuildAndRunContext:
    """A description of the current build and run environment."""
    def __init__(self):
        """Create an empty config."""
        self.config = self._empty_config()

    @staticmethod
    def _empty_config():
        return {'compilers': {}}

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

    def build_dir(self):
        """The location of the out-of-tree build."""
        compiler = self.config.get('compiler')
        build_dir = 'build'
        if compiler is not None and compiler != 'default':
            build_dir = f'build-{compiler}'

        build_dir += '-' + self.build_config()
        return os.path.abspath(os.path.join(self.home_dir(), build_dir))

    def build_config(self):
        """A particular version of the build, such as Debug, or Release."""
        return self.config.get('config', 'Debug')

    def has_default_compiler(self):
        """Check to see if CMake's compiler choice been overriden."""
        return self.config.get('compiler') is None and self.config.get('compiler') != 'default'

    def compiler(self):
        """The type of the compiler, independent of the path."""
        return self.config.get('compiler')

    def compiler_path(self):
        """Return the current compiler path or None if none set."""
        compiler_alias = self.config.get('compiler')
        if compiler_alias is None:
            return None

        if compiler_alias in self.config['compilers'].keys():
            return self.config['compilers'][compiler_alias]

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
            args = parser.parse_args(args.split())
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
            args = parser.parse_args(args.split())
            if args.key == 'compiler' and args.value not in self.config['compilers'].keys():
                print(f'Unknown compiler: {args.key}.  Add compiler aliases first.')
                return 1

            self.config[args.key] = args.value
            return 0
        except SystemExit:
            return 1