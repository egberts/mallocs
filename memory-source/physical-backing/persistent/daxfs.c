/*
 * DAX filesystem
 * persistent memory/device memory, directly mapped
 *
 * With DAX, userspace can map storage/device memory
 * directly without the normal page-cache path.
 * Depending on the hardware/filesystem, this can
 * represent persistent memory such as NVDIMM.
 */