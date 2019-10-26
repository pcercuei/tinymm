# tinymm
Tiny memory manager for tiny data

This project is a very simple memory manager for small data.

Why?
----

When allocating dynamically a lot of small structures, for instance in a linked list, calling the system's malloc() function for each allocation may result in a lot of system calls. To counter that, this library allocates bigger chunks of memory on demand, to reduce the number of calls to malloc().
