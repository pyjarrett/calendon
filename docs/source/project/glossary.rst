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

    core system
        Any of the built-in systems initialized prior to other systems to
        provide basic functionality.
          - Crash dumps    (Crash)
          - Logging        (Log)
          - Memory         (Mem)
          - Time           (Time)
          - Configuration  (Config)
          - User Interface (UI)
          - Rendering      (R)

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
        loaded at runtime by the driver which implements the system interface.

    system
        A logical set of code, which may include more than one component.
        Systems have a common prefix (e.g. ``cnLog_*``) and describe a method
        of configuration, lifecycle, and behavior.

        Systems might be built in, such as the core systems (e.g. Assets,
        Memory), loaded from shared libraries, or assembled within a running
        program with function pointers.

    tick
        A time-based update.  Prefer "tick" to "update."

    type
        A struct, enum, primitive type or type alias.
