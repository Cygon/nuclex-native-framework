Nuclex.Pixels.Native
====================

This is a convenient library for raster image processing. Its centerpiece
is the `Bitmap` class which either owns the memory storing the pixels or
merely wraps it for you. In either case, a `Bitmap` in `Nuclex::Pixels`
is a very lean object weighing 2 pointers and 4 integers.

For that, you get easy, standardized bitmap storage, copy-on-write,
optimized support for a wealth of pixel formats and pixel manipulation.


`Bitmap` Class
--------------

This class tries to give you a tidy standard interface to pixel-based
graphics. It's usage is simple:

```cpp
int main() {
  using Nuclex::Pixels::Bitmap;

  Bitmap myBitmap(640, 480);
  Bitmap copyOnWriteClone = myBitmap;

  copyOnWriteClone.Autonomize();
}
```

or

```cpp
// Our imaginary frame grabber gives us unwieldy data, wrap it up!
Bitmap makeItSane(
  void *pixelBuffer, std::size_t lineBytes, std::size_t lineCount
) {
  std::size_t actualWidth = lineBytes / 8 * 8;

  BitmapMemory memory;
  memory.PixelFormat = PixelFormat::R8_G8_B8_A8_Unsigned;
  memory.Width = CountBitsPerPixel(memory.PixelFormat) * lineBytes / 8;
  memory.Height = lineCount;
  memory.Stride = lineBytes;
  memory.Pixels = pixelBuffer;

  return Bitmap::FromExistingMemory(memory);
}
```

You can, at any time, access a bitmap's memory block with its `Access()`
method, obtaining the above data structure. Calling `Autonomize()` on
bitmaps using borrowed memory (like in the above example) will also make
the bitmap allocate its own memory and copy the foreign buffer.


`PixelIterator` class
---------------------

It's not as speedy as running through the raw data on your own, but the pixel
iterator gives you a C++-like iterator interface to navigate pixels in
a 2D bitmap with performance as good as such a wrapper can possibly allow:

```cpp
void fumblicateBitmap(const Bitmap &bitmap) {
  PixelIterator it(bitmap.Access());

  it.MoveTo(256, 256); // Jump to this X, Y position

  PixelIterator end = PixelIterator::GetEnd(myBitmapMemory);
  for(; it != end; ++it) {

    // *it is the current pixel address
    if(isFumblicatable(*it)) {
      it += Lines(1); // Pixel fumblicated, go down by 1
    }

  }
}
```
