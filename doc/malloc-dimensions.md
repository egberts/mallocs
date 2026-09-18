
if it manages large regions:
    extent-manager

if it manages fixed-size allocatable objects:
    block-allocator

if it merely supplies preallocated memory:
    memory-source

```
                    malloc permutation
                           │
          ┌────────────────┼────────────────┐
          │                │                │
      acquisition       placement       reclamation
          │                │                │
       mmap()          buddy           munmap()
       sbrk()          fibonacci       madvise()
       arena           double          retain
       slab            segregated
                      ...
```

