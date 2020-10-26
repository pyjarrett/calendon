"""
Interactive control mode for Crank.
"""
import argparse
import cmd
import shlex

from calendon.command_map import COMMANDS
from calendon.parsers import parser_add_top_level_args


def _map_command(self, command, parser, action):
    """Wraps argument parsing and execution of an action."""
    # Double-parser args because this needs to fit into a lambda.
    # Short-circuit on argument parsing to prevent command functions from needing
    # to perform error handling relating to argument parsing.
    wrap = lambda args: parser(CmdArgumentParser(usage=command)).parse_args(shlex.split(args)) is not None and action(
        self.ctx, parser(CmdArgumentParser(usage=command)).parse_args(shlex.split(args))) and False
    wrap.__doc__ = action.__doc__
    return wrap


class InteractiveMode(cmd.Cmd):
    """Handler for Crank's interactive mode."""
    prompt = "crank>  "

    def __init__(self, ctx):
        """Initialize interactive mode with a known context."""
        self._names = []
        self.ctx = ctx
        self.parser = CmdArgumentParser()
        self.reload = False
        for command in COMMANDS:
            parser = COMMANDS[command].parser
            action = COMMANDS[command].command
            setattr(self, f'do_{command}', _map_command(self, command, parser, action))
            self._names.append(f'do_{command}')
        parser_add_top_level_args(self.parser)
        super().__init__()

    def get_names(self):
        """
        Overridden to provide support for dynamically adding commands.

        cmd.Cmd uses dir(self.__class__) which won't include our monkey-patched
        methods in the default command list.
        """
        return self._names

    def do_reload(self, _arg):
        """Indicate the script should reload on the next pass."""
        self.reload = True
        return True

    def do_quit(self, _arg):
        """Stop and do not reload Crank."""
        return True

    def do_exit(self, _arg):
        """Stop and do not reload Crank."""
        return True


class CmdArgumentParser(argparse.ArgumentParser):
    """Base parser for use with Cmd to prevent crashing on bad parses."""
    def parse_args(self, args=None, namespace=None):
        """Catch the SystemExit so bad parses don't crash the interactive session."""
        try:
            return super().parse_args(args, namespace)
        except SystemExit:
            return None
