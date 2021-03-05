#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#include <type_traits>
#include <memory>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Determines whether the specified argument uses std::shared_ptr</summary>
    /// <typeparam name="TChecked">Type that will be checked</typeparam>
    /// <remarks>
    ///   The default case, always 'no'
    /// </remarks>
    template<typename TChecked>
    class IsSharedPtr : public std::false_type {};

    /// <summary>Determines whether the specified argument uses std::shared_ptr</summary>
    /// <typeparam name="TChecked">Type that will be checked</typeparam>
    /// <remarks>
    ///   Specialization for std::shared_ptr types, produces 'yes'
    /// </remarks>
    template <class TChecked>
    class IsSharedPtr<std::shared_ptr<TChecked>> : public std::true_type {};

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Checks whether a constructor argument can potentially be injected</summary>
    /// <typeparam name="TArgument">Constructor argument that will be checked</typeparam>
    /// <remarks>
    ///   Any services provided by the dependency injector are wrapped in std::shared_ptr to
    ///   control the lifetime of the service implementation.
    /// </remarks>
    template<typename TArgument>
    class IsInjectableArgument : public std::integral_constant<
      bool,
      (
        std::is_class<TArgument>::value &&
        !std::is_abstract<TArgument>::value &&
        IsSharedPtr<TArgument>::value
      )
    > {};

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
