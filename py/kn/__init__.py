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
import importlib
import os
import multiprocessing
import queue
import shutil
import subprocess
import sys
import threading
import time
from typing import IO, List, Optional

import kn.cmake as cmake
import kn.git as git
import kn.multiplatform as mp
import kn.project as proj


def base_arg_parser() -> argparse.ArgumentParser:
    """Creates a generic parser for additional arguments to commands."""
    usage = """generic
--help
--dry-run
--verbose
"""
    parser = argparse.ArgumentParser(usage=usage)
    parser.add_argument('--dry-run', action='store_true')
    parser.add_argument('--verbose', action='store_true')
    return parser


def parse_overrides(args: str, parser: argparse.ArgumentParser) -> Optional[argparse.Namespace]:
    """Parses an argument string with a given parser."""
    try:
        return parser.parse_args(args.split())
    except SystemExit:
        return None


def reload():
    """
    Reloads Knell and it's associated submodules.

    Provided to improve iteration of development on Hammer.
    """
    importlib.reload(cmake)
    importlib.reload(git)
    importlib.reload(mp)
    importlib.reload(proj)


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
              ]

    for program in checks:
        cmd_line = program
        cmd_line.extend(py_files())
        if run_program(cmd_line, cwd='py') != 0:
            return False

    return True


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


def all_demos(context: proj.BuildAndRunContext) -> List[str]:
    """Return a list of all currently available demos."""
    demo_glob = os.path.join(context.demo_dir(), mp.demo_glob())
    demos = [os.path.basename(demo) for demo in glob.glob(demo_glob)]
    return sorted([mp.shared_lib_to_root(d) for d in demos])


def run_demo(context: proj.BuildAndRunContext):
    """Run the current demo using a given context."""
    print('Running demo')
    if context.demo() is None:
        print('No demo selected to run')
        return 1

    return run_program(
        [os.path.join(context.build_dir(), context.driver_executable()),
         '--game',
         context.current_demo_path()],
        cwd='build')


class Hammer(cmd.Cmd):
    """
    Interactive command-line tool to simplify development with Knell.

    This allows building, testing, and running knell consistently across
    platforms.
    """
    prompt = ''
    intro = 'hammer: A tool to help with Knell building, running, and debugging\n'

    def __init__(self, interactive: bool):
        """Start up a Hammer instanced, pre-loaded with its config."""
        super().__init__()
        self.interactive = interactive
        self.context = proj.BuildAndRunContext()
        self.context.load()
        self.reload = False
        self.history: List[str] = []
        self.cmd_start_time = 0
        self.last_exit_code = 0
        self.prompt = self._generate_prompt()
        self._ensure_home_dir_exists()

    def _generate_prompt(self):
        """Return a prompt suitable for command input in Hammer."""
        current_prompt = f'{git.current_branch()} {self.context.build_config()}'

        if self.context.demo() is not None:
            current_prompt += f' {self.context.demo()}'

        return f'({current_prompt}) '

    def _ensure_home_dir_exists(self):
        if self.context.home_dir() is None:
            if self.interactive:
                print('No home directory for Knell specified.')
                self._input_home_dir()
            else:
                print('No root specified for Knell home, either as KNELL_HOME or in .hammer file.')
                sys.exit(1)

        if not os.path.isdir(self.context.home_dir()):
            print(f'Home directory {self.context.home_dir()} does not exist.')
            if self.interactive:
                self._input_home_dir()
            else:
                sys.exit(1)

    def _input_home_dir(self):
        possible_knell_root: str = input(f'Current Directory: {os.getcwd()}\n'
                                         'Where is the root of the Knell project? ')
        if os.path.isdir(possible_knell_root):
            self.context.set_home_dir(possible_knell_root)
        else:
            print(f'No directory exists at {possible_knell_root}')
            sys.exit(1)

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

        self.prompt = self._generate_prompt()
        return False

    def default(self, _args):
        """Override 'default' to print help."""
        self.do_help('')

    def do_version(self, args):
        """Print the current git version."""
        if args == '':
            print(f'hammer REPL version: {git.last_commit_date()}')
        else:
            print(f'Unknown arguments: {args}')

    def do_quit(self, _args):
        """Save configuration and exit."""
        self.reload = False
        return True

    def do_exit(self, _args):
        """Alias for quit."""
        self.reload = False
        return True

    def do_save(self, args):
        """Save the current configuration."""
        if args == '':
            self.context.save()
        else:
            print(f'Unknown arguments: {args}')

    def do_load(self, args):
        """Reload the configuration."""
        if args == '':
            self.context.load()
        else:
            print(f'Unknown arguments: {args}')

    def do_reload(self, args):
        """[For Development] Reload Hammer with updated source."""
        if args == '':
            self.reload = True
            self.context.save()
            return True

        print(f'Unknown arguments: {args}')
        return False

    def do_last_commit(self, args):
        """Print the hash and short log of the last commit."""
        if args == '':
            print(git.last_commit_short_log())
        else:
            print(f'Unknown arguments: {args}')

    def do_config(self, args):
        """Print the current state of configuration variables."""
        if args == '':
            print(json.dumps(self.context.values(), indent=4))
        else:
            print(f'Unknown arguments: {args}')

    def do_set(self, args):
        """Set configuration values."""
        self.last_exit_code = self.context.parse_config_value(args)

    def do_add(self, args):
        """Add a variable to the environment."""
        self.last_exit_code = self.context.add(args)

    def do_clean(self, args):
        """Wipe build directory."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            if overridden is None:
                return

            build_dir = overridden.build_dir()
            if os.path.exists(build_dir):
                if not os.path.isdir(build_dir):
                    print(f'Build directory {build_dir} exists as something other than a directory')
                else:
                    print(f'Wiping the build directory {build_dir}')
                    shutil.rmtree(build_dir)

    def do_gen(self, args):
        """Generate project files."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        if overrides is None:
            return
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            build_dir = overridden.build_dir()
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

            compiler = overridden.compiler()
            cmake_args.extend(cmake.generator_settings_for_compiler(compiler))
            self.last_exit_code = run_program(cmake_args, cwd=build_dir)

    def do_build(self, args):
        """Build using the current project configuration."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        if overrides is None:
            return
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            build_dir = overridden.build_dir()
            if not os.path.exists(build_dir):
                print(f'Build dir does not exist at {build_dir}')
                self.last_exit_code = 1
                return

            cmake_args = ['cmake', '--build', '.',
                          '--parallel', str(multiprocessing.cpu_count()),
                          '--config', overridden.build_config()]

            if not overridden.has_default_compiler():
                cmake_args.append(f'-DCMAKE_C_COMPILER={overridden.compiler_path()}')

            self.last_exit_code = run_program(cmake_args, cwd=build_dir)

    def do_pycheck(self, _args):
        """Run python checks on Hammer scripts."""
        run_pycheck()

    def do_check(self, args):
        """Run all tests."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        if overrides is None:
            return
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            build_dir = overridden.build_dir()
            if not os.path.exists(build_dir):
                print(f'Build dir does not exist at {build_dir}')
                return
            self.last_exit_code = run_program(['cmake', '--build', '.',
                                               '--target', 'check',
                                               '--config', overridden.build_config()],
                                              cwd=build_dir)

    def do_check_iterate(self, args):
        """Run only failed tests."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        if overrides is None:
            return
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            build_dir = overridden.build_dir()
            if not os.path.exists(build_dir):
                print(f'Build dir does not exist at {build_dir}')
                return
            self.last_exit_code = run_program(['cmake', '--build', '.',
                                               '--target', 'check-iterate',
                                               '--config', overridden.build_config()],
                                              cwd=build_dir)

    def do_demo(self, args):
        """List all available demos."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        if overrides is None:
            return
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            for demo in all_demos(overridden):
                print(demo)

    def do_run(self, args):
        """Use 'run demo' to run your currently selected demo."""
        overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
        if overrides is None:
            return
        with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
            build_dir = overridden.build_dir()
            if not os.path.exists(build_dir):
                print(f'Build dir does not exist at {build_dir}')
                return
            if args == 'demo':
                if overridden.demo() is None:
                    print('No demo to run')
                    return
                build_status = run_program(['cmake', '--build', '.',
                                            '--target', overridden.demo(),
                                            '--config', overridden.build_config()], cwd=build_dir)
                if build_status == 0:
                    self.last_exit_code = run_demo(overridden)
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
        def do_symbols(self, args):
            """List symbols exported in the Knell shared library."""
            overrides = parse_overrides(args, proj.add_build_args(base_arg_parser()))
            if overrides is None:
                return
            with proj.parse_build_context_with_overrides(self.context, overrides) as overridden:
                build_dir = overridden.build_dir()
                if not os.path.exists(build_dir):
                    print(f'Build dir does not exist at {build_dir}')
                    return
                lines: List[str] = subprocess.check_output(['nm', '-D', overridden.lib_path()],
                                                           cwd=overridden.build_dir()).decode().splitlines()
                categories = set()
                for line in filter(lambda sym_line: ' T ' in sym_line, lines):
                    # line will be in "ADDRESS T Symbol" format
                    symbol = line.split()[2]
                    categories.add(symbol.split('_')[0])
                    print(symbol)
                print()
                print(f'Exporting {len(lines)} symbols from systems')
                print(f'{" ".join(sorted(categories))}')
