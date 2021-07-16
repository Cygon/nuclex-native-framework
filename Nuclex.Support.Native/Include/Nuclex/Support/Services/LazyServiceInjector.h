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

#ifndef NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H
#define NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/ServiceContainer.h"

#include <map> // for std::map (storing services by std::type_info)

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The maximum number of constructor arguments that can be injected</summary>
  /// <remarks>
  ///   Increasing this value will result in (slightly) slower compiles. Though you might
  ///   want to reconsider your design if a single type consumes more than 8 services ;)
  /// </remarks>
  static constexpr std::size_t MaximumConstructorArgumentCount = 8;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#include "Nuclex/Support/Services/IntegerSequence.inl"
#include "Nuclex/Support/Services/Checks.inl"
#include "Nuclex/Support/Services/ConstructorSignature.inl"
#include "Nuclex/Support/Services/ConstructorSignatureDetector.inl"
#include "Nuclex/Support/Services/ServiceFactory.inl"

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Binds services and initializes them via constructor injection</summary>
  /// <remarks>
  ///   This is a very simplified dependency injector that only supports global services
  ///   stored in shared_ptrs.
  /// </remarks>
  class LazyServiceInjector : public ServiceProvider {

    #pragma region class TypeInfoComparer

    /// <summary>Compares instances of std::type_info</summary>
    private: class TypeInfoComparer {

      /// <summary>Determines the relationship of two std::type_info instances</summary>
      /// <param name="left">Type info to compare on the left side</param>
      /// <param name="right">Type info to compare on the right side</param>
      /// <returns>True if the left side comes before the right side</returns>
      public: bool operator()(const std::type_info *left, const std::type_info *right) const {
        return left->before(*right) != 0;
      }

    };

    #pragma endregion // class TypeInfoComparer

    #pragma region class BindSyntax

    /// <summary>Provides the syntax for the fluent Bind() method</summary>
    /// <typeparam name="TService">Service for which a binding will be set up</typeparam>
    public: template<typename TService> class BindSyntax {
      friend LazyServiceInjector;

      /// <summary>Initializes the syntax helper for binding services</summary>
      /// <param name="serviceInjector">Service injector on which services will be bound</param>
      protected: BindSyntax(LazyServiceInjector &serviceInjector) :
        serviceInjector(serviceInjector) {}

      /// <summary>Binds the service to a constructor-injected provider</summary>
      /// <typeparam name="TImplementation">Implementation of the service to use</typeparam>
      /// <remarks>
      ///   This binds the service to the specified service implementation
      /// </remarks>
      public: template<typename TImplementation> void To() {
        typedef Private::DetectConstructorSignature<TImplementation> ConstructorSignature;

        // Verify that the implementation actually implements the service
        static_assert(
          std::is_base_of<TService, TImplementation>::value,
          "Implementation must inherit from the service interface"
        );

        // Also verify that the implementation's constructor can be injected
        constexpr bool implementationHasInjectableConstructor = !std::is_base_of<
          Private::InvalidConstructorSignature, ConstructorSignature
        >::value;
        static_assert(
          implementationHasInjectableConstructor,
          "Implementation must have a constructor that can be dependency-injected "
          "(either providing a default constructor or using only std::shared_ptr arguments)"
        );

        // Implementation looks injectable, add the service factory method to the map
        const std::type_info &serviceTypeInfo = typeid(TService);
        this->serviceInjector.factories.insert(
          ServiceFactoryMap::value_type(
            &serviceTypeInfo,
            [](const ServiceProvider &serviceProvider) {
              typedef Private::ServiceFactory<TImplementation, ConstructorSignature> Factory;
              return Any(
                std::static_pointer_cast<TService>(Factory::CreateInstance(serviceProvider))
              );
            }
          )
        );
      }

      /// <summary>Binds the service to a factory method or functor used to create it</summary>
      /// <typeparam name="TResult">Type of value returned by the service factory</typeparam>
      /// <typeparam name="TMethod">Service factory method used to create the service</typeparam>
      public: template<
        typename TResult, std::shared_ptr<TResult>(*TMethod)(const ServiceProvider &)
      >
      void ToFactoryMethod() { 

        // Verify that whatever the factory method returns implements the service
        static_assert(
          std::is_base_of<TService, TResult>::value,
          "Factory method must return either the service type or one that "
          "inherits from it"
        );

        // Method does provide the service, add it to the map
        const std::type_info &serviceTypeInfo = typeid(TService);
        this->serviceInjector.factories.insert(
          ServiceFactoryMap::value_type(
            &serviceTypeInfo,
            [](const ServiceProvider &serviceProvider) {
              return Any(std::static_pointer_cast<TService>(TMethod(serviceProvider)));
            }
          )
        );

      }

      /// <summary>Binds the service to a factory method or functor used to create it</summary>
      /// <typeparam name="TMethod">Service factory method used to create the service</typeparam>
      public: template<
        std::shared_ptr<TService>(*TMethod)(const ServiceProvider &)
      >
      void ToFactoryMethod() { 

        // Method does provide the service, add it to the map
        const std::type_info &serviceTypeInfo = typeid(TService);
        this->serviceInjector.factories.insert(
          ServiceFactoryMap::value_type(
            &serviceTypeInfo,
            [](const ServiceProvider &serviceProvider) {
              return Any(TMethod(serviceProvider));
            }
          )
        );

      }

      /// <summary>Binds the service to an already constructed service instance</summary>
      /// <param name="instance">Instance that will be returned for the service</param>
      public: void ToInstance(const std::shared_ptr<TService> &instance) {
        const std::type_info &serviceTypeInfo = typeid(TService);
        this->serviceInjector.instances.insert(
          ServiceInstanceMap::value_type(&serviceTypeInfo, Any(instance))
        );
      }

      /// <summary>Assumes that the service and its implementation are the same type</summary>
      /// <remarks>
      ///   For trivial services that don't have an interface separate from their implementation
      ///   class (or when you just have to provide some implementation everywhere),
      ///   use this method to say that the service type is a non-abstract class and
      ///   should be created directly.
      /// </remarks>
      public: void ToSelf() {
        typedef Private::DetectConstructorSignature<TService> ConstructorSignature;

        constexpr bool serviceHasInjectableConstructor = !std::is_base_of<
          Private::InvalidConstructorSignature, ConstructorSignature
        >::value;
        static_assert(
          serviceHasInjectableConstructor,
          "Self-bound service must not be abstract and requires a constructor "
          "that can be dependency-injected (either providing a default constructor or "
          "using only std::shared_ptr arguments)"
        );

        // Service looks injectable, add the service factory method to the map
        const std::type_info &serviceTypeInfo = typeid(TService);
        this->serviceInjector.factories.insert(
          ServiceFactoryMap::value_type(
            &serviceTypeInfo,
            [](const ServiceProvider &serviceProvider) {
              typedef Private::ServiceFactory<TService, ConstructorSignature> Factory;
              return Any(Factory::CreateInstance(serviceProvider));
            }
          )
        );

      }

      /// <summary>Service injector to which the binding will be added</summary>
      private: LazyServiceInjector &serviceInjector;

    };

    #pragma endregion // class BindSyntax

    /// <summary>Initializes a new service injector</summary>
    public: NUCLEX_SUPPORT_API LazyServiceInjector() = default;

    /// <summary>Destroys the service injector and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~LazyServiceInjector() = default;

    /// <summary>Binds a provider to the specified service</summary>
    /// <returns>A syntax through which the provider to be bound can be selected</returns>
    public: template<typename TService>
    BindSyntax<TService> Bind() {
      return BindSyntax<TService>(*this);
    }

    // Unhide the templated Get method from the service provider
    using ServiceProvider::Get;

    // Unhide the templated TryGet method fro mthe service provider
    using ServiceProvider::TryGet;

    /// <summary>Creates a new instance of the specified service</summary>
    /// <typeparam name="TService">Type of service that will be created</typeparam>
    /// <returns>A new instance of the requested service</returns>
    public: template<typename TService>
    std::shared_ptr<TService> Create() const {
      const std::type_info &serviceTypeInfo = typeid(TService);
      std::shared_ptr<TService> newServiceInstance(
        Create(serviceTypeInfo).Get<std::shared_ptr<TService>>()
      );
      return newServiceInstance;
    }

    /// <summary>Looks up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
    /// </returns>
    protected: NUCLEX_SUPPORT_API const Any &Get(
      const std::type_info &serviceType
    ) const override;

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>An Any containing the service, if found, or an empty Any</returns>
    protected: NUCLEX_SUPPORT_API const Any &TryGet(
      const std::type_info &serviceType
    ) const override;

    /// <summary>Creates the specified service</summary>
    /// <param name="serviceType">Type of service that will be created</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
    /// </returns>
    protected: NUCLEX_SUPPORT_API Any Create(
      const std::type_info &serviceType
    ) const;

    /// <summary>Delegate for a factory method that creates a service</summary>
    private: typedef Any(*CreateServiceFunction)(const ServiceProvider &);

    /// <summary>Map of factories to create different services</summary> 
    private: typedef std::map<
      const std::type_info *, CreateServiceFunction, TypeInfoComparer
    > ServiceFactoryMap;

    /// <summary>Map of services permanently stored in the container</summary>
    private: typedef std::map<const std::type_info *, Any> ServiceInstanceMap;

    // These are both mutable. Reasoning: the service injector acts as if all services
    // already existed, so while services may get constructed as a result of requesting
    // them, to the caller there's no different between an already provided service
    // and one that is constructed during the Get() call.

    /// <summary>Factory methods to construct the various services</summary>
    private: mutable ServiceFactoryMap factories;
    /// <summary>Stores services that have already been initialized</summary>
    private: mutable ServiceInstanceMap instances;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H
