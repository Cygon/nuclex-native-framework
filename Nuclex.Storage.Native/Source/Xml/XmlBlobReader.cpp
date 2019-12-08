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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_STORAGE_SOURCE 1

#include "Nuclex/Storage/Xml/XmlBlobReader.h"
#include "Nuclex/Storage/Blob.h"

#include "XmlBlobReader.Impl.h"

#include "../Helpers/StringHelper.h"
#include "../Helpers/Lexical.h"

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  XmlBlobReader::XmlBlobReader(const std::shared_ptr<const Blob> &blob) :
    blob(blob),
    impl(new Impl(*blob.get())),
    enteredAttribute(nullptr) {}

  // ------------------------------------------------------------------------------------------- //

  XmlBlobReader::~XmlBlobReader() {
    // Must be in the implementation file because the inlined version would not know
    // the destructor of our impl class!    
  }

  // ------------------------------------------------------------------------------------------- //

  XmlReadEvent XmlBlobReader::Read() {
    return this->impl->Read();
  }

  // ------------------------------------------------------------------------------------------- //

  const std::string &XmlBlobReader::GetElementName() const {
    return this->impl->GetElementName();
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t XmlBlobReader::CountAttributes() const {
    return this->impl->CountAttributes();
  }

  // ------------------------------------------------------------------------------------------- //

  const std::string &XmlBlobReader::GetAttributeName(std::size_t index) const {
    return this->impl->GetAttributeName(index);
  }

  // ------------------------------------------------------------------------------------------- //

  bool XmlBlobReader::TryEnterAttribute(const std::string &attributeName) {
    this->enteredAttribute = this->impl->GetAttributeValue(attributeName);
    return (this->enteredAttribute != nullptr);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::LeaveAttribute() {
    if(this->enteredAttribute == nullptr) {
      throw std::logic_error("Tried to leave attribute without having entered one");
    }

    this->enteredAttribute = nullptr;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(bool &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<bool>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<bool>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::uint8_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::uint8_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::uint8_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::int8_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::int8_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::int8_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::uint16_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::uint16_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::uint16_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::int16_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::int16_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::int16_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::uint32_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::uint32_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::uint32_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::int32_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::int32_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::int32_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::uint64_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::uint64_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::uint64_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::int64_t &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<std::int64_t>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<std::int64_t>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(float &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<float>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<float>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(double &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::lexical_cast<double>(this->impl->GetElementText());
    } else {
      target = Helpers::lexical_cast<double>(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobReader::Read(std::string &target) {
    if(this->enteredAttribute == nullptr) {
      target = this->impl->GetElementText();
    } else {
      target = *this->enteredAttribute;
    }
  }

  // ------------------------------------------------------------------------------------------- //
  
  void XmlBlobReader::Read(std::wstring &target) {
    if(this->enteredAttribute == nullptr) {
      target = Helpers::StringHelper::WideCharFromUtf8(this->impl->GetElementText());
    } else {
      target = Helpers::StringHelper::WideCharFromUtf8(*this->enteredAttribute);
    }
  }

  // ------------------------------------------------------------------------------------------- //
  
  void XmlBlobReader::Read(void *buffer, std::size_t byteCount) {
    throw std::runtime_error("Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml
