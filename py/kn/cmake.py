"""
Integration with CMake.
"""
import subprocess
import sys


def generator_settings_for_compiler(compiler):
    """Makes settings to give the generator for a specific compiler."""
    settings = []
    if compiler != 'default' and compiler is not None:
        settings = [f'-DCMAKE_C_COMPILER={compiler}']

    # https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2015%202017.html
    if sys.platform == 'win32' and (compiler is None or compiler == 'default'):
        help_output = subprocess.check_output(['cmake', '--help'])
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
            settings.extend(['-G', generator, '-A', 'x64'])

    return settings
