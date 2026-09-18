
```
MEMORY SOURCE
    │
    ├── VM-backed
    │   ├── mmap anonymous
    │   ├── mmap file
    │   ├── shm
    │   ├── memfd
    │   └── huge pages
    │
    ├── pre-existing virtual memory
    │   ├── static
    │   ├── stack
    │   └── caller supplied
    │
    ├── physical/device-backed
    │   ├── device mapping
    │   ├── DAX
    │   ├── PMEM
    │   └── raw physical
    │
    └── external provider
        ├── shared pool
        ├── foreign allocator
        └── callback
```
