#!/usr/bin/env python3
import importlib
import kn


def reload_hammer():
    importlib.reload(kn)

import sys
if __name__ == '__main__':
    while True:
        toll = kn.Hammer(reload_hammer)
        toll.cmdloop()

        if not toll.reload:
            break
