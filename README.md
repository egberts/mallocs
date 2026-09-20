# mallocs

ALPHA release, not for general usage.  Still in discrete breakdown stage.

## Intro
  The goal is to breakdown `malloc()`/`free()` into discrete
  components, making all variants independently available for
  combinatorial benchmarkings.

## Overview
    Breakdown into discrete components enables the following
    philosophy of compositional approach to experimenting
    and testing new `malloc()` variants:


```text
             memory-source
                  |
                  v
             extent-manager
                  |
                  v
             block-allocator
                  |
                  v
                malloc()
```

The components are organized by their functional responsibilities:

- **memory-source**: Supplies preallocated memory.
- **extent-manager**: Manages large regions of memory.
- **block-allocator**: Manages fixed-size allocatable objects.

These are functional distinctions. They are not requirements that every
allocator must use all three components.

## Design

The project separates configuration, implementation, and testing.

- **Kconfig**: Packaging, primitive availability, and parameters.
- **Primitive implementation**: The underlying allocation mechanisms.
- **tester.c**: Composition, invocation, permutation, and measurement.

Kconfig makes the primitives available for experimentation.

New primitive can be introduced with relative ease.

`tester.c` determines how those primitives are composed and tested.

## REPL

A typical build and test cycle:

```sh
git clone https://github.com/egberts/mallocs
cd mallocs
makeconfig
make
build/mallocs
```
