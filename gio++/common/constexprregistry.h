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
#include <tuple>

namespace constexpr_registry
{

  template<typename Tag, std::size_t N>
  struct anchor
    {
      friend constexpr auto adl_probe (anchor<Tag, N>);
    };

  template<typename Tag, std::size_t N, auto Value>
  struct register_
    {
      friend constexpr auto adl_probe (anchor<Tag, N>) { return Value; }
    };

  template<typename Tag, std::size_t N>
  concept probe = requires { adl_probe (anchor<Tag, N> {}); };

  template<typename Tag, std::size_t T, std::size_t N = 0, auto... Vs>
  static inline constexpr auto collect () noexcept
    {

      if constexpr (N >= T)
        return std::tuple<decltype (Vs) ...> (Vs ...);

      else if constexpr (! probe<Tag, N>)

        return collect<Tag, T, N + 1, Vs ...> ();
      else
        return collect<Tag, T, N + 1, Vs ..., adl_probe (anchor<Tag, N> {})> ();
    }

  template<typename Tag, std::size_t N, auto Default>
  static inline constexpr auto guess_last () noexcept
    {

      if constexpr (N == 0)
        return Default;

      else if constexpr (probe<Tag, N>)

        return adl_probe (anchor<Tag, N> {});
      else
        return guess_last<Tag, N - 1, Default> ();
    }

  template<typename Tag, std::size_t N, auto Value>
    requires (N >= 1)
  struct install_once: register_<Tag, N, constexpr_registry::guess_last<Tag, N - 1, Value> ()>
    {
    };
}