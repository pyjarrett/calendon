#!/usr/bin/env python3
import importlib
import kn
import sys


def reload_hammer():
    importlib.reload(kn)


def run_repl():
    while True:
        toll = kn.Hammer(reload_hammer)
        toll.cmdloop()

        if not toll.reload:
            sys.exit(toll.last_exit_code)


if __name__ == '__main__':
    args = sys.argv[1:]
    if len(args) == 0:
        run_repl()
    else:
        toll = kn.Hammer(reload_hammer)
        toll.onecmd(' '.join(args))
        if toll.last_exit_code == 0:
            toll.do_save('')
        sys.exit(toll.last_exit_code)
