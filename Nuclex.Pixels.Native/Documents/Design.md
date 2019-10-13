Nuclex.Pixels
=============

Nuclex.Pixels loads and saves bitmaps in various file formats. It also provides
a very slim set of functions to do basic operations such as converting between
different pixel formats, efficiently reading and writing pixels and rescaling
images.

It is not intended to be a full image processing library and will never become
one. The intended uses for this library are to either serve in applications
that need to load user-provided images (like video games and web browsers) or
that implement their own image processing on top of this library.

It has been created with the motivation to avoid the problems plaguing the many
other libraries

Features
--------

* General
   * Stay clear of anything but ISO C++ in public headers
   * Cross-platform (at least Windows, Linux, Android)
   * Support a wealth of pixel formats, not just bits per pixel
     including 64 bit RGB, 16 bit gray, float and half-float pixel formats
   * Sub-images (use area within another image as its own image)
   * Copy-on-write for images
   * Thread-safety
   * Do not use CMake

* Codecs
   * Load and save images via pluggable image codecs
   * Recognize image format from file header
   * Load/write with well-designed streams (i.e. ReadAt, WriteAt)
   * Load only image informations (pixel format, resolution)

* Access
   * Pixel iterators that do stride/offset calculations efficiently
   * Pixel accessor that allows any bit depth to be accessed

* Processing
   * Conversion between pixel formats
   * Flipping and mirroring
   * High quality resizing
     including warp resizing for tiling textures

* Drawing
   * Very basic drawing routines to mark spots in images for debugging
     Pixel, Line, Rectangle, FilledRectangle, Ellipse, FilledEllipse, done.
