Building
========

Required Tools
--------------

Everywhere:

- **cmake v3.10** -- https://cmake.org/download/
    - Generates build configurations 

- **git** -- https://git-scm.com/downloads
    - For source control

- **python 3.7** -- https://www.python.org/downloads/
    - For the project's build/test front-end, ``crank``.

Linux:

- **SDL 2.0 libraries**

- **ccache** (optional, improves build speeds)

Mac:

- I don't have a Mac to test on so I don't know if Knell will even build on
  this platform.  If you're willing to help with Mac support, let me know!

Building and running the demos
------------------------------

``crank`` is a python wrapper for ``cmake`` and other tools, so people on different
platforms or with different IDEs, can do builds and run tests in the same way.
It also requires programs to register themselves so you know what tools your
build is using.  The only registered tool required to do a normal build is
``cmake``.

Your installation paths might be slightly different than those given here.


Windows, using powershell:

.. code-block:: powershell

    py/crank.py register cmake "C:/Program Files/CMake/bin/cmake.exe" 

Linux:

.. code-block:: bash

    py/crank.py register cmake /usr/bin/cmake

Now you can generate your build files and build

.. code-block:: powershell

    py/crank.py gen
    py/crank.py build

and run demos:

.. code-block:: powershell

    py/crank.py run planets
    py/crank.py run koch-curve
    py/crank.py run graphics-sample

Testing
-------

.. code-block:: powershell

    py/crank.py check

Help
----

.. code-block:: powershell

    py/crank.py --help
