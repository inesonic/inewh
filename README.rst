=====
inewh
=====
The inewh library provides a simple webhook interface compatible with the
`inerest_api_in_v1 <https://github.com/inesonic/inerest_api_in_v1>` library.

The API is designed to be more minimalist than the similar
`inerest_api_out_v1 <https://github.com/inesonic/inerest_api_out_v1>` library
and is designed for cases where you only need to access a single endpoint on
a limited basis.

The library is currently used as part of **Aion**, supported and sold by
`Inesonic, LLC <https://inesonic.com>` to send optional and anonymized usage
statistics.


Licensing
=========
This library is released under the MIT license.


Dependencies And Building
=========================
The library is Qt based and is built using either the qmake or cmake build
tool.  You will need to build the library using a recent version of Qt 5.  The
library has also been tested against Qt 6.

The library also depends on the inecrypto library.


qmake
-----
To build inerest_api_in_v1 using qmake:

.. code-block:: bash

   cd inecrypto
   mkdir build
   cd build
   qmake ../inecrypto.pro INECRYPTO_INCLUDE=<path to inecrypto headers>
   make

If you wish to create a debug build, change the qmake line to:

.. code-block:: bash

   qmake ../inecrypto.pro CONFIG+=debug
   make

Note that the qmake build environment currently does not have an install target
defined and will alway build the library as a static library.


cmake
-----
To build inerest_api_out_v1 using cmake:

.. code-block:: bash

   cd inerest_api_out_v1
   mkdir build
   cmake -B. -H.. -DCMAKE_INSTALL_PREFIX=/usr/local/
   make

To install, simply run

.. code-block:: bash

   make install

You can optionally also include any of the following variables on the cmake
command line.

+-------------------+--------------------------------------------------------+
| Variable          | Function                                               |
+===================+========================================================+
| inewh_TYPE        | Set to ``SHARED`` or ``STATIC`` to specify the type of |
|                   | library to be built.   A static library will be built  |
|                   | by default.                                            |
+-------------------+--------------------------------------------------------+
| INECRYPTO_INCLUDE | You can set this variable to indicate the location of  |
|                   | the inecrypto header files.  This variable only needs  |
|                   | to be set on Windows or if the headers are in a        |
|                   | non-standard location.                                 |
+-------------------+--------------------------------------------------------+
| INECRYPTO_LIB     | You can set this variable to indicate the full path    |
|                   | to the inecrypto static or shared library.  This       |
|                   | variable is only needed on Windows, if the library     |
|                   | is in a non-standard location, or if cmake can-not     |
|                   | locate the library after setting the                   |
|                   | ``INECRYPTO_LIBDIR`` variable.                         |
+-------------------+--------------------------------------------------------+
| INECRYPTO_LIBDIR  | You can use this variable to add one or more           |
|                   | directories to the inecrypto library search path.      |
|                   | Separate paths with spaces.                            |
+-------------------+--------------------------------------------------------+

Note that, at this time, the cmake environment does not include support for
testing.


Inesonic REST API Message Format
================================
For details on the supported message format, please see the documentation for
the `inerest_api_in_v1 <https::github.com/inesonic/inerest_api_in_v1>` library.
