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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Services/ServiceContainer.h"

#include <stdexcept> // for std::logic_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An any instance that does not carry any value</summary>
  static const std::any EmptyAny;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  const std::any &ServiceContainer::Get(const std::type_info &serviceType) const {
    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator == this->services.end()) {
      std::string message;
      message.reserve(17 + 32 + 13);
      message.append(u8"Service of type '", 17);
      message.append(serviceType.name());
      message.append(u8"' not present", 13);
      throw std::logic_error(message);
    }

    return iterator->second;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &ServiceContainer::TryGet(const std::type_info &serviceType) const {
    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator == this->services.end()) {
      return EmptyAny;
    } else {
      return iterator->second;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ServiceContainer::Add(const std::type_info &serviceType, const std::any &service) {
    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator != this->services.end()) {
      std::string message;
      message.reserve(14 + 32 + 15);
      message.append(u8"Service type '", 14);
      message.append(serviceType.name());
      message.append(u8"' already added", 15);
      throw std::logic_error(message);
    }

    this->services.insert(ServiceMap::value_type(&serviceType, service));
  }

  // ------------------------------------------------------------------------------------------- //

  bool ServiceContainer::Remove(const std::type_info &serviceType) {
    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator == this->services.end()) {
      return false;
    } else {
      this->services.erase(iterator);
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
