import importlib

import knell.cmake as cmake
import knell.context as context
import knell.crank as crank
import knell.multiplatform as mp
import knell.parsers as parsers
import knell.run as run


def reload():
    """
    Reloads Knell and it's associated submodules.

    Provided to improve iteration of development on Crank.
    """

    # Manually reload for now.
    # TODO: Implement a depedency trace of modules for better maintainability.
    importlib.reload(cmake)
    importlib.reload(mp)
    importlib.reload(run)
    importlib.reload(context)
    importlib.reload(parsers)
    importlib.reload(crank)
