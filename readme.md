# libonionrequests


c++ onion requests library with python nodejs and jvm wrapper libraries.

[![Build Status](https://ci.oxen.rocks/api/badges/majestrate/libonionrequests/status.svg?ref=refs/heads/dev)](https://ci.oxen.rocks/majestrate/libonionrequests)

## usage


building the library:

    $ mkdir -p build
    $ cd build
    $ cmake ..
    $ make

building docs:

    $ make doc
    $ python3 -m http.server -d docs/html

then browse to `http://127.0.0.1:8000`
