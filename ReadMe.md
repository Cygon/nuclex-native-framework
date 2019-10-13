Nuclex Native Framework
=======================

The Nuclex Native Framework is a set of C++ libraries carefully developed
to provide sane, reliable and efficient building blocks to use in your own
projects.

I call it a framework because its individual parts can provide you a very
neat and tidy environment around your code. Nevertheless, this is not
the kind of framework you build your application into, it's a set of
beautifully designed and robust components that you can use and will and
that make no assumptions about your code.


Components
==========

Build System
------------

I build and test my libraries on multiple platforms, including Linux,
Windows and Raspberry PIs. So I need a build system where I can simply
say "this directory is a project set up with my standard layout. The library
is called X. Go build it!"


Nuclex.Support.Native
---------------------

This library contains general-purpose code that I use in most of my projects
to prevent the insanity of multiple platforms with different text encodings,
threading facilities and compiler capabilities from leaking in


Others
------

I'll provide controlled releases of my other libraries over time. Currently
in my Subversion repository are `Nuclex.Game.Native`  (game-specific stuff
such as reliable cross-platform precision timing), `Nuclex.Pixels.Native`
(sane Bitmap class with copy-on-write and friendly use of borrowed memory,
pixels format conversions, resizing and such), `Nuclex.Geometry.Native`
(sweet 2D and 3D geometry classes without any rough edges) and a few others.
