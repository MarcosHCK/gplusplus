/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of gio++.
 *
 * gio++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gio++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once
#include <glib.h>
#include <memory>

namespace soo_ptr
{

  namespace details
    {

      typedef void* (*allocator_alloc) (size_t bytes);
      typedef void (*allocator_free) (size_t bytes, void* location);

      template<typename T,
               typename Location>
      static inline constexpr bool constructible_inplace = sizeof (Location) >= sizeof (T)
                                                        && alignof (Location) >= alignof (T);

      template<typename T, typename Location, details::allocator_alloc Alloc, details::allocator_free Free, typename... Args>
        requires std::constructible_from<T, Args ...>
      static inline T* create (Location* location, Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>);

      template<typename T, typename Location, details::allocator_alloc Alloc, details::allocator_free Free>
      struct creator;

      template<typename T, typename Location, details::allocator_free Free>
        requires std::destructible<T>
      static inline void destroy (Location* location, T* pointer) noexcept (std::is_nothrow_destructible_v<T>);
    }

  template<typename T>
  static inline constexpr const T* cast (void* const* location) noexcept
    {

      if constexpr (details::constructible_inplace<T, void*>)

        return (T*) location;
      else
        return (T*) *location;
    }

  template<typename T>
  static inline constexpr T* cast (void** location) noexcept
    {

      if constexpr (details::constructible_inplace<T, void*>)

        return (T*) location;
      else
        return (T*) *location;
    }

  template<typename T, typename Location, details::allocator_alloc Alloc, details::allocator_free Free, typename... Args>
    requires std::constructible_from<T, Args ...>
  static inline T* details::create (Location* location, Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
    {

      if constexpr (details::constructible_inplace<T, Location>)
        return std::construct_at ((T*) location, std::forward<Args> (args) ...);

      else if constexpr (std::is_nothrow_constructible_v<T, Args ...>)
        return std::construct_at ((T*) Alloc (sizeof (T)), std::forward<Args> (args) ...);

      else if constexpr (auto ptr = Alloc (sizeof (T)); true) try {
        return std::construct_at ((T*) ptr, std::forward<Args> (args) ...); }
  
      catch (...) {
        Free (sizeof (T), ptr); throw; }
    }

  template<typename T, typename Location, details::allocator_alloc Alloc, details::allocator_free Free>
  struct details::creator
    {

      template<typename... Args>
        requires std::constructible_from<T, Args ...>
      static inline T* operator() (Location* location, Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
        {
          return details::create<T, Location, Alloc, Free> (location, std::forward<Args> (args) ...);
        }
    };

  template<typename T, details::allocator_alloc Alloc, details::allocator_free Free>
  struct details::creator<T, void*, Alloc, Free>
    {

      template<typename... Args>
        requires std::constructible_from<T, Args ...>
      static inline T* operator() (void** location, Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
        {

          if constexpr (details::constructible_inplace<T, void*>)

            return details::create<T, void*, Alloc, Free> (location, std::forward<Args> (args) ...);
          else
            return (T*) (*location = (void*) details::create<T, void*, Alloc, Free> (location, std::forward<Args> (args) ...));
        }
    };

  template<typename T, typename Location, details::allocator_free Free>
    requires std::destructible<T>
  static inline void details::destroy (Location* location, T* pointer) noexcept (std::is_nothrow_destructible_v<T>)
    {

      if constexpr (details::constructible_inplace<T, Location>)

        { return (std::destroy_at ((T*) location)); }
      else
        { return (std::destroy_at ((T*) pointer), Free (sizeof (T), pointer)); }
    }

  /* { public } */

  template<typename T,
           typename Location = void*,
           details::allocator_alloc Alloc = g_slice_alloc,
           details::allocator_free Free = g_slice_free1,
           typename... Args>
    requires std::constructible_from<T, Args ...>
  static inline T* create (Location* location, Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
    {
      return (details::creator<T, Location, Alloc, Free> { }) (location, std::forward<Args> (args) ...);
    }

  template<typename T,
           typename Location,
           details::allocator_free Free = g_slice_free1>
    requires (std::is_destructible_v<T> && std::same_as<void*, Location>)
  static inline void destroy (Location* location) noexcept (std::is_nothrow_destructible_v<T>)
    {
      details::destroy<T, Location, Free> (location, cast<T> (location));
      *location = nullptr;
    }

  template<typename T,
           typename Location,
           details::allocator_free Free = g_slice_free1>
    requires (std::is_destructible_v<T>)
  static inline void destroy (Location* location, T* pointer) noexcept (std::is_nothrow_destructible_v<T>)
    {
      details::destroy<T, Location, Free> (location, pointer);
    }
}