# jmprof

A tiny heap profiler for GNU/Linux.

## Summary

### Preloading

- The dynamic linker (`ld.so`, `ld-linux.so*`) find and load the shared objects (shared libraries) needed by a program, prepare the program to run, and then run it. 
- There is an important environment variable for the dynamic linker called `LD_PRELOAD`, which is a list of additional, user-specified, shared libraries **to be loaded before all others.**
- We can leverage `LD_PRELOAD` to inject custom library code into any applications, allowing us to intercept (or override) the `*libc` function calls.
- In GNU C Library (glibc), `dlsym()` internally calls `calloc()`, which will lead to an infinite recursion if we try to retrieve the address of `calloc()` with it. Therefore, we need to use `__libc_calloc()` as the address of `calloc()`.
- Infinite recursion can also occur whenever we use external library functions that call `*alloc()`. In order to prevent this from happening, we can use a thread-local handle guard (a `pthread_key_t` variable) for each `*libc` memory allocation function.

### Stack Unwinding

- We need to **unwind** the stack to get a backtrace.
- In order to unwind the stack from within a running program (**local** unwinding), we can use `libunwind` with the macro `UNW_LOCAL_ONLY` defined.

### Symbol Resolution

- Each row in `/proc/$PID/maps` (`/fs/proc/base.c` in the GNU/Linux kernel source) describes a region of contiguous virtual memory in a process or thread.
- By finding the first address for `/proc/$PID/maps` that has mapping which includes the instruction pointer address, we can resolve the symbol that corresponds to the mapped address.
- The `libdwfl` library from `elfutils` can read DWARF, find and interpret debug information (the `.debug_info` section of an ELF), allowing us to perform symbol resolution in an easier way.

## Prerequisites

- GCC version 11.4.0+
- Git version 2.34.0+
- GNU coreutils version 8.3+
- GNU Make version 4.2+
- elfutils-dev(el) version 0.190+
- libunwind-dev(el) version 1.8.1+

```console
$ sudo xbps-install base-devel elfutils-devel libunwind-devel
```

## Building

```console
$ make && sudo make install
$ jmprof <your-program>
```

## References

- [Conrod, Jay. “Understanding Linux `/proc/pid/maps` or `/proc/self/maps`.” stackoverflow.com. September 09, 2009.](https://stackoverflow.com/a/1401595)
- [Nethercote, Nicholas & Seward, Julian. “How to Shadow Every Byte of Memory Used by a Program.” Third International ACM SIGPLAN/SIGOPS Conference on Virtual Execution Environments, San Diego, California, USA. June 13-15, 2007.](https://valgrind.org/docs/shadow-memory2007.pdf)
- [Nethercote, Nicholas & Seward, Julian. “Valgrind: A Framework for Heavyweight Dynamic Binary Instrumentation.” ACM SIGPLAN 2007 Conference on Programming Language Design and Implementation, San Diego, California, USA. June 13-15, 2007.](https://valgrind.org/docs/valgrind2007.pdf)
- [Picard, Romain. “`LD_PRELOAD` for real world heap access tracking.” blog.oakbits.com. April 11, 2012.](https://blog.oakbits.com/ld_preload-for-real-world-heap-access-tracking.html)
- [Wolff, Milian. “How to Write a Heap Profiler.” CppCon 2019, Aurora, Colorado, USA. September 15-20, 2019.](https://github.com/milianw/how-to-write-a-memory-profiler)

## License

MIT License

> Copyright (c) 2024 Jaedeok Kim <jdeokkim@protonmail.com>
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.
