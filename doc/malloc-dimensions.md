

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
