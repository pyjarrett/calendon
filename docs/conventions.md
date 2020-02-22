# Knell's Code Conventions

## Function Naming

- Type constructor by value: `<type> <type>_Make(args...)`
- Type constructor to pointer: `bool <type>_Create(<type>*, args...)`
- System initialization: `bool <SystemName>_Init(args...)`
- System shutdown: `void <System>_Shutdown()`

## Return value conventions

Returning `true` indicates success, returning `false` indicates failure.
