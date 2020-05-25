Crank
=====

``Crank`` is a cross-platform command line (CLI) tool to make common operations
on multiple platforms consistent.  You should be able to do nearly everything
Crank does manually, it's more of a support tool to simplify development on
Knell.  Crank is not required to do development when using Knell as a library.

The Commands Map
----------------

Crank loads its commands at runtime from the command map.
Commands get specified in the ``COMMANDS_DESCRIPTION`` with parsers and
commands in the form of ``parser_*`` and ``cmd_*`` in the ``knell.parser``
and ``knell.commands`` modules.

.. autodata:: knell.command_map.COMMANDS_DESCRIPTION
