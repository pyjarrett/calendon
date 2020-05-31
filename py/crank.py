#!/usr/bin/env python3
"""
Entry point for running Calendon's helper tool, "crank", interactively or
as a script.
"""
import sys

import calendon.context as context
import calendon.interactive as interactive
import calendon.script as script
from calendon import reload

# This tool is runnable as a script, providing the appropriate command and
# options to use.  Running commands individually should be indistinguishable
# from using an interactive environment.
if __name__ == '__main__':
    if len(sys.argv) == 1:
        # Establish the target environment for the script.
        calendon_home: str = context.default_calendon_home()

        # Build the context using the given home directory.
        ctx: context.ProjectContext = context.ProjectContext(calendon_home)

        while True:
            repl = interactive.InteractiveMode(ctx)
            repl.cmdloop()

            if repl.reload:
                print('Reloading Crank.')
                reload()
            else:
                sys.exit(0)

    sys.exit(script.run_as_script())
