REPL:

  git clone https://github.com/egberts/mallocs
  cd mallocs
  makeconfig
  make
  build/mallocs




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
