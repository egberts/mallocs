REPL:

  git clone https://github.com/egberts/mallocs
  cd mallocs
  makeconfig
  make
  build/mallocs

OVERVIEW

if it manages large regions:
    extent-manager

if it manages fixed-size allocatable objects:
    block-allocator

if it merely supplies preallocated memory:
    memory-source




             memory_source
                  │
                  ▼
             extent manager
                  │
                  ▼
            block allocator
                  │
                  ▼
              malloc()
