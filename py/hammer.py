#!/usr/bin/env python3

"""
Hammer can be run interactively or a single command at a time.

The same commands which would have been run from the interactive version.
"""

import importlib
import sys
import kn


def _run_repl():
    """Run Hammer interactively, reloading it if requested."""
    while True:
        repl = kn.Hammer()
        repl.cmdloop()

        if repl.reload:
            importlib.reload(kn)
        else:
            sys.exit(repl.last_exit_code)


def _main():
    args = sys.argv[1:]
    if len(args) == 0:
        _run_repl()
    else:
        hammer = kn.Hammer()
        hammer.onecmd(' '.join(args))
        if hammer.last_exit_code == 0:
            hammer.do_save('')
        sys.exit(hammer.last_exit_code)


if __name__ == '__main__':
    _main()
