# Knell's Code Conventions

## Function Naming

- Type constructor by value: `<type> <type>_Make(args...)`
- Type constructor to pointer: `bool <type>_Create(<type>*, args...)`
- System initialization: `bool <SystemName>_Init(args...)`
- System shutdown: `void <System>_Shutdown()`

## Return value conventions

Returning `true` indicates success, returning `false` indicates failure.

## Include conventions

- For source files, `#include "myfile.h"` should come first if a companion
  header exists.
- `#include <knell/kn.h>` should come next before any other headers.
- All Knell headers, except a companion header, should be referenced using
  angled brackets: e.g. `<knell/log.h>`.
 