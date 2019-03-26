Isode GCXP C++ Implementation
=============================

This is the Isode C++ header-only implementation of GCXP.  This
implementation is intended to be portable across a wide range of
platforms, requiring C++11 or better, a corresponding Standard C
Library implementation, and
[CBOR-lite](https://bitbucket.org/isode/cbor-lite).

The optional stream interface requires Boost [Asynchronous
I/O](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html).

The test framework requires [Boost
Test](http://www.boost.org/doc/libs/release/libs/test/doc/html/index.html),
[CMake](https://cmake.org), and [Ninja](https://ninja-build.org).
Other tools used in developing and maintaining this implementation
include [LLVM](https://llvm.org) (for clang++ and clang-format),
[FreeBSD](http://FreeBSD.org), and Git.

  * [include/gcxp](./include/gcxp) - headers
  * [unit](./unit) - unit tests

To use, add the include folder to your include search path.

To build the test code and run it, follow these steps

```
mkdir build
cd build
cmake -G Ninja ..
ninja
ninja unit-test
```

Copyright
---------

See [COPYRIGHT.md](./COPYRIGHT.md) for [copyright and other legal notices](./COPYRIGHT.md).
