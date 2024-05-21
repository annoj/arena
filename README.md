# Arena Allocator

This is a very simple implementation of an arena allocator. Arena allocators
can basically be used to namespace memory allocations and thereby simplify
memory management.

## WARNING

THIS ARENA ALLOCATOR IS UNDER ACTIVE DEVELOPMENT AND SUBJECT TO CHANGE, DO NOT
USE IN SERIOUS PROJECTS!

## STB Library

This is a single header file library, like https://github.com/nothings/stb.

## Quickstart

To use the arena allocator, just copy the arena.h header file to your source
tree, and include the header.  
You should choose exactely ONE (!) source file, where you not only include
arena.h, but also define the macro `ARENA_IMPLEMENTATION`. This is where the
arena implementation will be inserted into your source.

```c
#define ARENA_IMPLEMENTATION
#include "arena.h"
```

