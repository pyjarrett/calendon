"""
Map command names to parsers and actions so they don't need to be mapped individually.
"""
from __future__ import annotations  # See PEP 484 and PEP 563.
from dataclasses import dataclass
from typing import Any
import calendon.commands as cmd
import calendon.parsers as parsers


@dataclass
class CrankCommand:
    """A description of a command to hook up in Crank."""
    name: str
    help: str  # pylint: disable=redefined-builtin
    parser: Any
    command: Any

    @staticmethod
    def create(name: str, help: str) -> CrankCommand:  # pylint: disable=redefined-builtin
        """Make a command using parser_* and cmd_* functions from associated libraries."""
        parser = getattr(parsers, f'parser_{name}', None)
        command = getattr(cmd, f'cmd_{name}', None)

        if parser is None:
            raise ValueError(f'No parser function for {name}. parse_{name} is missing.')
        if command is None:
            raise ValueError(f'No command function for {name}. cmd_{name} is missing.')

        return CrankCommand(name=name, help=help, parser=parser, command=command)


COMMANDS_DESCRIPTION = [('clean', 'Remove build directories.'),
                        ('gen', 'Generate build types.'),
                        ('build', 'Do a build.'),
                        ('doc', 'Generate project documentation.'),
                        ('check', 'Run tests.'),
                        ('demo', 'Sets the default demo to run.'),
                        ('export', 'Creates an exported version of Calendon.'),
                        ('run', 'Run a program with the driver.'),
                        ('new', 'Create a new demo'),
                        ('register', 'Register a path as a safe program to run.'),
                        ('default', 'Default a value across multiple Crank commands.'),
                        ('reset', 'Clear a set default.'),
                        ('source', 'Read and run Crank commands from file'),
                        ('env', 'Print the Crank environment configuration'),
                        ('save', 'Save Crank configuration to file.'),
                        ('load', 'Load Crank configuration from file.'),
                        ('pysetup', 'Setup virtual environment for subtools.'),
                        ('pycheck', 'Run Python checks on Crank source.')]

COMMANDS = {name: CrankCommand.create(name, help) for name, help in COMMANDS_DESCRIPTION}
