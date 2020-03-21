import cmd
import subprocess


def git_branch():
    return subprocess.check_output('git branch --show-current'.split()).decode().strip()


def generate_prompt():
    return f'(knife {git_branch()}) '


class Knife(cmd.Cmd):
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
        self.reload_fn()
        self.reload = True
        return True

    def do_last_commit(self, arg):
        """Prints the has and short log of the last commit."""
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
