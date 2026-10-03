/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of gplusplus.
 *
 * gplusplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gplusplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once
#include <gio/gio.h>

namespace gplusplus::asynclib::details
{

  template<typename>
  struct async_function_end_details
    {
      static inline constexpr bool valid = false;
    };

  template<bool Noexcept, typename Result, typename Referrer>
  struct async_function_end_details<Result (*) (Referrer, GAsyncResult*, GError**) noexcept (Noexcept)>
    {

      using referrer_type = Referrer;
      using return_type = Result;

      static inline constexpr bool noexcept_v = Noexcept;
      static inline constexpr bool valid = true;
    };

  template<bool Noexcept, typename Result>
  struct async_function_end_details<Result (*) (GAsyncResult*, GError**) noexcept (Noexcept)>
    {

      using referrer_type = void;
      using return_type = Result;

      static inline constexpr bool noexcept_v = Noexcept;
      static inline constexpr bool valid = true;
    };

  template<typename T>
  concept async_function_end = async_function_end_details<T>::valid;
}