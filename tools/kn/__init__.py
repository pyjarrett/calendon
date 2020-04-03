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
from typing import List


def git_branch():
    return subprocess.check_output('git branch --show-current'.split()).decode().strip()


def git_cmd_version():
    return subprocess.check_output(['git', 'log', '-1', '--pretty=%ad',
                                    '--date=format:%d %b %H:%M', 'tools/kn']).decode().strip()


def generate_prompt():
    return f'({git_branch()}) '


# TODO: Should support building for clang, gcc and MSVC, wherever each one is available.
def supported_compilers():
    return ['default', 'clang', 'gcc']


def supported_commands():
    return ['gen']


def build_dir_for_compiler(compiler):
    if compiler == 'default' or compiler is None:
        return 'build'
    else:
        return f'build-{compiler}'


KN_CONFIG_COMPILER = 'compiler'
KN_CONFIG_KEYS = [KN_CONFIG_COMPILER]


def cmake_compiler_generator_settings(compiler):
    settings = []
    if compiler != 'default' and compiler is not None:
        settings = [f'-DCMAKE_C_COMPILER={compiler}']

    # https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2015%202017.html
    if sys.platform == 'win32' and (compiler is None or compiler == 'default'):
        help_output = subprocess.check_output(['cmake', '--help'])
        for line in help_output.decode().splitlines():
            if line.startswith('*'):
                print(line)
                generator = line[1:line.index('=')]
                if '[arch]' in generator:
                    generator = generator.replace('[arch]', '')
                generator = generator.strip()
                print(f'"{generator}"')
                break
        settings.extend(['-G', generator, '-A', 'x64'])

    return settings


def read_stream(stream, q):
    for line in stream:
        try:
            q.put(line.decode().strip())
        except UnicodeDecodeError:
            q.put(line.strip())


def run_program(command_line_array, **kwargs):
    """
    Runs another process while streaming its stdout and stderr.
    """
    print(f'Running: {" ".join(command_line_array)} {kwargs}')
    process = subprocess.Popen(command_line_array, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)

    out_queue = queue.Queue()
    err_queue = queue.Queue()

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


def os_specific_executable(named):
    """
    Produces an executable file name with an optional extension from the generic
    basename.
    """
    if sys.platform == 'win32':
        return named + '.exe'
    else:
        return named


def os_specific_shared_lib(named):
    """
    Produces a shared library with appropriate prefix and suffix from a generic
    basename.
    """
    if sys.platform == 'win32':
        return named + '.dll'
    else:
        return 'lib' + named + '.so'


def os_specific_demo_glob():
    """
    Produces a glob suitable for identifying demo shared libraries.
    """
    if sys.platform == 'win32':
        return '*.dll'
    else:
        return 'lib*.so'


def demo_name_from_os_specific_shared_lib(shared_lib: str):
    """
    Converts a name from an OS-specific shared library name to the generic name.
    """
    if sys.platform == 'win32':
        assert shared_lib.endswith('.dll')
        return os.path.splitext(shared_lib)[0]
    else:
        assert shared_lib.startswith('lib')
        assert shared_lib.endswith('.so')
        return shared_lib[3:-3]


class BuildAndRunContext:
    """
    A description of the current build and run environment.
    """

    def __init__(self):
        self.config = {}
        self.config['compilers'] = {}

    def save(self):
        with open(self.save_path(), 'w') as file:
            json.dump(self.config, file)

    def load(self):
        if os.path.isfile(self.save_path()):
            with open(self.save_path(), 'r') as file:
                self.config = json.load(file)
                if 'compilers' not in self.config:
                    self.config['compilers'] = {}

    def save_path(self):
        return self.config.get('save-path', '.hammer')

    def values(self):
        return self.config

    def driver_path(self):
        """Path to the Knell driver, relative to the build directory."""
        return os.path.join('src', 'driver', os_specific_executable('knell-driver'))

    def lib_path(self):
        """Path to the Knell lib itself, relative to the build directory."""
        return os.path.join('src', 'knell', os_specific_shared_lib('knell'))

    def demo_path(self):
        """Path to a demo, relative to the build directory."""
        return os.path.join('src', 'demos', os_specific_shared_lib(self.demo()))

    def build_dir(self):
        """
        The location of the out-of-tree build.
        """
        return build_dir_for_compiler(self.config.get('compiler'))

    def build_config(self):
        """
        A particular version of the build, such as Debug, or Release.
        """
        return self.config.get('config', 'Debug')

    def has_default_compiler(self):
        return self.config.get('compiler') is not None

    def compiler(self):
        """
        The type of the compiler, independent of the path.
        """
        return self.config.get('compiler')

    def compiler_path(self):
        compiler_alias = self.config.get('compiler')
        if compiler_alias is None:
            return None

        if compiler_alias in self.config['compilers'].keys():
            return self.config['compilers'][compiler_alias]

    def demo(self):
        """
        The generic name of the current demo (without a prefix or suffix).
        :return:
        """
        return self.config.get('demo')

    def add(self, args):
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
                else:
                    print(f'Compiler "{args.alias}" does not exist at {args.path}')
                    return 1
        except SystemExit:
            return 1

    def parse(self, args):
        parser = argparse.ArgumentParser(usage='key value\n    Sets a key equal to a value.\n')
        parser.add_argument('key', choices=['compiler', 'config', 'demo'])
        parser.add_argument('value')

        try:
            args = parser.parse_args(args.split())
            if args.key == 'compiler' and args.value not in self.config['compilers'].keys():
                print(f'Unknown compiler: {args.key}.  Add compiler aliases first.')
                return 1
            else:
                self.config[args.key] = args.value
                return 0
        except SystemExit:
            return 1


def demos(context: BuildAndRunContext) -> List[str]:
    """
    Returns a list of all currently available demos.
    """
    demo_glob = os.path.join(context.build_dir(), 'src', 'demos', os_specific_demo_glob())
    demos = [os.path.basename(demo) for demo in glob.glob(demo_glob)]
    return sorted([demo_name_from_os_specific_shared_lib(d) for d in demos])


def run_demo(context: BuildAndRunContext):
    """
    Runs the current demo using a given context.
    """
    print('Running demo')
    if context.demo() is None:
        print('No demo selected to run')
        return 1
    else:
        return run_program([context.driver_path(), '--game', context.demo_path()], cwd=context.build_dir())


class Hammer(cmd.Cmd):
    """
    Interactive command-line tool to simplify development with Knell.  This
    allows building, testing, and running knell consistently across platforms.
    """
    prompt = generate_prompt()
    intro = 'hammer: A tool to help with Knell building, running, and debugging\n'

    def __init__(self, reload_fn):
        super().__init__()
        self.context = BuildAndRunContext()
        self.context.load()
        self.reload_fn = reload_fn
        self.reload = False
        self.history = []
        self.cmd_start_time = 0
        self.last_exit_code = 0

    @staticmethod
    def update_prompt():
        Hammer.prompt = generate_prompt()

    def precmd(self, line):
        self.cmd_start_time = time.monotonic()
        self.history.append(line)
        return line

    def postcmd(self, stop, line):
        now = time.monotonic()
        print(f'{line} {now - self.cmd_start_time:>8.2f}s')

        if not stop:
            Hammer.update_prompt()
            return False
        else:
            return True

    def default(self, arg):
        self.do_help('')

    def do_version(self, arg):
        print(f'hammer REPL version: {git_cmd_version()}')

    def do_quit(self, arg):
        self.reload = False
        return True

    def do_exit(self, arg):
        """Alias for quit."""
        self.reload = False
        return True

    def do_save(self, arg):
        self.context.save(arg)

    def do_load(self, arg):
        self.context.load(arg)

    def do_reload(self, args):
        """[For Development] Reloads Hammer with updated source."""
        self.reload_fn()
        self.reload = True
        self.context.save()
        return True

    def do_last_commit(self, arg):
        """Prints the hash and short log of the last commit."""
        print(subprocess.check_output('git log -1 --pretty=format:%h:%s'.split()).decode())

    def do_config(self, arg):
        """Print the current state of configuration variables."""
        print(json.dumps(self.context.values(), indent=4))

    def do_set(self, args):
        """
        Sets configuration values.
        """
        self.last_exit_code = self.context.parse(args)

    def do_add(self, args):
        self.last_exit_code = self.context.add(args)

    def do_clean(self, args):
        """
        Wipes build directory.
        """
        build_dir = self.context.build_dir()
        if os.path.exists(build_dir):
            if not os.path.isdir(build_dir):
                print(f'Build directory {build_dir} exists as something other than a directory')
                return
            else:
                print(f'Wiping the build directory {build_dir}')
                shutil.rmtree(build_dir)

    def do_sync(self, args):
        """
        Fetch, rebase and push to all remotes.
        """
        commands = [
            'git fetch',
            'git pull --rebase',
            'git push origin',
        ]
        for line in commands:
            self.last_exit_code = run_program(line.split())
            if self.last_exit_code != 0:
                break

    def do_gen(self, args):
        """
        Generate project files.
        """
        build_dir = self.context.build_dir()
        if os.path.exists(build_dir):
            if not os.path.isdir(build_dir):
                print(f'Build directory {build_dir} exists as something other than a directory')
                return
            else:
                print(f'{build_dir} exists.')
                return

        print(f'Creating build directory {build_dir}')
        os.mkdir(build_dir)
        cmake_args = ['cmake', '..']

        compiler = self.context.compiler()
        cmake_args.extend(cmake_compiler_generator_settings(compiler))
        self.last_exit_code = run_program(cmake_args, cwd=build_dir)

    def do_build(self, args):
        """
        Builds using the current project configuration.
        """
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

    def do_check(self, args):
        """
        Runs all tests.
        """
        build_dir = self.context.build_dir()
        if not os.path.exists(build_dir):
            print(f'Build dir does not exist at {build_dir}')
            return
        self.last_exit_code = run_program(['cmake', '--build', '.',
                                           '--target', 'check',
                                           '--config', self.context.build_config()],
                                          cwd=build_dir)

    def do_check_iterate(self, args):
        """
        Runs only failed tests.
        """
        build_dir = self.context.build_dir()
        if not os.path.exists(build_dir):
            print(f'Build dir does not exist at {build_dir}')
            return
        self.last_exit_code = run_program(['cmake', '--build', '.',
                                           '--target', 'check-iterate',
                                           '--config', self.context.build_config()],
                                          cwd=build_dir)

    def do_demo(self, args):
        """
        Lists all available demos.
        """
        for demo in demos(self.context):
            print(demo)

    def do_run(self, args):
        """
        Use 'run demo' to run your currently selected demo.
        """
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

    def do_history(self, args):
        """
        Prints command history.
        """
        for index, line in enumerate(self.history):
            print(f'{index:<5} {line}')

    def do_redo(self, args):
        """
        "redo i" re-runs command 'i' from history.
        """
        try:
            index = int(args)
            if index >= 0 and index < len(self.history):
                self.onecmd(self.history[index])
            else:
                print(f'Invalid history id {index}')
        except ValueError:
            print(f'Can only redo history commands based on index.')

    if sys.platform != 'win32':
        def do_symbols(self, args):
            """
            Lists symbols exported in the Knell shared library.
            """
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
