#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#if !defined(NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H)
#error This header must be included via LazyServiceInjector.h
#endif

#include <cstddef>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Vardiadic template whose arguments are a generated integer sequence</summary>
    /// <typeparam name="Integers">Sequential integers as variadic argument list</typeparam>
    template<std::size_t... Integers>
    class IntegerSequence {

      /// <summary>The type of the integer sequence</summary>
      public: typedef IntegerSequence Type;

      /// <summary>Helper that prepends another integer to the sequence</summary>
      public: template<std::size_t PrependedInteger>
      using Prepend = IntegerSequence<PrependedInteger, Integers...>;

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>
    ///   Recursively constructs a variadic template whose arguments are an integer sequence
    /// </summary>
    /// <typeparam name="MaximumInteger">Largest integer (inclusive) in the sequence</typeparam>
    /// <typeparam name="TIntegerSequence">
    ///   Integer sequence constructed by previous recursions or empty of this is the first
    /// </typeparam>
    template<std::size_t MaximumInteger, typename TIntegerSequence>
    class IntegerSequenceBuilder {

      /// <summary>The recursively formed integer sequence</summary>
      /// <remarks>
      ///   This recursively defines itself all the way down until the integer is zero,
      ///   building an IntegerSequence along the way (prepending the value so
      ///   that it still produces an ascending sequence).
      /// </remarks>
      public: typedef typename IntegerSequenceBuilder<
        MaximumInteger - 1,
        typename TIntegerSequence::template Prepend<MaximumInteger>
      >::Type Type;

    };

    /// <summary>Specialization for the final recursion that just returns the sequence</summary>
    template<typename TIntegerSequence>
    class IntegerSequenceBuilder<0, TIntegerSequence> {

      /// <summary>The integer sequence the template was instantiated with</summary>
      public: typedef TIntegerSequence Type;

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Builds variadic template whose parameters are a sequence of integers</summary>
    /// <typeparam name="MaximumInteger">
    ///   Integer up to which a sequence will be formed (inclusive)
    /// </typeparam>
    template<std::size_t MaximumInteger>
    using BuildIntegerSequence = typename IntegerSequenceBuilder<
      MaximumInteger, IntegerSequence<>
    >::Type;

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
