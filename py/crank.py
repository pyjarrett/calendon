"""
Entry point for running Knell's helper tool, "crank", interactively or
as a script.
"""
import sys

from knell import crank

# This tool is runnable as a script, providing the appropriate command and
# options to use.  Running commands individually should be indistinguishable
# from using an interactive environment.
if __name__ == '__main__':
    if len(sys.argv) == 1:
        crank.run_interactive_loop()
        sys.exit(0)

    sys.exit(crank.run_as_script())
