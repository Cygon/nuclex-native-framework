Nuclex.Support.Native License
=============================

This library is licensed under the IBM Common Public License (CPL) v1.0,
https://www.ibm.com/developerworks/library/os-cpl.html

Not to be mistaken for the GNU Public License (GPL).

You can:
--------

- Use this library in commercial applications.
- Use this library in closed-source applications.

- Distribute this library's unmodified source code.
- Distribute binaries compiled from the unmodified source code.

- Modify the source code of this library and keep the changes to yourself
  so long as you also keep the compiled binaries to yourself.

You can not:
------------

- Make changes to this library and then publish the compiled binaries
  without also sharing the changes you made to this library's source code.

- Sue anyone for issues you have with this code.

- Pretend that you wrote this or use the names of any of this library's
  authors to promote your own work.


Attribution
===========

If you distribute binaries of this library, you should include this license
file somewhere in your documentation or other legal text.

A mention of your product's use of this library as well as of the embedded
third-party libraries in your splash screen, credits or such would be nice,
but is not required.


Example Attribution Text
------------------------

Uses supporting code from the Nuclex.Support.Native library, written by
Markus Ewald and licensed under the terms of the IBM Common Public License
(https://opensource.org/licenses/cpl1.0.php)

Nuclex.Support.Native also embeds additional libraries, belonging to their
respective owners and used according to their respective licenses:

  * The Dragon 4 floating point formatter under the ZLib license
  * The Erthink integer formatter unter the Apache 2.0 license
  * The Ryu floating point formatter under the Apache 2.0 license
  * Multi-threaded queues under the Boost Software license


Third-Party Code
================

This library also embeds carefully selected third-party code which falls
under its own licenses, listed below


Dragon 4 String Formatter: ZLib License
---------------------------------------

The Dragon 4 String Formatter is used to accurately convert floating-point
values into strings. It is written by Ryan Juckett and licensed under
the ZLib license (https://www.zlib.net/zlib_license.html)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


Arturo Martin-de-Nicolas Integer Formatter: MIT License
-------------------------------------------------------

The integer formatter from the itoa library is used to convert integral
values into strings. It is written by Arturo Martin-de-Nicolas and licensed
under MIT license (https://mit-license.org/)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


James Edward Anhalt III Integer Formatter: MIT License
------------------------------------------------------

The integer formatter from the itoa library is used to convert integral
values into strings. It is written by James Edward Anhalt III and licensed
under MIT license (https://mit-license.org/)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


Ryu String to Float Parser: Apache 2.0 License
---------------------------------------------=

Code from the Ryu library is used to convert strings to floating point values
independent of the system locale. It is written by Ulf Adams and licensed under
the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


MoodyCamel Lock-Free Unbounded Concurrent Queue
-----------------------------------------------

The unbounded concurrent queue code directly forwards to an implementation
by Cameron Desrochers, included in a subdirectory. It is license under
the Boost Software License (https://www.boost.org/users/license.html)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.
