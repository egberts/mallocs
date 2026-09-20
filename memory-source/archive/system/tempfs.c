/*
 * RAM and/or swap
 tmpfs is particularly interesting because its pages
 are RAM/swap-backed but represented through the
 filesystem/shmem machinery.

 For example:

    mmap(... MAP_SHARED ...);

against a tmpfs object gives a different provenance
from anonymous memory even though the actual physical
pages may ultimately be ordinary RAM.
*/