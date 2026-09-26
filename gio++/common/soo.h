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
#include <new>
#include <utility>

namespace soo_ptr
{

  namespace details
    {

      typedef void* (*allocator_alloc) (size_t bytes);
      typedef void (*allocator_free) (size_t bytes, void* location);
    }

  template<typename T>
  static inline const T* cast (const void** location) noexcept
    {

      if constexpr (sizeof (void*) >= sizeof (T) && alignof (void*) >= alignof (T))

        return (const T*) location;
      else
        return (const T*) *location;
    }

  template<typename T>
  static inline T* cast (void** location) noexcept
    {

      if constexpr (sizeof (void*) >= sizeof (T) && alignof (void*) >= alignof (T))

        return (T*) location;
      else
        return (T*) *location;
    }

  template<typename T,
           details::allocator_alloc Alloc = g_slice_alloc,
           details::allocator_free Free = g_slice_free1,
           typename... Args>
    requires (std::is_constructible_v<T, Args ...>)
  static inline T* create (void** location, Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
    {

      if constexpr (sizeof (void*) >= sizeof (T) && alignof (void*) >= alignof (T))
        return new (location) T (std::forward<Args> (args) ...);

      else if constexpr (std::is_nothrow_constructible_v<T, Args ...>)
        return new (*location = Alloc (sizeof (T))) T (std::forward<Args> (args) ...);

      else
        { *location = Alloc (sizeof (T));

          try
            { return new (*location) T (std::forward<Args> (args) ...); }
          catch (...)
            { Free (sizeof (T), *location); *location = nullptr; throw; }
        }
    }

  template<typename T, details::allocator_free Free = g_slice_free1>
    requires (std::is_destructible_v<T>)
  static inline void destroy (void** location) noexcept (std::is_nothrow_destructible_v<T>)
    {

      if constexpr (sizeof (void*) >= sizeof (T) && alignof (void*) >= alignof (T))

        return ((T*) location)->~T ();
      else
        return ((T*) *location)->~T (), Free (sizeof (T), *location);
    }
}