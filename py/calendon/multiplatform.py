"""
File conversions to support multiplatforms.
"""

from typing import Optional
import os
import subprocess
import sys


def shared_lib_glob():
    """Produce a glob suitable for identifying demo shared libraries."""
    if sys.platform == 'win32':
        return '*.dll'

    return 'lib*.so'


def static_lib_glob():
    """Produce a glob suitable for identifying static libraries."""
    if sys.platform == 'win32':
        return '*.lib'

    return '*.a'


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
        raise ValueError('Shared lib does not match "lib*.so"')

    return shared_lib[3:-3]


def file_opener() -> str:
    """The shell program to use to open a file with the default program."""
    return {'linux': 'xdg-open', 'win32': 'start'}[sys.platform]


def open_file(filename: str):
    """Opens a file using a default program."""
    subprocess.Popen([file_opener(), filename], shell=False)


def recommended_executable(program_name: str) -> Optional[str]:
    """Shows the program which would be executed based on path."""
    if sys.platform == 'win32':
        program_locator = 'where'
    else:
        program_locator = 'which'

    try:
        return subprocess.check_output([program_locator, program_name]).decode().strip()
    except subprocess.CalledProcessError:
        print(f'Unable to find program to execute for: {program_name}')
        return None
