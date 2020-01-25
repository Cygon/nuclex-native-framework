#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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
#include <type_traits>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Informations about an argument passed to the constructor of a type</summary>
    /// <typeparam name="ArgumentIndex">Index of this argument</typeparam>
    template<std::size_t ArgumentIndex>
    class ConstructorArgument {

      /// <summary>This type</summary>
      public: typedef ConstructorArgument Type;

      /// <summary>Index of this argument on the constructor</summary>
      public: constexpr static std::size_t Index = ArgumentIndex;

      /// <summary>Initializes a new constructor argument<summary>
      /// <param name="serviceActivator">
      ///   Activator through which the argument will be resolved when it is used
      /// </param>
      public: ConstructorArgument(const ServiceProvider &serviceActivator) :
        serviceActivator(serviceActivator) {}

      /// <summary>Implicitly converts the placeholder to the argument's type</summary>
      /// <typeparam name="TArgument">The full type of the argument</typeparam>
      /// <returns>The value that will be passed as the constructor argument</returns>
      public: template<
        typename TArgument,
        typename = typename std::enable_if<
          IsInjectableArgument<typename std::decay<TArgument>::type>::value
        >::type
      >
      operator TArgument() const {
        typedef typename TArgument::element_type ServiceType;
        return this->serviceActivator.Get<ServiceType>();
      }

      /// <summary>Activator through which the argument will be resolved when needed</summary>
      private: const ServiceProvider &serviceActivator;

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Stores a constructor signature (the number and type of its arguments)</summary>
    /// <typeparam name="TArguments">Arguments required by the constructor</typeparam>
    template<typename... TArguments>
    class ConstructorSignature {

      /// <summary>The type of this constructor signature itself</summary>
      public: typedef ConstructorSignature Type;

      /// <summary>Number of arguments being passed to the constructor</summary>
      public: static constexpr std::size_t ArgumentCount = sizeof...(TArguments);

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Used if the constructor signature cannot be determined</summary>
    class InvalidConstructorSignature {
    
      /// <summary>The type of this constructor signature itself</summary>
      public: typedef InvalidConstructorSignature Type;

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
