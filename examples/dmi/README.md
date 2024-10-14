# DMI interface example (`dmi`)

On Linux, the MappedMemory peripheral can be configured to use a region of
memory that can be shared with another process via a memory mapped file (MMF).

In order to do the shared memory allocation, the MappedMemory instance
requires the sharedMemory constructor attribute to be set to the directory
where the MMFs will be created.  On Ubuntu Linux the appropriate setting
is "/dev/shm", which is a block of memory mounted as a tmpfs file system.
The filenames will then be of the form /dev/shm/renode-sharedSegment-\*

When MappedMemory has the sharedMemory attribute set, the SystemC process
can request a pointer by using get\_direct\_mem\_ptr().  When this method
is called, the renode bridge passes the MMF filename to the SystemC
process, so that the SystemC process can map the file to memory and perform
reads and writes directly to the shared memory.

At the end of simulation, the MMFs will be deleted.  Note that if renode
crashes mid-simulation, the cleanup may not occur, so it is advisable to
manually remove leftover files to free up system memory.  This can be
accomplished with:
```
rm /dev/shm/renode-sharedSegment-*
```
