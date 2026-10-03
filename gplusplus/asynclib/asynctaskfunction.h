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
#include <gplusplus/asynclib/asynctask.h>

namespace gplusplus::asynclib::details
{

  template<typename>
  struct async_task_function_details
    {
      static inline constexpr bool valid = false;
    };

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           typename Functor,
           bool Noexcept,
           typename... Args>
  struct async_task_function_details<async_task<_Begin, _End, Functor> (*) (Args...) noexcept (Noexcept)>
    {

      static inline constexpr auto begin = _Begin;
      static inline constexpr auto end = _End;
      static inline constexpr bool noexcept_v = Noexcept;
      static inline constexpr bool valid = true;

      using arguments_tuple = std::tuple<Args ...>;
      using begin_details = async_function_begin_details<decltype (_Begin)>;
      using end_details = async_function_end_details<decltype (_End)>;
    };

  template<typename T>
  concept async_task_function = async_task_function_details<T>::valid;

  template<async_task_function auto _Function,
           typename Referrer>
  struct async_task_function_operable_finishable
    {

      using function_details = async_task_function_details<decltype (_Function)>;

      static inline constexpr typename function_details::end_details::return_type finish (Referrer referrer, GAsyncResult* async_result, GError** error)
          noexcept (function_details::end_details::noexcept_v)
        {
          return function_details::end (referrer, async_result, error);
        }
    };

  template<async_task_function auto _Function>
  struct async_task_function_operable_finishable<_Function, void>
    {

      using function_details = async_task_function_details<decltype (_Function)>;

      static inline constexpr typename function_details::end_details::return_type finish (GAsyncResult* async_result, GError** error)
          noexcept (function_details::end_details::noexcept_v)
        {
          return function_details::end (async_result, error);
        }
    };

  template<async_task_function auto _Function>
  struct async_task_function_operable: public async_task_function_operable_finishable<_Function,
                                        typename async_task_function_details<decltype (_Function)>::end_details::referrer_type>
    {

      static inline constexpr auto function = _Function;

      using function_type = decltype (_Function);
      using function_details = async_task_function_details<decltype (_Function)>;
    };
}