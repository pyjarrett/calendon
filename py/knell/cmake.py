"""
Integration with CMake.
"""
from typing import Optional
import subprocess
import sys

def generator_settings_for_compiler(cmake_path: str, compiler_path: Optional[str]):
    """Makes settings to give the generator for a specific compiler."""
    settings = []
    if compiler_path is not None:
        settings = [f'-DCMAKE_C_COMPILER={compiler_path}']

    # https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2015%202017.html
    if sys.platform == 'win32':
        if compiler_path is None:
            arch = 'x64'
            help_output = subprocess.check_output([cmake_path, '--help'])
            generator = None
            for line in help_output.decode().splitlines():
                if line.startswith('*'):
                    print(line)
                    generator = line[1:line.index('=')]
                    if '[arch]' in generator:
                        generator = generator.replace('[arch]', '')
                    generator = generator.strip()
                    print(f'"{generator}"')
                    break
            if generator is not None:
                settings.extend(['-G', generator, '-A', arch])
        else:
            settings.extend(['-G', 'Unix Makefiles'])

    return settings
