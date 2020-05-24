Code Conventions
========================

Function Naming
---------------

**Type constructor by value:**

.. code-block::

    <type> <type>_Make(args...)

Use this for types which have no pointers and whose bits can be copied around.

Example:

.. doxygenfunction:: float2_Make

**Creating one of a limited number of resource:**

.. code-block::

    bool <type>_Create*(<type>*, args...)

Resources created and used on the fly usually have additional tracking and
limits on the number of outstanding resources.

Example:

.. doxygenfunction:: R_CreateSprite

**Setting values on a non-copyable type**

.. code-block::

    <type>_Set(<type>*, args...)

Some resources cannot be copied, but do not making allocations.

Example:

.. doxygenfunction:: PathBuffer_Set

**System initialization**

.. code-block::

    bool <SystemName>_Init(args...)

Systems may fail initialization.

Example:

.. doxygenfunction:: Log_Init

**System shutdown**

.. code-block::

    void <System>_Shutdown()

Shutdown failures are not as important.

Example:

.. doxygenfunction:: Log_Shutdown

Return value conventions
------------------------

Returning ``true`` indicates success, returning ``false`` indicates failure.

Include conventions
-------------------

- For source files, ``#include "myfile.h"`` should come first if a companion
  header exists.
- ``#include <knell/kn.h>`` should come next before any other headers.
- All Knell headers, except a companion header, should be referenced using
  angled brackets: e.g. ``<knell/log.h>``.

Comments
--------

Comment should be terminated by a period (.) to indicate that it is a complete
thought and not accidentally cut off.

Use Javadoc style ``/** */`` for documentation.

.. code-block::

