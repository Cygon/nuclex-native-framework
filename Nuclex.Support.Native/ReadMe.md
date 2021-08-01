Nuclex.Support.Native
=====================

This library contains general-purpose supporting code with a focus
on small but well-designed pieces you can use throughout other projects
to make you life easier.

There are unit tests for the whole library, so everything is verifiably
working on all platforms tested (Linux, Windows, Raspberry).

**Text**
* Locale-independent string/number conversion
* Conversion between std::string and std::wstring
* Case-insensitive UTF-8 string comparison
* UTF-8 wildcard matching

**Settings**
* Retrieve and store application settings in the registry (Windows-only)
* Retrieve and store application settings in .ini files

**Threading**
* Thread pool for micro tasks with std::future
* Portable, fast Semaphores, Latches and Gates
* Run child processes and intercept stdin/stdout/stderr

**Helpers**
* A modern ScopeGuard plus a transactional variant
* Fast and lightweight signal/slot implementation
* Lean dependency injector with automatic constructor detection
* Scoped temporary file and directory classes

**Everything:**
* Supports Windows, Linux and ARM Linux (Raspberry PI)
* Compiles cleanly at maximum warning levels with MSVC, GCC and clang
* If it's there, it's unit-tested


lexical_cast & lexical_append
-----------------------------

This is almost identical to the `lexical_cast` function found in Boost,
but it avoids the heavyweight iostreams library. By shipping its own
number/string conversion code, locale issues are avoided and you'll get
the same results independent of the system's locale.

This is very nice if you have to serialize data (i.e. JSON or XML) on
multiple platforms because the data is binary-reproducible.

```cpp
std::string myString = lexical_cast<std::string>(12.34f);
float myFloat = lexical_cast<float>(u8"43.21");
```

Or to append text to an `std::string` or buffer without intermediate copies:

```cpp
std::string scoreText = u8"Current score: ";
lexical_append(scoreText, 110025);
```

Uses [Dragon4](http://www.ryanjuckett.com/programming/printing-floating-point-numbers/),
[Erthink](https://abf.io/erthink/erthink) and [Ryu](https://github.com/ulfjack/ryu)
internally, which have licenses that allow this use. See Copyright.md
in the Documents directory for more details.


StringConverter & StringMatcher
-------------------------------

Useful helper methods for strings, such as conversion between UTF-8,
UTF-16 and UTF-32. Can also convert between "wide char" strings and UTF-8.
Wide chars are the bad side of unicode that Windows programmers have to deal
with, often coming from `TEXT()` macros that expand to `L"my string"`,
thus creating UTF-16 on Windows and UTF-32 on Linux.

```cpp
// This works on any platform, whether wchar_t is UTF-16 or UTF-32
std::string utf8 = StringConverter::Utf8FromWide(L"Hello World");

// If you need to read or write UTF-16 to communicate with Windows systems
std::u16string alwaysUtf16 = StringConverter::Utf16FromUtf8(u8"Hello World");
```

Also performs case-insensitive UTF-8 string comparison (done right by using
the case folding table released by the unicode consortium):

```cpp
// Comparison uses current case folding table and should be as safe as ICU.
bool areEqual = StringMatcher::AreEqual(u8"Hello", u8"hello");
```

And UTF-8 wildcard matching as known from various shells:

```cpp
bool returnsTrue = StringMatcher::FitsWildcard(
  u8"Cupboard-Albedo.png", u8"*-Albedo.png"
);
bool alsoReturnsTrue = StringMatcher::FitsWildcard(
  u8"食器棚〜Albedo.png", u8"*〜Albedo.png"
);
```

For containers like `std::map` and `std::unordered_map`, custom functors
compatible to `std::less`, `std::equal_to` and `std::hash` are provided,
allowing you to build associative containers that ignore case:

```cpp
typedef std::unordered_map<
  std::string, int,
  CaseInsensitiveUtf8Hash, CaseInsensitiveUtf8EqualTo
> StringIntegerMap;

StringIntegerMap ingredients;
ingredients[u8"Rødløg"] = 2;
int onionCount = ingredients.at(u8"RØDLØG"); // Different case, still a match
```


Application Settings Storage
----------------------------

Most non-trivial applications need to store their settings somewhere.
With the `SettingsStore` interface, you can transparently access settings
that are either stored in memory (`MemorySettingsStore`), in an .ini file
(`IniSettingsStore`) or in the registry (`RegistrySettingsStore`) on
Windows systems.

Using the `IniSettingsStore` is the most portable solution and care has been
taken to implement an .ini parser that not only preserves formatting and
comments in the .ini file, but also adheres to the existing file's style when
new properties are added to it.

```cpp
IniSettingsStore settings(u8"awesome-game.ini");
// on Windows, try this: RegistrySettingsStore settings(u8"HKCU/My/Game");
// or as mock for unit tests: MemorySettingsStore settings;

// Properties return an std::optional, so you can detect with .has_value()
// if the property is missing and gracefully fall back to a default value.
std::optional<std::uint32_t> resolutionX = (
  settings.Retrieve<std::uint32_t>(u8"Video", u8"ResolutionX")
);

// ...or provide a default value right away via .value_or():
std::uint32_t resolutionY = (
  settings.Retrieve<std::uint32_t>(u8"Video", u8"ResolutionX").value_or(1080)
);

// There's a shared base class for all implementations that you pass to
// any methods dealing with settings:
SettingsStore &abstractSettings = settings;

// Storing properties is just as simple and everything, including
// templated methods, is available through the shared base class:
abstractSettings.Store<bool>(std::string(), u8"FirstLaunch", false);
```


ScopeGuard
----------

This is a simple helper that lets you run cleanup code when leaving a scope such
as a method, loop or an explicit scope. It is executed even when the scope exit
is due to an exception.

Using scope guards not only avoids error-prone manual cleanup before throwing
an exception, but also avoids the try..catch..rethrow pattern that obscures
the original origin of exceptions.

```cpp
void test(::image_t *image) {
  {
    ::pixel_counter_t *counter = ::awesomelib_create_pixel_counter();
    assert((counter != nullptr));

    ON_SCOPE_EXIT { ::awesomelib_destroy_pixel_counter(counter); };

    int pixelCount = ::awesomelib_count_pixels(counter, image);
    if(pixelCount < 0) {
      throw std::runtime_error(u8"Oh no! Pixel counting failed!");
    }

    reticulatePixels(pixelCount);
  }
}
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
  public: int Add(int first, int second) override {
    return first + second + 1;
  }
  public: int Multiply(int first, int second) override {
    return first + first * second;
  };
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


Thread Pool
-----------

Thread pools keep a bunch of threads ready to go. As soon as you schedule tasks
to a thread pool, they get picked up by the next available worker thread.

This avoids the overhead of creating and destroying threads, allowing you to run
even small tasks in a thread.

It also helps scalability. As long as you can break your tasks down into small
individual steps, they can be distributed to any number of threads. Systems with
more CPU cores automatically perform more work in parallel.

```cpp
int testMethod(int a, int b) { return a * b - (a + b); }

int main() {
  ThreadPool pool;

  std::future<int> future = testPool.Schedule(&testMethod, 12, 34);
  // Do something else here...
  int result = future.get();
}
```


Gates, Semaphores and Latches
-----------------------------

Gates can be either opened or closed. Any thread calling Wait() will block on
the gate until it is opened by another thread. This is useful to black access
to a system before it is ready to work or to stop a single thread on shutdown
until all worker threads have vacated a system.

Semaphores are a well-known threading primitive working like a counted mutex:
one thread can pass for each call to Post() that was made in the past or while
a thread was waiting.

They're useful for work queues and advanced locking on resources that can only
handle limited parallelism (i.e. due to memory or hardware constraints).

Finally, Latches are like inverted Semaphores. Threads can pass through when
the latches' count reaches zero. This is useful if you want to delay some
shutdown or disconnect code until the last thread stops using a resource.

Note that C++20 is also getting a semaphore class, but until then, this one
provides you with a portable implementation that also avoids limitations on
Posix systems where the default semaphore uses timeouts based on wall
clock time.

```cpp
int main() {
  Semaphore sem(0);

  // Let one current of future thread through
  sem.Post();

  // Wait until the semaphore is posed (incremented)
  sem.WaitAndDecrement();
}
```


Child Processes
---------------

This class makes it easy to spawn child process and to capture the output they
send to stdout and stderr.

Creating child processes correctly is a rather complicated task that differs
a lot between Windows and Linux. This wrapper provides a sane, portable way to
launch, observe, wait on or kill child processes. It can be used for launchers,
auto-updaters or if you want to run a command-line application such as `ffmpeg`
or your C++ compiler:

```cpp
void handleFfmpegStdOut(const char *characters, std::size_t characterCount) {
  // Parse progress, log or just print it
}

int main() {
  using Nuclex::Support::Threading::Process;

  // Finds ffmpeg in 1) same directory as executable or 2) PATH environment
  Process encoder(u8"ffmpeg");
  {
    encoder.SetWorkingDirectory(u8"~/video-encodes");
    encoder.StdOut.Subscribe<&handleFfmpegStdOut>();

    encoder.Start(
      { u8"-i input.avi", u8"-vcodec v210", u8"-an", u8"-y", u8"output.avi" }
    );
    bool hasExited = encoder.Wait(std::chrono::milliseconds(60000));

    // Use encoder.Write() to send something to the process' stdin
    // Use encoder.Kill() to ask for termination and/or murder the process
  }
  int exitCode = encoder.Join();

  return exitCode;
}
```


Variant
-------

A variant is a variable that can store any type of value. It's essentially
what variables in dynamically typed languages are made of.

This implementation differs from `std::variant` of C++ 17 (which lets you
choose which types it can store). The `Nuclex::Support::Variant` can store
all primitive C++ types, strings and objects (within `std::any`) and provides
reasonable conversions between all of these.


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
