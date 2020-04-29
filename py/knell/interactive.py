import argparse
import cmd
import shlex

import knell.script
from knell.command_map import COMMAND_PARSERS
from knell.parsers import parser_add_top_level_args


class InteractiveMode(cmd.Cmd):
    """Handler for Crank's interactive mode."""
    prompt = "crank>  "

    def __init__(self, ctx):
        """Initialize interactive mode with a known context."""
        self._names = []
        self.ctx = ctx
        self.parser = CmdArgumentParser()
        self.reload = False
        for command in COMMAND_PARSERS:
            parser = COMMAND_PARSERS[command][0]
            action = COMMAND_PARSERS[command][1]
            setattr(self, f'do_{command}', map_command(self, command, parser, action))
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
        self.reload = True
        return True

    def do_quit(self, _arg):
        """Stop and do not reload Crank."""
        return True

    def do_exit(self, _arg):
        """Stop and do not reload Crank."""
        return True


class CmdArgumentParser(argparse.ArgumentParser):
    def parse_args(self, args):
        """Catch the SystemExit so bad parses don't crash the interactive session."""
        try:
            return super().parse_args(args)
        except SystemExit:
            return None


def map_command(self, command, parser, action):
    # Double-parser args because this needs to fit into a lambda.
    # Shortcircuit on argument parsing to prevent command functions from needing
    # to perform error handling relating to argument parsing.
    wrap = lambda args: knell.script.parse_args(shlex.split(args)) is not None and action(
        self.ctx, knell.script.parse_args(shlex.split(args))) and False
    wrap.__doc__ = action.__doc__
    return wrap