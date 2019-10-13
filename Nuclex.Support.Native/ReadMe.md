Nuclex.Support.Native
=====================

This library contains general-purpose supporting code with a focus
on small but well-organized pieces you can use throughout other projects
to make you life easier.

There are unit tests for the whole library, so everything is verifiably
working on all platforms tested (Linux, Windows, Raspberry).


lexical_cast
------------

This is almost identical to the `lexical_cast` function found in Boost,
but it avoids iostreams (which would be a relatively heavy dependency).

It also ships its own number/string conversion code, therefore the output
will always look identical, no matter what platform, standard library or
system locale is being used.

This is very nice if you serialize data on multiple platforms because
the data is binary-reproducible.

Uses [Dragon4](http://www.ryanjuckett.com/programming/printing-floating-point-numbers/),
[Erthink](https://abf.io/erthink/erthink) internally, which have licenses that
allow this use. See Copyright.md in the Documents directory.


StringConverter
---------------

Wraps [UTF8-CPP](https://github.com/nemtrif/utfcpp) and limits its interface
to simple conversions between `std::string` (assumed to be holding UTF-8)
and `std::wstring` (assumed to be holding UTF-16 on Windows systems where
`wchar_t` is 16 bits wide and the compiler turns `L"something"` into UTF-16)
(assumed to be holding UTF-32 on Unix systems where `wchar_t` is 32 bits wide
and the compiler turns `L"something"` into UTF-32).

My convention is to only use UTF-8. The StringConverter's only purpose is to
allow interfacing with Microsoft's unicode API (which requires UTF-16) and to
disarm `TEXT()` macros used by cargo-culting Windows programmers who picked up
the idea that using wide chars is necessary, or useful, for localization ;-)


Events (Signal/Slot system)
---------------------------

A robust, lean and fast signal/slot system. It's extremely fast (no library
I tested it against could keep up) and yet offers a decent set of functionality
including:

  * Collecting return values from callbacks
  * Doing so into an existing container without allocating memory
  * Callbacks unsubscribing themselves inside the notification call
  * Callbacks subscribing additional callbacks inside the notification call
  * Unsubscribing with just the method pointer (no connection object to keep)

The basic syntax is this:

```cpp
void test() {
  std::cout << "You called?" << std::endl;
}
int main() {
  Nuclex::Support::Events::Event<void()> event;
  event.Subscribe<test>();
  event();
}
```

You can find some benchmarks on my blog:
[Nuclex Signal/Slot Library: Benchmarks](http://blog.nuclex-games.com/2019/10/nuclex-signal-slot-benchmarks)


Any
---

Opaquely wraps a type so it can be transmitted between two pieces of code
without making public APIs dependent on a library- os OS-specific type.

Imagine this:

```cpp
class AwesomeVulkanRenderer {

  public: SetRenderWindow(const Nuclex::Support::Any &windowHandle);

};
```

The renderer's public header does not need to include `X11/Xlib.h` or
`Windows.h`, the `Any` can be passed to it (and to some internal factory
class or whatever, too). Only code that has the key (included `X11/Xlib.h`
or `Windows.h`), namely the code responsible for setting up an OS-level
window, would be able to fetch the correct value.

If you simply used a `void *` or `uintptr_t`, you could pass random nonsense
to the renderer.

This will be replaced with `std::any` in C++ 17.


Variant
-------

A variant is a variable that can store any type of value. It's essentially
what variables in dynamically typed languages are made of.

This implementation differs from `std::variant` of C++ 17 (which lets you
choose which types it can store). The `Nuclex::Support::Variant` can store
all primitive C++ types, strings and objects (within `Nuclex::Support::Any`)
and provides reasonable conversion between all of these.
