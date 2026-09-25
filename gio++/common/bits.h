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
#include <climits>
#include <concepts>
#include <limits>

namespace bits
{

  template<std::size_t _Value>
    requires (_Value > 0)
    struct log2 { static inline constexpr std::size_t value = 1 + log2<_Value / 2>::value; };

  template<>
    struct log2<1> { static inline constexpr std::size_t value = 0; };

  template<std::size_t _Value>
    static inline constexpr std::size_t log2_v = log2<_Value>::value;

  template<std::size_t _Value>
    static inline constexpr bool is_pow2_v = _Value > 0 && (_Value & (_Value - 1)) == 0;
}

namespace bits
{

  template<std::size_t _By,
           std::unsigned_integral T>
    requires (0 < _By && _By < static_cast<std::size_t> (std::numeric_limits<T>::max ()))
  [[gnu::always_inline]]
  static inline constexpr T align_up (T value) noexcept
    {
      return ((value + (static_cast<T> (_By) - 1)) / static_cast<T> (_By)) * static_cast<T> (_By);
    }

  template<std::size_t _By,
           std::unsigned_integral T>
    requires (0 < _By && _By < sizeof (T) * CHAR_BIT)
  [[gnu::always_inline]]
  static inline constexpr T rot (T value) noexcept
    {
      return (value << _By) | (value >> (sizeof (T) * CHAR_BIT - _By));
    }
}