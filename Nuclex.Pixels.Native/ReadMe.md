Nuclex.Pixels.Native
====================

This library contains a clean implementation of all the graphics plumbing
code you need in a video game or graphical application. It covers a lean
bitmap class that can work on foreign memory, pixel format conversion,
image file loading/saving and image scaling.

As usual, there are unit tests for the whole library, so everything is
verifiably working on all platforms tested (Linux, Windows, Raspberry).

**Bitmaps**
* Clean cross-platform definition of all pixel formats
* Bitmap instances can use own or access foreign memory
* Copy-on-write with sub-bitmaps (called views)
* Autonomize bitmaps to clone foreign memory or force copy

**Storage**
* BitmapSerializer to read and write image file formats
* All I/O happens through 3-method VirtualFile interface
* Supports .png, .tif, .tga, .jpg, .webp and .exr out-of-the-box
* Extensible (i.e. proprietary formats or image library integration)

**Scaling**
* Simple scaling (nearest neighbour and linear interpolation)
* Advanced scaling (lanczos and Aleksey Vaneev's AVIR)

**Everything:**
* Supports Windows, Linux and ARM Linux (Raspberry PI)
* Compiles cleanly at maximum warning levels with MSVC, GCC and clang
* If it's there, it's unit-tested


Bitmap class
------------

The Bitmap class wraps a simple pixel-based image. The pixels can either be
allocated by the Bitmap class upon constructor or it can access an existing
memory buffer.

```cpp
// Construct a new bitmap using its own memory
Bitmap blankImage(1024, 768, PixelFormat::A8_R8_G8_B8_Unsigned);

// Bitmaps are copy-on-write, so the pixels aren't copied here and
// this operation is super cheap (as in, copying 5 ints).
Bitmap cheapCopy = blankImage;

// You can also force an immediate copy by calling Autonomize() on the Bitmap
Bitmap fullCopy = blankImage;
fullCopy.Autonomize();
```

Because the Bitmap class is such a minimal container, you can cheaply
construct one accessing mapped video memory and then directly load an image
file into it without a stopover in system memory. Or in the other direction,
directly save or rescale a screenshot taken directly from a memory-mapped
render target texture.

```cpp
void *pixels;
::vkMapMemory(device, stagingBufferMemory, 0, totalImageByteCount, 0, &pixels);

// Construct a bitmap that uses the mapped video memory to store its pixels
Bitmap videoMemoryImage = Bitmap::InExistingMemory(
  BitmapMemory {
    2048, // width
    2048, // height
    2048 * 4, // stride (pitch) in bytes
    PixelFormat::A8_R8_G8_B8_Unsigned,
    pixels
  }
);

// Let the bitmap serializer load an image file directory into video memory
myBitmapSerializer.Reload(videoMemoryImage, u8"StoneWall-Albedo.png");

::vkUnmapMemory(device, stagingBufferMemory);
```

Or, if you wanted to instead save the contents of a video memory buffer,
you could call `videoMemoryImage.Autonomize()` on the bitmap, thus forcing
it to create its own system memory copy of the video memory buffer.
The bitmap will then remain valid after `::vkUnmapMemory()` and you can
rescale and/or save it on your own time.


BitmapSerializer
----------------

The `BitmapSerializer` reads and writes different image file formats.
Depending on the compile-time configuration of the Nuclex.Pixels library,
a newly constructed instance will already be able to read and write `.png`,
`.tif`, `.tga`, `.jpg`, `.webp` and `.exr` files.

```cpp
// You should set up one instance and make it available as a service,
// for example via the dependency injector in Nuclex.Support.
BitmapSerializer myBitmapSerializer;

// The serializer can quickly read just the image's metadata (width, height
// and pixel format) without needing to read the whole pixel buffer.
std::optional<BitmapInfo> bitmapInfos = myBitmapSerializer.TryReadInfo(
  u8"example-image.png"
);

// Otherwise, reading an image file is as easy as this.
// The Bitmap will select the closest matching pixel format to the file.
Bitmap loadedBitmap = myBitmapSerializer.Load(u8"example-image.png");

// And saving is possible, too, of course
myBitmapSerializer.Save(loadedBitmap, u8"saved-bitmap.jpg");
```

Of course, instead of a file name, all methods accept a `VirtualFile`
instead, doing all I/O through the 3 methods (`GetSize()`, `ReadAt()` and
`WriteAt()`) defined in the `VirtualFile` interface.

If you need to handle a proprietary format or wish to support the dozens of
image file formats some image libraries bring with them, you can implement
one (or multiple) formats as a `BitmapCodec` that is registerable via
the `BitmapSerializer::RegisterCodec()` method:

```cpp
myBitmapSerializer.RegisterCodec(std::make_unique<MyProprietaryCodec>());
myBitmapSerializer.RegisterCodec(std::make_unique<FreeImageAdapterCodec>());
```


Scaling
-------

Using the `BitmapScaler` class, any bitmap can be scaled up and down using
algorithms ranging from simple nearest-neighbor sampling to filter curves
(hermite, mitchell, lanczos). Nuclex.Pixels also embeds the AVIR library
(https://github.com/avaneev/avir) which offers very good results and pretty
good performance, too.

```cpp
Bitmap bigPhoto = myBitmapSerializer.Load(u8"~/Pictures/photo.jpg");

Bitmap smallPhoto = BitmapScaler::Rescale(
  bigPhoto,
  Size(1920, 1080),
  ResamplingMethod::Avir
);

myBitmapSerializer.Save(smallPhoto, u8"~/Pictures/photo-small.jpg");
```

The scaling methods in this library are intended for usage to create mipmaps
and to match imported images to texture resolutions. If your goal is to
process photos or video frames, have a look at RealSR
(https://github.com/Tencent/Real-SR) and its cross-platform Vulkan port
to NCNN (https://github.com/nihui/realsr-ncnn-vulkan), it takes minutes to
process but its results will far exceed the quality of any other methods.


Drawing
-------

For your convenience, Nuclex.Pixels also offers a small amount of basic
drawing methods. These are provided to place markers and frames in images,
but are not a replacement for the capabilities of a proper graphics library
that can draw anti-aliased shapes, vary line width and more.

```cpp
Bitmap canvas(1920, 1080);

SimpleShapeDrawer::Clear(canvas, RgbColor { 1.0f, 1.0f, 1.0f, 1.0f });
SimpleShapeDrawer::FillRectangle(
  canvas,
  RgbColor { 1.0f, 1.0f, 1.0f, 1.0f },
  950, 530, 20, 20
);
```

For advanced drawing needs, have a look at Blend2D (https://blend2d.com/),
which produces very good quality and is probably the fastest vector graphics
rasterizer, too. Also consider Cairo (https://www.cairographics.org/), a
cross-platform graphics library that is very mature and used in many Linux
applications or, if you're already using Qt (https://www.qt.io/), its own
built-in graphics functions.
