Nuclex.Pixels.Native License
============================

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
  as long as you also keep the compiled binaries to yourself.

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

The Nuclex.Pixels.Native library is used for image file loading/saving,
pixel format conversion and resizing. It is written by Markus Ewald and
licensed under the terms of the IBM Common Public License
(https://opensource.org/licenses/cpl1.0.php)

This library also embeds additional libraries, belonging to their respective
owners and used according to their respective licenses:

  * The AVIR image resampling library under the MIT license
//  * LibTARGA for .tga loading/saving under the MIT license
  * LibTIFF for .tif loading/saving under the LibTIFF license
  * LibPNG for .png loading/saving under the LibPNG license
  * LlibJPEG for .jpg loading/saving under the LibJPEG license
  * OpenEXR for .exr loading/saving under a modified BSD license
  * ZLib internally by LibTIFF and LibPNG under the ZLib license


Third-Party Code
================

Nuclex.Pixels.Native also embeds carefully selected third-party code which
falls under its own licenses, listed below


AVIR high quality image resampling library
------------------------------------------

The AVIR library is used to do image resampling with the Lanczos filter and
with its own resizing method. It is written by Aleksey Vaneev and licensed
under the MIT license (https://en.wikipedia.org/wiki/MIT_License)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


LibTARGA: MIT License
---------------------

Loading and saving of image files in the .tga format is done via a modified
version of LibTARGA. It was originally written my Michael L. Gleicher and
offered under the MIT License (https://en.wikipedia.org/wiki/MIT_License)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


LibTIFF: LibTIFF License (MIT/BSD-like)
---------------------------------------

Loading and saving of image files in the .tif format is done via LibTIFF. It
was released by Silicon Graphics, Inc. and continued by Sam Leffler under the
LibTIFF license (https://gitlab.com/libtiff/libtiff/-/blob/master/COPYRIGHT)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


LibPNG: LibPNG License
----------------------

Loading and saving of image files in the .png format is done via LibPNG.
It is copyrighted to the PNG Reference Library Authors and licensed under
the LibPNG license (http://www.libpng.org/pub/png/src/libpng-LICENSE.txt)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


LibJPEG: LibJPEG License
------------------------

Loading and saving of image files in the .jpg format is done via LibJPEG.
It is copyrighted to the Independent JPEG Group and licensed under the
LibJPEG license (https://jpegclub.org/reference/libjpeg-license/)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


OpenEXR: Modified BSD License
-----------------------------

Loading and saving of image files in the .exr format is done via OpenEXR.
It is copyrighted to the OpenEXR Contributors and licensed under the a
modified BSD license (https://www.openexr.com/license.html)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


ZLib: ZLib License
------------------

Some compression methods inside LibPNG and LibTIFF rely on ZLib. ZLib is
copyrighted to Jean-loup Gailly and Mark Adler and licensed under the
ZLib license (https://www.zlib.net/zlib_license.html)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.
