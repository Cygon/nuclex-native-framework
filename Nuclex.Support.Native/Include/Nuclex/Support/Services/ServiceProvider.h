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

#ifndef NUCLEX_SUPPORT_SERVICES_SERVICEPROVIDER_H
#define NUCLEX_SUPPORT_SERVICES_SERVICEPROVIDER_H

#include "Nuclex/Support/Config.h"

#include <memory> // for std::shared_ptr
#include <type_traits> // for std::decay, std::type_info
#include <any> // for std::any

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides services to the application</summary>
  /// <remarks>
  ///   This is an interface through which services can be looked up. It is either used
  ///   manually (but beware of the service locator anti-pattern!) or as part of
  ///   a dependency injection framework.
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ServiceProvider {

    /// <summary>Destroys the service provider and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~ServiceProvider() = default;

    /// <summary>Looks up the specified service</summary>
    /// <typeparam name="TService">Type of service that will be looked up</typeparam>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="std::any" />
    /// </returns>
    public: template<typename TService> const std::shared_ptr<TService> &Get() const {
      typedef std::shared_ptr<TService> SharedServicePointer;
      return std::any_cast<const SharedServicePointer &>(
        Get(typeid(typename std::decay<TService>::type))
      );
    }

    /// <summary>Tries to look up the specified service</summary>
    /// <typeparam name="TService">Type of service that will be looked up</typeparam>
    /// <param name="service">Shared pointer that will receive the service if found</param>
    /// <returns>True if the specified service was found and retrieved</returns>
    public: template<typename TService> bool TryGet(std::shared_ptr<TService> &service) const {
      typedef typename std::decay<TService>::type VanillaServiceType;
      typedef std::shared_ptr<VanillaServiceType> SharedServicePointer;

      std::any serviceAsAny = TryGet(typeid(VanillaServiceType));
      if(serviceAsAny.has_value()) {
        service = std::any_cast<SharedServicePointer>(serviceAsAny);
        return true;
      } else {
        service.reset();
        return false;
      }
    }

    /// <summary>Looks up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
    /// </returns>
    protected: NUCLEX_SUPPORT_API virtual const std::any &Get(
      const std::type_info &serviceType
    ) const = 0;

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>An Any containing the service, if found, or an empty Any</returns>
    /// <remarks>
    ///   <para>
    ///     An empty <see cref="Any" /> will be returned if the specified service has not
    ///     been activated yet (for a mere container, that means it's not in the container,
    ///     for a factory, it means it has not been constructed yet or its lifetime requires
    ///     that the service instance is not stored).
    ///   </para>
    ///   <para>
    ///     If there is another problem, this method will still throw an exception.
    ///   </para>
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual const std::any &TryGet(
      const std::type_info &serviceType
    ) const = 0;

    //private: ServiceProvider(const ServiceProvider &);
    //private: ServiceProvider &operator =(const ServiceProvider &);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_SERVICEPROVIDER_H
