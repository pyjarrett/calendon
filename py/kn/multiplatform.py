"""
File conversions to support multiplatforms.
"""

import os
import sys


def demo_glob():
    """Produce a glob suitable for identifying demo shared libraries."""
    if sys.platform == 'win32':
        return '*.dll'

    return 'lib*.so'


def root_to_executable(named):
    """Produce an executable file name from the generic basename."""
    if sys.platform == 'win32':
        return named + '.exe'

    return named


def root_to_shared_lib(named):
    """Give a shared library with appropriate prefix/suffix from a basename."""
    if sys.platform == 'win32':
        return named + '.dll'

    return 'lib' + named + '.so'


def shared_lib_to_root(shared_lib: str):
    """
    Converts a name from an OS-specific shared library name to the generic name.
    """
    if sys.platform == 'win32':
        if not shared_lib.endswith('.dll'):
            raise ValueError('Shared library provided without .dll suffix.')
        return os.path.splitext(shared_lib)[0]

    if not shared_lib.startswith('lib') or shared_lib.endswith('.so'):
        raise ValueError(f'Shared lib does not match "lib*.so"')

    return shared_lib[3:-3]