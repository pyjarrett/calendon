Code Conventions
========================

Prefixing structs, macros and functions
---------------------------------------

Calendon uses prefixes to avoid namespace clashes like the Vulkan library.  Use
the ``cn`` prefix but with different cases between different types.

**Structs**

Prefix structs with ``Cn`` (capital).

.. code-block::

    Cn<StructNameInPascalCase>

Example:

.. code-block::

    CnDriverConfig

**Functions**

Prefix functions with ``cn`` (lower-case).

.. code-block::

    cn<System>_ActionInPascalCase

    cn<Type>_ActionInPascalCase

Example:

.. code-block::

    // Driver Init(ialization)
    cnDriver_Init

    // CnFloat4x4 non-uniform scaling
    cnFloat4x4_NonUniformScale

**Macros**

Prefix macros with ``CN`` (ALL CAPS).

.. code-block::

    CN_<MACRO_NAME_IN_ALL_CAPS>

Example:

.. code-block::

    CN_ASSERT

    CN_FATAL_ERROR

**ENUMS**

Prefix enum values with ``CN`` (ALL CAPS).

.. code-block::

    CN_<ENUM_TYPE>_<VALUE>

Example:

.. code-block::

    enum {
        CN_LOG_ERROR,
        CN_LOG_TRACE,
        //...
    };

Prefix enum types with `Cn` (capitalize)

.. code-block::

    typedef enum {
    } Cn<EnumName>;

Example:

.. code-block::

    CnUtf8ByteValidity

**Header Guards**

Prefix header guards with ``CN_`` and use a ``_H`` suffix.

.. code-block::

    CN_ENV_H


Function Naming
---------------

**Type constructor by value:**

.. code-block::

    <type> <type>_Make(args...)

Use this for types which have no pointers and whose bits can be copied around
while retaining their original meaning.

Example:

.. doxygenfunction:: cnFloat2_Make

**Creating one of a limited number of resource:**

.. code-block::

    bool cn<type>_Create*(<type>*, args...)

Resources created and used on the fly usually have additional tracking and
limits on the number of outstanding resources.

Example:

.. doxygenfunction:: cnR_CreateSprite

**Setting values on a non-copyable type**

.. code-block::

    cn<type>_Set(<type>*, args...)

Some resources cannot be copied, but do not making allocations.

Example:

.. doxygenfunction:: cnPathBuffer_Set

**System initialization**

.. code-block::

    bool cn<SystemName>_Init(args...)

Systems may fail initialization.

Example:

.. doxygenfunction:: cnLog_Init

**System shutdown**

.. code-block::

    void cn<System>_Shutdown()

Shutdown failures are not as important.

Example:

.. doxygenfunction:: cnLog_Shutdown

Return values
------------------------

Returning ``true`` indicates success, returning ``false`` indicates failure.

Primitive Types
---------------

- Prefer ``NULL`` to ``0``
- Prefer ``float`` over ``double``.
- Used sized versions of integers and macro format specifiers:

+-------------------+-----------------+
| Integer Types     |  Format         |
+===================+=================+
| ``int8_t``        |  ``PRIi8``      |
+-------------------+-----------------+
| ``int16_t``       |  ``PRIi16``     |
+-------------------+-----------------+
| ``int32_t``       |  ``PRIi32``     |
+-------------------+-----------------+
| ``int64_t``       |  ``PRIi64``     |
+-------------------+-----------------+
| ``uint8_t``       |  ``PRIu8``      |
+-------------------+-----------------+
| ``uint16_t``      |  ``PRIu16``     |
+-------------------+-----------------+
| ``uint32_t``      |  ``PRIu32``     |
+-------------------+-----------------+
| ``uint64_t``      |  ``PRIu64``     |
+-------------------+-----------------+

- Use ``size_t`` for types which must deal with memory sizes or offsets.

#include's
-------------------

- For source files, ``#include "myfile.h"`` should come first if a companion
  header exists.
- ``#include <calendon/cn.h>`` should come next before any other headers.
- All Calendon headers, except a companion header, should be referenced using
  angled brackets: e.g. ``<calendon/log.h>``.

Documentation and Comments
--------------------------

Comment should be terminated by a period (.) to indicate that it is a complete
thought and not accidentally cut off.

Use Javadoc style ``/** */`` for documentation with "space-star-space" on
intermediate lines.  Functions should be documented in source (``.c``) files to
keep their documentation close to the code.  Macros, structs, and typedef must
be documented at their definition sites.

.. code-block::

    /**
     * Errors are serious issues which must be addressed prior to shipping a product
     * and fixed as soon as possible when detected in release.
     */
    #define CN_ERROR(system, msg, ...) \
        CN_LOG(system, CN_LOG_ERROR, msg, ##__VA_ARGS__); \
        CN_DEBUG_BREAK()
