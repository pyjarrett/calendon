Glossary
========

Calendon-specific terminology
-----------------------------

These high-level terms may be needed to understand concepts within Calendon and
to communicate clearly and effectively with other developers.  Lower level
concepts should be documented in the source code itself.

.. glossary::

    asset
        Outside data provided as an input to a program.  This term is
        independent of storage location.  e.g. an image asset could
        either be loaded from a file or from an archive.

    component
         A header (.h) file with an associated source (.c) file.

    Calendon
        Refers to the overall project of building a 2D game engine when not
        clarified.

    Calendon driver
        An executable provided by Calendon that will start and load a payload
        as given in the command line.

    Calendon library
        The main shared or static library created by building the Calendon
        project.

    Calendon testable library
        The main shared or static library created by building the Calendon
        project, which also contains functions exported specifically for
        internal testing.

    crank
        A command-line tool included in Calendon to simplify building and
        testing Calendon across platforms.

    demo
        Payloads created while developing Calendon to design or demonstrate
        features.  These are not guaranteed to work and are subject to change
        without notice.

    export
        The assembly of Calendon header files and library to a location
        suitable for use as a cohesive unit as a library by another program.

    freestanding program
        A program which links to the Calendon library and provides its own main
        entry point.

    driven program
        A payload loaded and run by the Calendon driver.

    payload
        A game, demo or tool shared library (or dynamic link library)
        which implements the basic interface for initializing, ticking, drawing
        and shutting down.

    plugin
        A piece of code which implements the basic interface for initializing,
        ticking, drawing and shutting down.  Plugins might not be loaded from
        shared libraries and be assembled within a running program, such as
        using function pointers.

    system
        A logical set of code, which may include more than one component.
        Systems have a common prefix (e.g. ``cnLog_*``) and usually have an
        initialization (``*_Init``) and a shutdown (``*_Shutdown``) function.

    tick
        A time-based update.  Prefer "tick" to "update."

    type
        A struct, enum, primitive type or type alias.
