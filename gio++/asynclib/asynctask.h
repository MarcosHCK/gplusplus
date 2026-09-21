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
#include <gio++/asynclib/asyncfunctionbegin.h>
#include <gio++/asynclib/asyncfunctionend.h>

namespace gioplusplus::asynclib::details
{

  template<async_function_end auto _End,
           typename Referrer>
  struct async_task_completer
    {

      using end_details = async_function_end_details<decltype (_End)>;

      static inline typename end_details::return_type finish (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept (end_details::noexcept_v)
        {
          return _End ((Referrer) source_object, async_result, error);
        }
    };

  template<async_function_end auto _End>
  struct async_task_completer<_End, void>
    {

      using end_details = async_function_end_details<decltype (_End)>;

      static inline typename end_details::return_type finish (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept (end_details::noexcept_v)
        {
          return _End (async_result, error);
        }
    };

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           typename Functor>
    requires (std::is_invocable_r_v<void, Functor, GAsyncReadyCallback, gpointer>)
  struct async_task: public async_task_completer<_End, typename async_function_end_details<decltype (_End)>::referrer_type>
    {

      Functor begin;
      using begin_details = async_function_begin_details<decltype (_Begin)>;
      using end_details = async_function_end_details<decltype (_End)>;

      static inline constexpr bool noexcept_v = std::is_nothrow_invocable_v<Functor, GAsyncReadyCallback, gpointer>;

      inline constexpr async_task (Functor&& _begin) noexcept: begin (std::forward<Functor&&> (_begin))
        { }

      inline constexpr void operator() (GAsyncReadyCallback callback, gpointer user_data) noexcept (noexcept_v)
        {
          begin (callback, user_data);
        }
    };
}