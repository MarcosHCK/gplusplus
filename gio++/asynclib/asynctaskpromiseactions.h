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
#include <gio++/common/soo.h>

namespace gioplusplus::asynclib::details
{

  template<typename T>
    requires (std::is_destructible_v<T>)
  static inline constexpr void _g_task_return_object_notify (gpointer ptr) noexcept (std::is_nothrow_destructible_v<T>)
    {
      soo_ptr::destroy<T> (&ptr);
    }

  template<typename T,
           std::convertible_to<T> U = T>
    requires (std::is_constructible_v<T, U>)
  static inline constexpr void g_task_return_object (GTask* task, U&& value) noexcept (std::is_nothrow_constructible_v<T, U>)
    {
      gpointer pointer;
      soo_ptr::create<T> (&pointer, std::forward<U> (value));
    return g_task_return_pointer (task, pointer, _g_task_return_object_notify<T>);
    }

  template<> inline constexpr void g_task_return_object<bool, bool> (GTask* task, bool&& value) noexcept
    {
      return g_task_return_boolean (task, value);
    }

  template<typename T>
    requires (std::is_move_constructible_v<T>)
  static inline constexpr T g_task_propagate_object (GTask* task, GError** error) noexcept (std::is_nothrow_move_constructible_v<T>)
    {

      GError* tmperr = NULL;

      if (auto ptr = g_task_propagate_pointer (task, &tmperr); G_LIKELY (NULL == tmperr))

        return std::move (*soo_ptr::cast<T> (&ptr));
      else
        return (g_propagate_error (error, tmperr), T ());
    }
}