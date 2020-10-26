import importlib

import calendon.cmake as cmake
import calendon.commands as commands
import calendon.command_map as command_map
import calendon.context as context
import calendon.interactive as interactive
import calendon.multiplatform as mp
import calendon.parsers as parsers
import calendon.run as run


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
