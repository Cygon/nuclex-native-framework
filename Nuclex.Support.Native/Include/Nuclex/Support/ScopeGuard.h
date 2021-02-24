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

#ifndef NUCLEX_SUPPORT_SCOPEGUARD_H
#define NUCLEX_SUPPORT_SCOPEGUARD_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::nullptr_t
#include <utility> // for std::forward

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>RAII helper that executes a lambda expression when going out of scope</summary>
  /// <typeparam name="TLambda">Lambda expression that will be executed</typeparam>
  /// <remarks>
  ///   <para>
  ///     This is a C++14 implementation of the well-known scope guard concept. A scope guard
  ///     is a stack-allocated empty object which will run some cleanup code when the scope in
  ///     which it lives is exited.
  ///   </para>
  ///   <para>
  ///     This ensures that the cleanup code always runs, even when the method is terminated
  ///     early by an exception. It is faster and better than try..catch..(re-)throw because
  ///     it will not interupt the exception (this keeping the original site of the exception
  ///     for any debuggers or error reporting tools).
  ///   </para>
  ///   <para>
  ///     <code>
  ///       void Dummy() {
  ///         FILE *file = ::fopen(u8"myfile", "rb");
  ///         if(file == nullptr) {
  ///           throw std::runtime_error(u8"Could not open 'myfile' for reading");
  ///         }
  ///
  ///         ON_SCOPE_EXIT { ::fclose(file); }
  ///
  ///         std::uint32_t magic = 0;
  ///         ::fread(&magic, sizeof(magic), 1, file);
  ///         enforceMatchingSignature(magic);
  ///
  ///         complexCodeThatMightThrow(file);
  ///       }
  ///     </code>
  ///   </para>
  /// </remarks>
  template<typename TLambda>
  class ScopeGuard {

    /// <summary>
    ///   Initializes a new scope guard running the specified expression upon destruction
    /// </summary>
    /// <param name="cleanUpExpression">
    ///   Lambda expression with the clean up code that needs to be executed
    /// </param>
    public: ScopeGuard(TLambda &&cleanUpExpression) :
      cleanUpExpression(std::forward<TLambda>(cleanUpExpression)) {}

    /// <summary>Executes the cleanup code as the scope guard leaves scope</summary>
    public: ~ScopeGuard() {
      cleanUpExpression();
    }

    /// <summary>Lambda expression containing the cleanup code</summary>
    private: TLambda cleanUpExpression;

  };

  // ------------------------------------------------------------------------------------------- //
#if 0
  /// <summary>Creates scope guard running the specified clean-up code</summary>
  /// <param name="cleanUpExpression">Lambda expression with the clean-up code</param>
  /// <returns>A scope guard running the specified clean-up code</returns>
  template<typename TLambda>
  ScopeGuard<TLambda> MakeScopeGuard(TLambda &&cleanUpExpression) {
    return ScopeGuard<TLambda>(std::forward<TLambda>(cleanUpExpression));
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  /// <summary>RAII helper that executes a lambda expression when going out of scope</summary>
  /// <typeparam name="TLambda">Lambda expression that will be executed</typeparam>
  /// <remarks>
  ///   <para>
  ///     See the <see cref="ScopeGuard" /> class for a general introduction into scope guards.
  ///     This variant of the scope guard can be 'disarmed' in case you only want to run
  ///     the cleanup code if the scope is exited due to an error.
  ///   </para>
  ///   <para>
  ///     <code>
  ///       void Dummy(SceneGraph &sceneGraph) {
  ///         Entity *spider = sceneGraph.NewEntity();
  ///         auto spiderSceneGraphGuard = ON_SCOPE_EXIT_TRANSACTION {
  ///           sceneGraph.RemoveEntity(spider);
  ///         }
  ///
  ///         spider->FindClosestPlayer();
  ///         spider->SetAggroMode();
  ///
  ///         // If no exception was thrown up to this point, the spider is ready
  ///         // to attack and we can keep it in the scene graph.
  ///         spiderSceneGraphGuard.Commit();
  ///       }
  ///     </code>
  ///   </para>
  ///   <para>
  ///     In the above example, the call to 'Commit()' will disable the scope guard and
  ///     prevent it from removing the spider from the scene graph again. In other words,
  ///     the transaction (that is, the whole process of creating and setting up the spider)
  ///     is complete and can be committed, thus no longer needs to be rolled back on exit.
  ///   </para>
  /// </remarks>
  template<typename TLambda>
  class TransactionalScopeGuard {

    /// <summary>
    ///   Initializes a new scope guard running the specified expression upon destruction
    /// </summary>
    /// <param name="cleanUpExpression">
    ///   Lambda expression with the clean up code that needs to be executed
    /// </param>
    public: TransactionalScopeGuard(TLambda &&cleanUpExpression) :
      cleanUpExpression(std::forward<TLambda>(cleanUpExpression)),
      armed(true) {}

    /// <summary>Executes the cleanup code as the scope guard leaves scope</summary>
    public: ~TransactionalScopeGuard() {
      if(this->armed) {
        cleanUpExpression();
      }
    }

    /// <summary>Disarms the scope guard, preventing the clean up code from running</summary>
    /// <remarks>
    ///   This method is typically used when your cleanup code reverts some change
    ///   the should become permanent unless the scope is exited through an exception.
    /// </remarks>
    public: void Commit() {
      this->armed = false;
    }

    /// <summary>Lambda expression containing the cleanup code</summary>
    private: TLambda cleanUpExpression;
    /// <summary>Whether the scope guard will execute the clean upcode</summary>
    private: bool armed;

  };

  // ------------------------------------------------------------------------------------------- //
#if 0
  /// <summary>Creates scope guard running the specified clean-up code</summary>
  /// <param name="cleanUpExpression">Lambda expression with the clean-up code</param>
  /// <returns>A scope guard running the specified clean-up code</returns>
  template<typename TLambda>
  TransactionalScopeGuard<TLambda> MakeTransactionalScopeGuard(TLambda &&cleanUpExpression) {
    return TransactionalScopeGuard<TLambda>(std::forward<TLambda>(cleanUpExpression));
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

// --------------------------------------------------------------------------------------------- //

// Typical C-style macro to concatenate two names in the preprocessor
#define NUCLEX_SUPPORT_CONCAT_IMPL(x, y) x##y
#define NUCLEX_SUPPORT_CONCAT(x, y) NUCLEX_SUPPORT_CONCAT_IMPL(x, y)

// Macro to give scope guards unique names, either sequential or line numbers
#if defined(__COUNTER__)
  #define NUCLEX_SUPPORT_UNIQUE_VARIABLE(name) NUCLEX_SUPPORT_CONCAT(name, __COUNTER__)
#else
  #define NUCLEX_SUPPORT_UNIQUE_VARIABLE(name) NUCLEX_SUPPORT_CONCAT(name, __LINE__)
#endif

// --------------------------------------------------------------------------------------------- //

/// <summary>Creates scope guard running the specified clean-up code</summary>
/// <param name="cleanUpExpression">Lambda expression with the clean-up code</param>
/// <returns>A scope guard running the specified clean-up code</returns>
template<typename TLambda>
::Nuclex::Support::ScopeGuard<TLambda> operator +(std::nullptr_t, TLambda &&cleanUpExpression) {
  return ::Nuclex::Support::ScopeGuard<TLambda>(std::forward<TLambda>(cleanUpExpression));
}

// Macro that allows you to conveniently define some code to be run at scope exit
#define ON_SCOPE_EXIT auto NUCLEX_SUPPORT_UNIQUE_VARIABLE(onScopeExit) = nullptr + [&]()

// --------------------------------------------------------------------------------------------- //

/// <summary>Creates scope guard running the specified clean-up code</summary>
/// <param name="cleanUpExpression">Lambda expression with the clean-up code</param>
/// <returns>A scope guard running the specified clean-up code</returns>
template<typename TLambda>
::Nuclex::Support::TransactionalScopeGuard<TLambda> operator -(
  std::nullptr_t, TLambda &&cleanUpExpression
) {
  return ::Nuclex::Support::TransactionalScopeGuard<TLambda>(
    std::forward<TLambda>(cleanUpExpression)
  );
}

// Macro that allows you to conveniently define some code to be run at scope exit
#define ON_SCOPE_EXIT_TRANSACTION nullptr - [&]()

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_SUPPORT_SCOPEGUARD_H
