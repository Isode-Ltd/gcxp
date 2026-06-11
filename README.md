GCXP
====

This is a C++ header-only implementation of GCXP, the Guard Content eXchange Protocol. This implementation is intended to be portable across a wide range of platforms, requiring [C++14](https://en.wikipedia.org/wiki/C%2B%2B14) or better, a corresponding Standard C Library implementation, and [CBOR-lite](https://bitbucket.org/isode/cbor-lite).

The example programs and stream interface requires [Boost.Asio](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html) with TLS support provided using [OpenSSL](https://openssl.org). The test framework requires [Boost.Test](http://www.boost.org/doc/libs/release/libs/test/doc/html/index.html). Build tools used include [CMake](https://cmake.org), and [Ninja](https://ninja-build.org).

Other tools used in developing and maintaining this implementation include [LLVM](https://llvm.org) (for clang++ and clang-format) and [Git](https://git-scm.com).

  * [include/gcxp](./include/gcxp) - headers
  * [consumer](./consumer) - consumer example program
  * [producer](./producer) - producer example program
  * [unit](./unit) - unit tests

More information about GCXP is available at [https://github.com/Isode-Ltd/gcxp/wiki](https://github.com/Isode-Ltd/gcxp/wiki).

Contributing
------------

GCXP is open-source. Community contributions are welcomed. See [contributing guidelines](CONTRIBUTING.md) for more information.


Copyright
---------

See [COPYRIGHT.md](./COPYRIGHT.md) for [copyright and other legal notices](./COPYRIGHT.md).
