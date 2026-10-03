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
#include <gplusplus/asynclib/asyncfunctionbegin.h>
#include <gplusplus/asynclib/asyncfunctionend.h>
#include <gplusplus/asynclib/asynctask.h>

namespace gplusplus::asynclib::details
{

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           typename>
  struct async_function_base;

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           bool Noexcept,
           typename... Args>
  struct async_function_base<_Begin, _End, void (Args ...) noexcept (Noexcept)>
    {

      using begin_details = async_function_begin_details<decltype (_Begin)>;
      using end_details = async_function_end_details<decltype (_End)>;

      inline constexpr auto operator() (Args... args) const
        {

          auto lambda = [...args = std::forward<Args> (args)]
                        (GAsyncReadyCallback callback, gpointer user_data) noexcept (Noexcept) -> void
            {
              _Begin (args..., callback, user_data);
            };
        return async_task<_Begin, _End, decltype (lambda)> (std::move (lambda));
        }
    };

  template<async_function_begin auto _Begin,
           async_function_end auto _End>
  struct async_function: public async_function_base<_Begin, _End, typename async_function_begin_details<decltype (_Begin)>::signature_type>
    {
    };
}