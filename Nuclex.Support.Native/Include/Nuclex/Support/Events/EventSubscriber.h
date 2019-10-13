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

#ifndef NUCLEX_SUPPORT_EVENTS_EVENTSUBSCRIBER_H
#define NUCLEX_SUPPORT_EVENTS_EVENTSUBSCRIBER_H

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Base class for event subscribers that enables automatic unsubscribing</summary>
  class EventSubscriber {

    #pragma region class Subscription

    /// <summary>Stores the informations about an event subscription</summary>
    private: class Subscription {

      /// <summary>Terminates the event subscription</summary>
      public: virtual ~Subscription() {}

    };

    #pragma endregion // class Subscription

    #pragma region class Subscription1

    /// <summary>Stores the informations about an event subscription</summary>
    private: template<typename TArgument1> class Subscription1 : public Subscription {

      /// <summary>Initializes a new event subscription</summary>
      public: Subscription1() {
        // TODO: Add required variables
      }
      /// <summary>Terminates the event subscription</summary>
      public: virtual ~Subscription1() {}

    };

    #pragma endregion // class Subscription

    /// <summary>Initializes a new event subscriber</summary>
    public: EventSubscriber() {}

    /// <summary>Initializes a new event subscriber</summary>
    public: EventSubscriber(const EventSubscriber &) {
      // Do nothing here. Event subscriptions are not cloned.
    }

    /// <summary>Frees all resources owned by the instance</summary>
    public: virtual ~EventSubscriber() {
      // TODO: Unsubscribe events
    }

    /// <summary>Assigns another event subscriber to this one</summary>
    public: EventSubscriber &operator =(const EventSubscriber &) {
      // Do nothing here
    }

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_EVENTSUBSCRIBER_H
