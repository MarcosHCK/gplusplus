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
#include <memory>
#include <utility>

template<typename T>
class alignas (1) unaligned_store
{

  static inline constexpr auto align = alignof (T);
  static inline constexpr auto size = sizeof (T);
  std::byte _region [1 == align ? size : align + size];

  inline constexpr auto aligned_region () const noexcept
    {
      auto miss = ((std::uintptr_t) _region) % align;
      return &_region [(align - miss) % align];
    }

  inline constexpr auto aligned_region () noexcept
    {
      auto miss = ((std::uintptr_t) _region) % align;
      return &_region [(align - miss) % align];
    }

public:

  inline constexpr ~unaligned_store () noexcept (std::is_nothrow_destructible_v<T>)
    {
      std::destroy_at ((T*) aligned_region ());
    }

  inline constexpr unaligned_store (unaligned_store&& o) noexcept (std::is_nothrow_move_constructible_v<T>)
    {
      std::construct_at ((T*) aligned_region (), std::move (*o));
    }

  inline constexpr unaligned_store (const unaligned_store& o) noexcept (std::is_nothrow_copy_constructible_v<T>)
    {
      std::construct_at ((T*) aligned_region (), o.operator* ());
    }

  template<typename... Args>
    requires std::constructible_from<T, Args ...>
  inline constexpr unaligned_store (Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
    {
      std::construct_at ((T*) aligned_region (), std::forward<Args> (args) ...);
    }

  inline constexpr const T& operator* () const noexcept { return (const T&) *this; }
  inline constexpr T& operator* () noexcept { return (T&) *this; }

  inline constexpr operator const T& () const noexcept { return *(const T*) aligned_region (); }
  inline constexpr operator T& () noexcept { return *(T*) aligned_region (); }
};