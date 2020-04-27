"""
Entry point for running Knell's helper tool, "crank", interactively or
as a script.
"""
import sys

from knell import crank, reload

# This tool is runnable as a script, providing the appropriate command and
# options to use.  Running commands individually should be indistinguishable
# from using an interactive environment.
if __name__ == '__main__':
    if len(sys.argv) == 1:
        # Establish the target environment for the script.
        knell_home: str = crank.default_knell_home()

        # Build the context using the given home directory.
        ctx: crank.ProjectContext = crank.ProjectContext(knell_home)

        while True:
            repl = crank.InteractiveMode(ctx)
            repl.cmdloop()

            if repl.reload:
                print('Reloading Crank.')
                reload()
            else:
                sys.exit(0)

    sys.exit(crank.run_as_script())
