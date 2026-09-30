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
#include <cstddef>
#include <string_view>

template<std::size_t N,
         std::integral T = char>
struct constexpr_string
{

  std::size_t _size;
  T _value [N + 1];

  inline constexpr constexpr_string (const char (&value) [N]) noexcept:
      _size (0 != value [N - 1] ? N : N - 1), _value ()
    {

      _value [_size] = 0;

      for (std::remove_cvref_t<decltype (_size)> i = 0; i < _size; ++i)
        _value [i] = value [i];
    }

  inline constexpr auto c_str () const noexcept { return _value; }
  inline constexpr auto size () const noexcept { return _size; }

  inline constexpr operator std::string_view () const noexcept { return std::string_view (_value, _size); }
};