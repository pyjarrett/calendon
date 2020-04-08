"""
Knell build tool module for Hammer tool.

Hammer is the programmer's helper tool for having consistent commands across
environment and simplifying the passing of command line parameters between
multiple runs of the game.
"""
import argparse
import cmd
import glob
import json
import os
import multiprocessing
import queue
import shutil
import subprocess
import sys
import threading
import time
from typing import IO, List

import kn.cmake as cmake
import kn.git as git
import kn.multiplatform as mp


def py_files() -> List[str]:
    """
    A list containing the top-level directories and files to provide to tools.
    """
    return ['hammer.py', 'kn']


def run_pycheck() -> bool:
    """
    Run all the checks on the Hammer Python source.

    Return true if the tests succeeded.
    """
    checks = [['mypy'],
              ['pylint'],
              ['pycodestyle', '--max-line-length=120', '--show-source', '--statistics', '--count'],
              ['pydocstyle', '--ignore=D200,D203,D204,D212,D401'],
              ['bandit', '-r'],
              ]

    for program in checks:
        cmd_line = program
        cmd_line.extend(py_files())
        if run_program(cmd_line, cwd='py') != 0:
            return False

    return True


def generate_prompt():
    """Return a prompt suitable for command input in Hammer."""
    return f'({git.current_branch()}) '


def build_dir_for_compiler(compiler):
    """Maps a compiler to its own out-of-tree build directory."""
    if compiler == 'default' or compiler is None:
        return 'build'
    return f'build-{compiler}'


def read_stream(stream: IO, queued_lines: queue.Queue):
    """Reads a stream into a queue."""
    for line in stream:
        try:
            queued_lines.put(line.decode().strip())
        except UnicodeDecodeError:
            queued_lines.put(line.strip())


def run_program(command_line: List[str], **kwargs):
    """Runs another process while streaming its stdout and stderr."""
    print(f'Running: {" ".join(command_line)} {kwargs}')
    process = subprocess.Popen(command_line,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               **kwargs)

    out_queue: queue.Queue = queue.Queue()
    err_queue: queue.Queue = queue.Queue()

    out_thread = threading.Thread(target=read_stream, args=(process.stdout, out_queue))
    err_thread = threading.Thread(target=read_stream, args=(process.stderr, err_queue))

    out_thread.start()
    err_thread.start()

    while out_thread.is_alive() or err_thread.is_alive() or not out_queue.empty() or not err_queue.empty():
        try:
            line = out_queue.get_nowait()
            print(line)
        except queue.Empty:
            pass

        try:
            line = err_queue.get_nowait()
            print(f'err:{line}')
        except queue.Empty:
            pass

    out_thread.join()
    err_thread.join()

    return process.wait()


class BuildAndRunContext:
    """A description of the current build and run environment."""
    def __init__(self):
        """Create an empty config."""
        self.config = {}
        self.config['compilers'] = {}

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
        else:
            print(f'Config file {self.save_path()} does not exist.')

    def save_path(self):
        """The currently set configuration save path."""
        return self.config.get('save-path', '.hammer')

    def values(self):
        """A copy of all configuration values."""
        return self.config

    def driver_path(self):
        """Path to the Knell driver, relative to the build directory."""
        return os.path.join('src', 'driver', mp.root_to_executable('knell-driver'))

    def lib_path(self):
        """Path to the Knell lib itself, relative to the build directory."""
        return os.path.join('src', 'knell', mp.root_to_shared_lib('knell'))

    def demo_path(self):
        """Path to a demo, relative to the build directory."""
        return os.path.join('src', 'demos', mp.root_to_shared_lib(self.demo()))

    def build_dir(self):
        """The location of the out-of-tree build."""
        return build_dir_for_compiler(self.config.get('compiler'))

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


def all_demos(context: BuildAndRunContext) -> List[str]:
    """Return a list of all currently available demos."""
    demo_glob = os.path.join(context.build_dir(), 'src', 'demos', mp.demo_glob())
    demos = [os.path.basename(demo) for demo in glob.glob(demo_glob)]
    return sorted([mp.shared_lib_to_root(d) for d in demos])


def run_demo(context: BuildAndRunContext):
    """Run the current demo using a given context."""
    print('Running demo')
    if context.demo() is None:
        print('No demo selected to run')
        return 1

    return run_program(
        [os.path.join(context.build_dir(), context.driver_path()),
         '--game',
         'src/demos/planets.dll'],
        cwd='build')


class Hammer(cmd.Cmd):
    """
    Interactive command-line tool to simplify development with Knell.

    This allows building, testing, and running knell consistently across
    platforms.
    """
    prompt = generate_prompt()
    intro = 'hammer: A tool to help with Knell building, running, and debugging\n'

    def __init__(self):
        """Start up a Hammer instanced, pre-loaded with its config."""
        super().__init__()
        self.context = BuildAndRunContext()
        self.context.load()
        self.reload = False
        self.history = []
        self.cmd_start_time = 0
        self.last_exit_code = 0

    @staticmethod
    def _update_prompt():
        Hammer.prompt = generate_prompt()

    def precmd(self, line):
        """Executed before each command is interpreted and executed."""
        self.cmd_start_time = time.monotonic()
        self.history.append(line)
        return line

    def postcmd(self, stop, line):
        """Executed after each command."""
        now = time.monotonic()
        print(f'{line} {now - self.cmd_start_time:>8.2f}s')

        if stop:
            return True

        Hammer._update_prompt()
        return False

    def default(self, _args):
        """Override 'default' to print help."""
        self.do_help('')

    def do_version(self, _args):
        """Print the current git version."""
        print(f'hammer REPL version: {git.last_commit_date()}')

    def do_quit(self, _args):
        """Save configuration and exit."""
        self.reload = False
        return True

    def do_exit(self, _args):
        """Alias for quit."""
        self.reload = False
        return True

    def do_save(self, _args):
        """Save the current configuration."""
        self.context.save()

    def do_load(self, _args):
        """Reload the configuration."""
        self.context.load()

    def do_reload(self, _args):
        """[For Development] Reload Hammer with updated source."""
        self.reload = True
        self.context.save()
        return True

    def do_last_commit(self, _args):
        """Print the hash and short log of the last commit."""
        print(subprocess.check_output('git log -1 --pretty=format:%h:%s'.split()).decode())

    def do_config(self, _args):
        """Print the current state of configuration variables."""
        print(json.dumps(self.context.values(), indent=4))

    def do_set(self, args):
        """Set configuration values."""
        self.last_exit_code = self.context.parse_config_value(args)

    def do_add(self, args):
        """Add a variable to the environment."""
        self.last_exit_code = self.context.add(args)

    def do_clean(self, _args):
        """Wipe build directory."""
        build_dir = self.context.build_dir()
        if os.path.exists(build_dir):
            if not os.path.isdir(build_dir):
                print(f'Build directory {build_dir} exists as something other than a directory')
            else:
                print(f'Wiping the build directory {build_dir}')
                shutil.rmtree(build_dir)

    def do_gen(self, args):
        """Generate project files."""
        build_dir = self.context.build_dir()
        if os.path.exists(build_dir):
            if not os.path.isdir(build_dir):
                print(f'Build directory {build_dir} exists as something other than a directory')

            print(f'{build_dir} exists.')
            return

        print(f'Creating build directory {build_dir}')
        os.mkdir(build_dir)
        cmake_args = ['cmake', '..']

        if args == '--enable-ccache':
            cmake_args.append('-DKN_ENABLE_CCACHE=1')

        compiler = self.context.compiler()
        cmake_args.extend(cmake.generator_settings_for_compiler(compiler))
        self.last_exit_code = run_program(cmake_args, cwd=build_dir)

    def do_build(self, _args):
        """Build using the current project configuration."""
        build_dir = self.context.build_dir()
        if not os.path.exists(build_dir):
            print(f'Build dir does not exist at {build_dir}')
            self.last_exit_code = 1
            return

        cmake_args = ['cmake', '--build', '.',
                      '--parallel', str(multiprocessing.cpu_count()),
                      '--config', self.context.build_config()]

        if not self.context.has_default_compiler():
            cmake_args.append(f'-DCMAKE_C_COMPILER={self.context.compiler_path()}')

        self.last_exit_code = run_program(cmake_args, cwd=build_dir)

    def do_pycheck(self, _args):
        """Run python checks on Hammer scripts."""
        run_pycheck()

    def do_check(self, _args):
        """Run all tests."""
        build_dir = self.context.build_dir()
        if not os.path.exists(build_dir):
            print(f'Build dir does not exist at {build_dir}')
            return
        self.last_exit_code = run_program(['cmake', '--build', '.',
                                           '--target', 'check',
                                           '--config', self.context.build_config()],
                                          cwd=build_dir)

    def do_check_iterate(self, _args):
        """Run only failed tests."""
        build_dir = self.context.build_dir()
        if not os.path.exists(build_dir):
            print(f'Build dir does not exist at {build_dir}')
            return
        self.last_exit_code = run_program(['cmake', '--build', '.',
                                           '--target', 'check-iterate',
                                           '--config', self.context.build_config()],
                                          cwd=build_dir)

    def do_demo(self, _args):
        """List all available demos."""
        for demo in all_demos(self.context):
            print(demo)

    def do_run(self, args):
        """Use 'run demo' to run your currently selected demo."""
        build_dir = self.context.build_dir()
        if not os.path.exists(build_dir):
            print(f'Build dir does not exist at {build_dir}')
            return
        if args == 'demo':
            if self.context.demo() is None:
                print('No demo to run')
                return
            build_status = run_program(['cmake', '--build', '.',
                                        '--target', self.context.demo(),
                                        '--config', self.context.build_config()], cwd=build_dir)
            if build_status == 0:
                self.last_exit_code = run_demo(self.context)
            else:
                self.last_exit_code = build_status

    def do_history(self, _args):
        """Print command history."""
        for index, line in enumerate(self.history):
            print(f'{index:<5} {line}')

    def do_redo(self, args):
        """'redo i' re-runs command 'i' from history."""
        try:
            index = int(args)
            if index >= 0 and index < len(self.history):
                self.onecmd(self.history[index])
            else:
                print(f'Invalid history id {index}')
        except ValueError:
            print(f'Can only redo history commands based on index.')

    if sys.platform != 'win32':
        def do_symbols(self, _args):
            """List symbols exported in the Knell shared library."""
            build_dir = self.context.build_dir()
            if not os.path.exists(build_dir):
                print(f'Build dir does not exist at {build_dir}')
                return
            lines: List[str] = subprocess.check_output(['nm', '-D', self.context.lib_path()],
                                                       cwd=self.context.build_dir()).decode().splitlines()
            categories = set()
            for line in filter(lambda sym_line: ' T ' in sym_line, lines):
                # line will be in "ADDRESS T Symbol" format
                symbol = line.split()[2]
                categories.add(symbol.split('_')[0])
                print(symbol)
            print()
            print(f'Exporting {len(lines)} symbols from systems')
            print(f'{" ".join(sorted(categories))}')
