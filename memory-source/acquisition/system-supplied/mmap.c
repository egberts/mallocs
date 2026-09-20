/*
 *  Locked anonymous memory
 * mmap() + mlock()
 * Ordinary RAM, but prevented from swapping
 * Linux can expose physical address ranges through mechanisms such as /dev/mem, subject to kernel configuration and access restrictions.

That's a special case:

physical address
       ↓
   mmap()
       ↓
user virtual address

It is fundamentally different from asking the VM subsystem for anonymous pages.

I would treat this as physical-address memory, not ordinary RAM.
 */