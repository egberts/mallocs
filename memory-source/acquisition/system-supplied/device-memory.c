/*
 * Device memory
 * mmap() on device driver
 * device-specific physical memory
 * A device driver can expose physical memory to userspace through mmap().
 *     * PCI BAR memory
 *     * GPU memory
 *     * FPGA memory
 *     * NIC/device memory
 *     * accelerator memory
 *
 * Categorized differently as:
 *    * system RAM
 *    * device RAM
 *    * persistent memory
 */