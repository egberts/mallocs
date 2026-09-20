## Notable citations:

### Size classes, selection, & fit policies

These works concern mapping allocation requests to available sizes, choosing a fitting segment, and analyzing the consequences of those choices.

* [Kenneth C. Knowlton, A Fast Storage Allocator (1965)](https://doi.org/10.1145/365447.365454), Fast storage allocation; size-based organization
* [J. A. Campbell, A Note on an Optimal-Fit Method for Dynamic Allocation of Storage (1971)](https://doi.org/10.1093/comjnl/14.1.7), Optimal-fit selection
* [D. S. Hirschberg, A Class of Dynamic Memory Allocation Algorithms (1973)](https://doi.org/10.1145/362375.362392), Families of allocation algorithms
* [J. S. Fenton and D. W. Payne, Dynamic Storage Allocations of Arbitrary Sized Segments (1974)](https://doi.org/10.1145/800213.806592), Arbitrary-sized segment allocation
* [J. M. Robson, Worst Case Fragmentation of First Fit and Best Fit Storage Allocation Strategies (1977)](https://doi.org/10.1093/comjnl/20.3.242), First-fit and best-fit selection; fragmentation
* [M. Tadman, Fast-Fit: A New Hierarchical Dynamic Storage Allocation Technique (1978)](https://ics.uci.edu/~standish/thesessupervised.pdf), Hierarchical fit selection
* [Ivor P. Page, Optimal Fit of Arbitrary Sized Segments (1982)](https://doi.org/10.1093/comjnl/25.1.32), Optimal-fit selection for arbitrary segments
* [C. J. Stephenson, Fast Fits: New Methods for Dynamic Storage Allocation (1983)](https://doi.org/10.1145/800217.806613), Fast-fit selection mechanisms
* [R. Brent, Efficient Implementation of the First-Fit Strategy for Dynamic Storage Allocation (1989)](https://doi.org/10.1145/75334.75346), First-fit implementation
* [Wilson et al., Dynamic Storage Allocation: A Survey and Critical Review (1995)](https://www.cs.hmc.edu/~oneill/gc-library/Wilson-Alloc-Survey-1995.pdf), Survey of fit strategies and allocator designs
* [A Memory Allocator (Doug Lea, 2000)](http://gee.cs.oswego.edu/dl/html/malloc.html), Practical allocator organization and fit mechanisms
* [Heap and Allocators (2015)](http://www.cs.dartmouth.edu/~sergey/cs108/2015/heaps-and-allocators.txt), Allocator organization and size handling

### Buddy

These works concern dividing memory into allocatable units, locating related blocks, and recombining free blocks.

* [BIPOP table — span-based allocator, local and remote free lists (S. Schneider, 2006)](), Span-based allocation; local and remote free lists
* [P. W. Purdom and S. M. Stigler, Statistical Properties of the Buddy System (1970)](https://doi.org/10.1145/321607.321617), Buddy-system behavior and statistical properties
* [B. Cranston and R. Thomas, A Simplified Recombination Scheme for the Fibonacci Buddy System (1975)](https://doi.org/10.1145/360825.360854), Fibonacci buddy recombination
* [J. L. Peterson and T. A. Norman, Buddy Systems (1977)](https://doi.org/10.1145/359623.359629), Buddy-system algorithms
* [David S. Wise, The Double Buddy-System (1978)](https://hdl.handle.net/2022/33889), Double-buddy organization
* [A. G. Bromley, Memory Fragmentation in Buddy Methods for Dynamic Storage Allocation (1980)](https://doi.org/10.1007/BF00007432), Buddy fragmentation
* [A. Gottlieb and J. Wilson, Parallelizing the Usual Buddy Algorithm (1982)](), Parallel buddy allocation

### Fragmentation, algorithmic bounds, and comparative analysis

These references help evaluate allocator behavior rather than define a single mechanism. They are particularly relevant to the benchmarker's measurement and analysis responsibilities.

* [P. W. Purdom, S. M. Stigler, and Tat-Ong Cheam, Statistical Investigation of Three Storage Allocation Algorithms (1971)](), Comparative statistical evaluation
* [M. R. Garey, R. L. Graham, and J. D. Ullman, Worst-Case Analysis of Memory Allocation Algorithms (1972)](https://doi.org/10.1145/800152.804907), Worst-case analysis
* [J. M. Robson, Worst Case Fragmentation of First Fit and Best Fit Storage Allocation Strategies (1977)](https://doi.org/10.1093/comjnl/20.3.242), Worst-case fragmentation
* [E. G. Coffman, Jr., T. T. Kadota, and L. A. Shepp, On the Asymptotic Optimality of First-Fit Storage Allocation (1985)](https://doi.org/10.1109/TSE.1985.232200), Asymptotic analysis
* [Wilson et al., Dynamic Storage Allocation: A Survey and Critical Review (1995)](https://www.cs.hmc.edu/~oneill/gc-library/Wilson-Alloc-Survey-1995.pdf), Comparative review and terminology
* [Johnstone, The Memory Fragmentation Problem: Solved? (1997)](https://doi.org/10.1145/301589.286864), Fragmentation analysis
* [Risco-Martín et al., Simulation of High-Performance Memory Allocators (2024)](https://arxiv.org/pdf/2406.15776), Allocator simulation and performance evaluation

### Concurrent allocation, ownership, and synchronization

This grouping covers mechanisms for coordinating allocation and free operations across processors or threads. These are not necessarily allocation algorithms in their own right; some are synchronization or metadata techniques used by an allocator.

* [R. J. Maher, Problems of Storage Allocation in a Multiprocessor Multiprogrammed System (1961)](https://dblp.org/rec/journals/cacm/Maher61), Multiprocessor allocation concerns
* [Solaris mtmalloc (2003)](https://web.archive.org/web/20111019163341/http://developers.sun.com/solaris/articles/multiproc/multiproc.html), Multithreaded allocation
* [Treiber (1986), remote free-list encoding using a Treiber stack](), Lock-free stack / remote free-list organization
* [Afek et al. (2010), segment queue](), Concurrent queue; quasi-linearizability
* [Haas et al. (2013), multi-core distributed queue](), Distributed concurrent queue
* [Henzinger et al. (2013), k-FIFO queue](), Concurrent FIFO queue
* Process-owner encoding, Ownership metadata
* Epoch encoding, Epoch-based metadata / coordination
* [Hazard pointers — M. M. Michael (2004)](https://doi.org/10.1109/TPDS.2004.8), Safe memory reclamation for lock-free objects
* Constant-time modulo synchronization, Synchronization strategy; exact semantics to be established from the source
* Arena memory pools — CPU/core and thread variants, Locality and ownership partition

### Local allocation, caches, pools, and spans

These mechanisms reduce the cost of repeatedly allocating small objects or provide local pools of reusable memory.

* [Pool semantics — remote free-list encoding using Treiber stack (R. K. Treiber, 1986)](), Pool organization and remote deallocation
* [David R. Hanson, Fast Allocation and Deallocation of Memory Based on Object Lifetimes (1990)](https://doi.org/10.1002/spe.4380200104), Lifetime-oriented allocation
* [Hoard, A Scalable Memory Allocator for Multithreaded Applications (2000)](https://doi.org/10.1145/378993.379232), Per-processor heaps and superblock-based allocation
* [mimalloc, Free List Sharding in Action (2019)](https://doi.org/10.1007/978-3-030-34175-6_13), Page-local allocation and local/remote free paths
* [jemalloc, A Scalable Concurrent malloc(3) Implementation for FreeBSD (2006)](https://people.freebsd.org/~jasone/jemalloc/bsdcan2006/jemalloc.pdf), Arenas, bins, runs/extents, and size classes
* [TCMalloc, Thread-Caching Malloc (2005)](https://google.github.io/tcmalloc/overview.html), Thread caching and central free-space management
* BIPOP table (Schneider, 2006), Span-based allocation; local and remote free lists
* Heap-bucket size classes, Bucket-based size selection; avoiding an object header
* Single-core local allocation buffers (CLABs), Core-local allocation buffering
* Thread-specific local allocation buffers (TLABs), Thread-local allocation buffering
* Arena memory pool (CPU/core and thread, separately), Local pools and allocation ownership
* Linked-list free space, Free-space organization
* Pool semantics with remote free-list encoding, Pool organization and remote deallocation

### Memory sourcing, large allocations, and returning memory

These are primarily about where memory comes from and when it can be returned to its provider. They should not all be classified as extant-memory algorithms.

* Direct `mmap()` for large size-class blocks, Direct memory-source acquisition
* Early return to OS pool, Memory release / source interaction
* FreeBSD `madvise()`, Returning or advising about memory pages
* [Anatomy of a Program in Memory (2009)](https://manybutfinite.com/post/anatomy-of-a-program-in-memory/), Process memory layout and virtual memory context
* [The Origins of Malloc (2017)](https://www.spinellis.gr/blog/20170914/), Historical context for allocator interfaces

### Allocator implementations, source studies, and historical references

These are useful as implementation specimens for deriving primitives directly from code. They may contribute to several of the functional groupings above.

* [A Memory Allocator (Doug Lea, 2000)](http://gee.cs.oswego.edu/dl/html/malloc.html), dlmalloc design and implementation
* [Solaris mtmalloc (2003)](https://web.archive.org/web/20111019163341/http://developers.sun.com/solaris/articles/multiproc/multiproc.html), Multithreaded allocator implementation
* [Understanding glibc malloc (2015)](https://sploitfun.wordpress.com/2015/02/10/understanding-glibc-malloc), glibc allocator source study
* [GrapheneOS hardened_malloc (2019)](https://github.com/GrapheneOS/hardened_malloc), Hardened allocator implementation
* [A History of malloc (2010)](https://betathoughts.blogspot.com/2010/02/history-of-malloc.html), Historical survey
* [The Origins of Malloc (2017)](https://www.spinellis.gr/blog/20170914/), Historical context
* [Dynamic Storage Allocation: A Survey and Critical Review (1995)](https://www.cs.hmc.edu/~oneill/gc-library/Wilson-Alloc-Survey-1995.pdf), Historical and technical survey
* [Simulation of High-Performance Memory Allocators (2024)](https://arxiv.org/pdf/2406.15776), Simulation and benchmarking methodology

### Queue

* Segment queue — quasi-linearizability (Y. Afek, 2010)
* Multi-core distributed queue (A. Haas, 2013)
* k-FIFO queue (T. A. Henzinger, 2013)

### Notable malloc libraries

These are the implementations you've selected for the source-based comparison. They are rows in the allocator table, not primitive categories.

* [Hoard](https://github.com/emeryberger/Hoard), Concurrent, per-processor heap design
* [phmalloc](https://github.com/), Allocator specimen; inspect the latest source
* [mimalloc](https://github.com/microsoft/mimalloc), Page-oriented allocation and local/remote free paths
* [jemalloc](https://github.com/jemalloc/jemalloc), Arena, bin, and extent organization
* [TCMalloc](https://github.com/google/tcmalloc), Thread-cache and central-cache organization
* [glibc malloc](https://sourceware.org/glibc/), General-purpose allocator implementation
* BSD malloc, General-purpose allocator implementation

