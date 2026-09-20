/*
 * HugeTLB
 * mmap(MAP_HUGETLB)
 * Linux HugeTLB pages are genuinely a distinct physical-memory class.
 * These pages come from the kernel's reserved HugeTLB
 * pools rather than ordinary demand-paged memory.
 */

mmap(NULL, size,
     PROT_READ | PROT_WRITE,
     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
     -1, 0);