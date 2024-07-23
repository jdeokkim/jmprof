# jmprof

A tiny, experimental heap profiler for GNU/Linux.

## Prerequisites

- GCC version 11.4.0+
- Git version 2.34.0+
- GNU coreutils version 8.3+
- GNU Make version 4.2+
- elfutils-dev(el) version 0.190+
- libpfm4-dev(el) version 4.13.0+
- libunwind-dev(el) version 1.8.1+

<!-- - libsanitizer-devel 13.2.0+ -->

### Void Linux

```console
$ sudo xbps-install base-devel elfutils-devel libpfm4-devel libunwind-devel
```

## Building

```console
$ make && sudo make install
```

## Example

```
$ jmprof uname -a
jmprof: info: creating a named pipe '/tmp/jmprof-fifo.7086'
jmprof: info: intercepting `*alloc()` calls via /usr/lib/libjmprof.so

<==============================================================================

Linux void-dgist 6.6.40_1 #1 SMP PREEMPT_DYNAMIC Mon Jul 15 20:29:49 UTC 2024 x86_64 GNU/Linux

==============================================================================>

jmprof v0.0.6-dev by Jaedeok Kim (jdeokkim@protonmail.com)

> /usr/bin/uname

SUMMARY: 
  30 allocs, 2 frees (8592 bytes alloc-ed)

  ~ alloc #2 (! 519242 ms) -> [120 bytes @ 0x55b6c00102c0]: 
    @ 0x7fd55d7517a6: malloc (src/preload.c:147:9)
      (in /usr/lib/libjmprof.so)
    @ 0x7fd55d58d9c4: _nl_load_locale_from_archive (./locale/loadarchive.c:460:9)
      (in /usr/lib/libc.so.6)
    @ 0x7fd55d58cf47: _nl_find_locale (./locale/findlocale.c:153:10)
      (in /usr/lib/libc.so.6)
    @ 0x7fd55d58f8d7: setlocale (./locale/setlocale.c:337:24)
      (in /usr/lib/libc.so.6)
    @ 0x55b6bf36c38d: ?? (:0:0)
      (in /usr/bin/uname)
    @ 0x7fd55d581c4c: __libc_start_call_main (../sysdeps/nptl/libc_start_call_main.h:74:3)
      (in /usr/lib/libc.so.6)
    @ 0x7fd55d581d05: __libc_start_main@@GLIBC_2.34 (../csu/libc-start.c:128:20)
      (in /usr/lib/libc.so.6)
    @ 0x55b6bf36c751: ?? (:0:0)
      (in /usr/bin/uname)

(...)

  ~ alloc #29 (! 851615 ms) -> [24 bytes @ 0x55b6c00114d0]: 
    @ 0x7fd55d7517a6: malloc (src/preload.c:147:9)
      (in /usr/lib/libjmprof.so)
    @ 0x7fd55d5fa7fa: __strdup (./string/strdup.c:44:6)
      (in /usr/lib/libc.so.6)
    @ 0x7fd55d594488: textdomain (./intl/textdomain.c:94:20)
      (in /usr/lib/libc.so.6)
    @ 0x55b6bf36c3a4: ?? (:0:0)
      (in /usr/bin/uname)
    @ 0x7fd55d581c4c: __libc_start_call_main (../sysdeps/nptl/libc_start_call_main.h:74:3)
      (in /usr/lib/libc.so.6)
    @ 0x7fd55d581d05: __libc_start_main@@GLIBC_2.34 (../csu/libc-start.c:128:20)
      (in /usr/lib/libc.so.6)
    @ 0x55b6bf36c751: ?? (:0:0)
      (in /usr/bin/uname)

jmprof: info: cleaning up
```

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

## References

- [Bakhvalov, Denis. “Advanced profiling topics. PEBS and LBR.” easyperf.net. June 08, 2018.](https://easyperf.net/blog/2018/06/08/Advanced-profiling-topics-PEBS-and-LBR)
- [Brais, Hadi. “An Introduction to the Cache Hit and Miss Performance Monitoring Events.” hadibrais.wordpress.com. March 26, 2019.](https://hadibrais.wordpress.com/2019/03/26/an-introduction-to-the-cache-hit-and-miss-performance-monitoring-events/)
- [Conrod, Jay. “Understanding Linux `/proc/pid/maps` or `/proc/self/maps`.” stackoverflow.com. September 09, 2009.](https://stackoverflow.com/a/1401595)
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
