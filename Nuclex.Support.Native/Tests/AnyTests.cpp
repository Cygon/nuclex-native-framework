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
#define NUCLEX_GEOMETRY_SOURCE 1

#include "Nuclex/Support/Any.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class that sets an external boolean when an instance is destroyed</summary>
  class DestructionSignaller {

    /// <summary>Initializes a new destruction signalling helper</summary>
    /// <param name="booleanToSet">Boolean that will be set upon destruction</param>
    public: DestructionSignaller(bool *booleanToSet) :
      booleanToSet(booleanToSet) {}

    /// <summary>Sets the boolean to indicate destruction unless disarmed</summary>
    public: ~DestructionSignaller() {
      if(this->booleanToSet != nullptr) {
        *this->booleanToSet = true;
      }
    }

    /// <summary>Disarms the signaller in case the external boolean is no longer valid</summary>
    public: void Disarm() {
      this->booleanToSet = nullptr;
    }

    /// <summary>Pointer to an external boolean that will be set</summary>
    private: bool *booleanToSet;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(AnyTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Any test(12345);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AnyTest, AnyCanBeCopied) {
    Any original(12345);
    Any copy(original);

    EXPECT_EQ(12345, copy.Get<int>());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AnyTest, AccessingWrongTypeThrowsException) {
    Any test(12345);

    EXPECT_THROW(
      test.Get<float>(),
      std::bad_cast
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AnyTest, AnyCanBeAssignedToAnotherAny) {
    bool copiedOverInstanceWasDestroyed = false;
    {
      DestructionSignaller signaller(&copiedOverInstanceWasDestroyed);
      Any test(signaller);
      signaller.Disarm();

      // The Any should have made a single copy of the signaller, so at this point,
      // no instances of the signaller should have been destroyed yet
      EXPECT_EQ(copiedOverInstanceWasDestroyed, false);

      // Overwrite the instance with another 'Any' value, destroying its earlier contents
      Any other(321);
      test = other;

      // Now the signaller within the first any should have been destroyed
      EXPECT_EQ(copiedOverInstanceWasDestroyed, true);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
