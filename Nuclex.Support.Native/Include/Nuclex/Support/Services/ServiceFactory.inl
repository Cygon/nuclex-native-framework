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

#include <memory> // for std::shared_ptr, std::make_shared()

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Constructs unknown types automatically injecting their dependencies</summary>
    /// <typeparam name="TImplementation">Type that will be constructed</typeparam>
    /// <typeparam name="TConstructorSignature">
    ///   Constructor signature obtained from the constructor signature detector
    /// <typeparam>
    template<typename TImplementation, typename TConstructorSignature>
    class ServiceFactory;

    /// <summary>Constructs unknown types automatically injecting their dependencies</summary>
    /// <typeparam name="TImplementation">Type that will be constructed</typeparam>
    /// <remarks>
    ///   Specialization for default-constructible types
    /// </remarks>
    template<typename TImplementation>
    class ServiceFactory<TImplementation, ConstructorSignature<>> {

      /// <summary>Creates a new instance of the service factory's type</summary>
      /// <param name="serviceProvider">Not used in this specialization</param>
      /// <returns>The new instance of the service factory's type</returns>
      public: static std::shared_ptr<TImplementation> CreateInstance(const ServiceProvider &) {
        return std::make_shared<TImplementation>();
      }

    };

    /// <summary>Constructs unknown types automatically injecting their dependencies</summary>
    /// <typeparam name="TImplementation">Type that will be constructed</typeparam>
    /// <typeparam name="TConstructorSignature">
    ///   Constructor signature obtained from the constructor signature detector
    /// <typeparam>
    /// <remarks>
    ///   Variadic version for types with 1 or more constructor arguments
    /// </remarks>
    template<typename TImplementation, typename... TArguments>
    class ServiceFactory<TImplementation, ConstructorSignature<TArguments...>> {

      /// <summary>Creates a new instance of the service factory's type</summary>
      /// <param name="serviceProvider">
      ///   Service provider that will be used to resolve constructor dependencies
      /// </param>
      /// <returns>The new instance of the service factory's type</returns>
      public: static std::shared_ptr<TImplementation> CreateInstance(
        const ServiceProvider &serviceProvider
      ) {
        return std::make_shared<TImplementation>(typename TArguments::Type(serviceProvider)...);
      }

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
