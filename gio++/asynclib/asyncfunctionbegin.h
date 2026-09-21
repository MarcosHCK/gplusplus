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
#include <gio/gio.h>
#include <tuple>

namespace gioplusplus::asynclib::details
{

  template<typename>
  struct async_function_begin_details
    {
      static inline constexpr bool valid = false;
    };

  template<bool Noexcept, typename... Args>
  struct async_function_begin_details<void (*) (Args...) noexcept (Noexcept)>
    {

      using arguments_tuple = std::tuple<Args ...>;

      static inline constexpr auto arguments_n = sizeof... (Args) - 2;
      static inline constexpr auto noexcept_v = Noexcept;

      static inline constexpr bool valid = std::is_convertible_v<typename std::tuple_element_t<sizeof... (Args) - 2, arguments_tuple>, GAsyncReadyCallback>
                                        && std::is_convertible_v<typename std::tuple_element_t<sizeof... (Args) - 1, arguments_tuple>, gpointer>;

    private:

      template<size_t... Is>
      static auto get_signature_type (std::index_sequence<Is...>)
        -> void (*) (typename std::tuple_element_t<Is, arguments_tuple> ...) noexcept (Noexcept);

    public:
      using signature_type = typename std::remove_pointer_t<decltype (get_signature_type (std::make_index_sequence<sizeof... (Args) - 2> ()))>;
    };

  template<typename T>
  concept async_function_begin = async_function_begin_details<T>::valid;
}