import cmd
import os
import shutil
import subprocess


def git_branch():
    return subprocess.check_output('git branch --show-current'.split()).decode().strip()


def generate_prompt():
    return f'(knife {git_branch()}) '


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


def cmake_compiler_settings(compiler):
    return [f'-DCMAKE_C_COMPILER={compiler}']


class Knife(cmd.Cmd):
    """
    Interactive command-line tool to simplify development with Knell.  This
    allows building, testing, and running knell consistently across platforms.
    """
    prompt = generate_prompt()
    intro = 'Knife: A tool to help with Knell building, running, and debugging\n'

    def __init__(self, reload_fn):
        super().__init__()
        self.config = {}
        self.reload_fn = reload_fn
        self.reload = False

    @staticmethod
    def update_prompt():
        Knife.prompt = generate_prompt()

    def postcmd(self, stop, line):
        if not stop:
            Knife.update_prompt()
            return False
        else:
            return True

    def do_quit(self, arg):
        self.reload = False
        return True

    def do_exit(self, arg):
        self.reload = False
        return True

    def do_echo(self, arg):
        print(arg)

    def do_reload(self, args):
        """[For Development] Reloads knife with updated source."""
        self.reload_fn()
        self.reload = True
        return True

    def do_last_commit(self, arg):
        """Prints the hash and short log of the last commit."""
        print(subprocess.check_output('git log -1 --pretty=format:%h:%s'.split()).decode())

    def do_config(self, arg):
        """Print the current state of configuration variables."""
        print(self.config)

    def do_set(self, args):
        words = args.split()
        if len(words) == 0:
            print('No words to set')
            return
        if len(words) == 2:
            key = words[0]
            value = words[1]
            self.config[key] = value
        else:
            key = words.pop(0)
            value = " ".join(words)
            self.config[key] = value

    def do_clean(self, args):
        words = args.split()
        compiler = None
        if len(words) == 1:
            compiler = words[0]
        elif len(words) == 0:
            compiler = 'default'
        else:
            print(f'Cannot clean with {args}')
            return

        build_dir = build_dir_for_compiler(compiler)
        if os.path.exists(build_dir):
            if not os.path.isdir(build_dir):
                print(f'Build directory {build_dir} exists as something other than a directory')
                return
            else:
                print(f'Wiping the build directory {build_dir}')
                shutil.rmtree(build_dir)


    def do_gen(self, args):
        words = args.split()
        compiler = None
        if len(words) >= 1:
            compiler = words[0]
        elif len(words) == 0:
            compiler = 'default'
        else:
            print(f'Cannot generate with {args}')
            return

        build_dir = build_dir_for_compiler(compiler)
        if os.path.exists(build_dir):
            if not os.path.isdir(build_dir):
                print(f'Build directory {build_dir} exists as something other than a directory')
                return
            elif '--force' in words:
                print(f'Wiping the build directory {build_dir}')
                shutil.rmtree(build_dir)
            else:
                print(f'{build_dir} exists.  Rerun with --force to remove the existing directory')
                return

        print(f'Creating build directory {build_dir}')
        os.mkdir(build_dir)
        cmake_args = ['cmake', '..']
        if compiler is not None:
            cmake_args.extend(cmake_compiler_settings(compiler))
        print(cmake_args)
        subprocess.check_call(cmake_args, cwd=build_dir)