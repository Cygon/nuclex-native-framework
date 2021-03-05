Nuclex.Storage.Native
=====================

This library has everything you need to access data from the file system,
including from archive files (like `.zip`, `.rar` or `.7z`), directly
streaming compressed data, serializing arbitraty data structures to XML files
and reading them back, patching, hashing and more.

As always, there are unit tests for the whole library, so everything is
verifiably working on all platforms tested (Linux, Windows, Raspberry).

* Compression algorithm manager to select appropriate algorithms
* Common and exotic algorithms wrapped behind a clean streaming interface
  * Deflate (via ZLib, known from `.zip`)
  * LZMA (via LZip)
  * LZMA (via LZMA-SDK, known from `.7z`)
  * Brotli (Google's high-end algorithm)
  * UnRar (known from `.rar`)
  * BSC (high-compressing BZip2-like)
  * CSC (high-compressing LZMA-like)
  * ZPaq (ultra-high compressing bytecode-based system)
  * Tangelo (optimized ZPaq, faster but similar ratios)
* Reading and writing binary data with endian-awareness
* File system abstraction with airtight error handling
* Find each platform's intended locations for storing different kinds of data
  * i.e. executables, game assets, saved games, configuration files

* (WIP) Hashing with MD5, SHA-1 SHA-256
* (WIP) Binary diffs and patching

* Supports Windows, Linux and ARM Linux (Raspberry PI)
* Compiles cleanly at maximum warning levels with MSVC, GCC and clang
* Everything is unit-tested


Compression
-----------

```cpp

```

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

I'm naming these *events* rather than *signals* because the term *signal*
is taken by `std::signal` for something entirely different and and least
in Microsoft land, the term *event* is pretty common for this concept.


Dependency Injector
-------------------

A dependency injector that is non-intrusive, easy to use and automatically
detects your constructor parameters. With this, systems no longer need to
rely on anti-patterns like singletons and remain unit-testable.

  * Easy, fluent syntax inspired by Ninject for .NET
  * All standard C++, no macros, no preprocessor, no code generator, no XML
  * Service implementation type is only needed at site of service registration
  * Recursive dependency resolution

Here's an example that shows how it works:

```cpp
class CalculatorService {
  public: virtual int Add(int first, int second) = 0;
  public: virtual int Multiply(int first, int second) = 0;
};

class BrokenCalculator : public virtual CalculatorService {
  public: int Add(int first, int second) override { return first + second + 1; }
  public: int Multiply(int first, int second) override { return first + first * second; };
};

class CalculatorUser {
  public: CalculatorUser(const std::shared_ptr<CalculatorService> &calculator) :
    calculator(calculator) {}

  public: int CalculateSomething() {
    return this->calculator->Add(1, 2) + this->calculator->Multiply(2, 2);
  }

  private: std::shared_ptr<CalculatorService> calculator;
};

int main() {
  Nuclex::Support::Services::LazyServiceInjector serviceInjector;

  // Yep, it detects the constructor arguments, you can add or remove them :)
  // Yep, it's non-intrusive, standard C++ and statically compiled :)
  serviceInjector.Bind<CalculatorService>().To<BrokenCalculator>();
  serviceInjector.Bind<CalculatorUser>().ToSelf();

  std::shared_ptr<CalculatorUser> user = serviceInjector.Get<CalculatorUser>();
  assert(!!user);

  int result = user->CalculateSomething();
}
```

Variant
-------

A variant is a variable that can store any type of value. It's essentially
what variables in dynamically typed languages are made of.

This implementation differs from `std::variant` of C++ 17 (which lets you
choose which types it can store). The `Nuclex::Support::Variant` can store
all primitive C++ types, strings and objects (within `Nuclex::Support::Any`)
and provides reasonable conversion between all of these.


Containers
----------

`Nuclex::Support` offers a few interfaces in case you want to expose lists,
sets and key/value pairs in a public API.

There are also a few specialty collections, such as a `RingBuffer` class optimized
for batch-processing and an alternative streaming buffer under the name
`ShiftBuffer` which offers better performance if your use case typically empties
(or mostly empties) the container when taking data out of it.

Shift buffers keep all data linear (no wrap-around) and wait for an opportunity
to cheaply empty the buffer (i.e. when none or only a few bytes remain in it).
When reading from the buffer, you can obtain a pointer into the buffer's memory,
thus handling the data without an unnecessary `memcpy()` / `std::copy_n()` call.
