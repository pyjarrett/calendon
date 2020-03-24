#!/usr/bin/env python3
import importlib
import kn


def reload_knife():
    importlib.reload(kn)

import sys
if __name__ == '__main__':
    while True:
        knife = kn.Knife(reload_knife)
        knife.cmdloop()

        if not knife.reload:
            break
