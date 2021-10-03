import importlib

from calendon import cmake
from calendon import commands
from calendon import command_map
from calendon import context
from calendon import interactive
from calendon import multiplatform as mp
from calendon import parsers
from calendon import run


def reload():
    """
    Reloads Calendon and it's associated submodules.

    Provided to improve iteration of development on Crank.
    """
    # Manually reload for now.
    # TODO: Implement a depedency trace of modules for better maintainability.
    importlib.reload(cmake)
    importlib.reload(mp)
    importlib.reload(run)
    importlib.reload(context)
    importlib.reload(parsers)
    importlib.reload(commands)
    importlib.reload(command_map)
    importlib.reload(interactive)
