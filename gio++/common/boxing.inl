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
#include <glib-object.h>
#include <type_traits>

namespace boxing::details
{

  template<typename F, typename T>
  concept _copy_func = std::is_invocable_r_v<T*, F, T*> || std::is_same_v<F, std::nullptr_t>;

  template<typename F, typename T>
  concept _free_func = std::is_invocable_r_v<void, F, T*> || std::is_same_v<F, std::nullptr_t>;

  static GBytes* _g_bytes_ref (GBytes* bytes) noexcept
    { return NULL == bytes ? NULL : g_bytes_ref (bytes); }

  template<typename T>
  static void _g_free (T* object) noexcept { return g_free ((void*) object); }

  template<typename T>
  static T* _g_object_ref (T* object) noexcept
    { return NULL == object ? NULL : g_object_ref (object); }

  template<typename T>
  static void _g_object_unref (T* object) noexcept { return g_object_unref (object); }
}