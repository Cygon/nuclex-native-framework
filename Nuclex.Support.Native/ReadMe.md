Nuclex.Support.Native
=====================

This library contains general-purpose supporting code with a focus
on small but well-organized pieces you can use throughout other projects
to make you life easier.

There are unit tests for the whole library, so everything is verifiably
working on all platforms tested (Linux, Windows, Raspberry).

* Locale-independent string/number conversion
* Conversion between std::string and std::wstring
* Case-insensitive UTF-8 string comparison
* UTF-8 wildcard matching
* Fast and lightweight events (publisher/subscriber pattern)
* Lean dependency injector with automatic constructor detection

* Supports Windows, Linux and ARM Linux (Raspberry PI)
* Compiles cleanly at maximum warning levels with MSVC, GCC and clang
* Everything is unit-tested


lexical_cast
------------

This is almost identical to the `lexical_cast` function found in Boost,
but it avoids the heavyweight iostreams library. By shipping its own
number/string conversion code, locale issues are avoided and you'll get
the same results independent of the system's locale.

This is very nice if you have to serialize data (i.e. JSON or XML) on
multiple platforms because the data is binary-reproducible.

Uses [Dragon4](http://www.ryanjuckett.com/programming/printing-floating-point-numbers/),
[Erthink](https://abf.io/erthink/erthink) and [Ryu](https://github.com/ulfjack/ryu)
internally, which have licenses that allow this use. See Copyright.md
in the Documents directory for more details.

```cpp
std::string myString = lexical_cast<std::string>(12.34f);
float myFloat = lexical_cast<float>(u8"43.21");
```


StringConverter
---------------

Useful helper methods for string, such as conversion between UTF-8,
UTF-16 and UTF-32 and case-insensitive UTF-8 string comparison (done right
by using the case folding table released by the unicode consortium).

Can also convert between "wide char" strings and UTF-8. Wide chars are
the bad side of unicode that Windows programmers have to deal with, with
many `TEXT()` macros expanding to `L"my string"` which generates UTF-16 on
Windows and UTF-32 on Linux. 

Uses [UTF8-CPP](https://github.com/nemtrif/utfcpp) and some custom code.
See Copyright.md in the Documents directory for more details.

```cpp
// This works on any platform, whether wchar_t is UTF-16 or UTF-32
std::string utf8 = StringConverter::Utf8FromWide(L"Hello World");

// If you need to read or write UTF-16 to communicate with Windows systems
std::u16string alwaysUtf16 = StringConverter::Utf16FromUtf8(u8"Hello World");
```


StringMatcher
-------------

A UTF-8 wildcard matcher for strings.

```cpp
bool retursTrue = StringMatcher::FitsWilcard(
  u8"Cupboard-Albedo.png", u8"*-Albedo.png"
);
```


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


Any
---

Opaquely wraps a type so it can be transmitted between two pieces of code
without making public APIs dependent on a library- or OS-specific type.

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
