== Intro ==
  Goal is to breakdown malloc()/free() into discrete
  components and make all variants available for
  combinatorial benchmarkings.

== Overview ==
    Breakdown into discrete components enables the following
    philosophy of testing new malloc variants:


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

        if it manages large regions:
            extent-manager

        if it manages fixed-size allocatable objects:
            block-allocator

        if it merely supplies preallocated memory:
            memory-source

== Design ==

    Kconfig
        = packaging / availability / parameters

    primitive implementation
        = mechanism

    tester.c
        = composition / invocation / permutation / measurement

== REPL ==
Typical build and test cycle (REPL) comprises of:

  git clone https://github.com/egberts/mallocs
  cd mallocs
  makeconfig
  make
  build/mallocs

REPL:

  git clone https://github.com/egberts/mallocs
  cd mallocs
  makeconfig
  make
  build/mallocs

