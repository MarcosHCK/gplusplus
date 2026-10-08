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
#include <concepts>
#include <gio/gio.h>

namespace gplusplus::asynclib::details
{

  template<typename T>
  concept async_task_type = requires ()
    {

      typename T::begin_details;
      typename T::end_details;
      typename T::end_details::return_type;

      requires requires (T e, GAsyncReadyCallback ac, gpointer ud)
        { { e (ac, ud) } -> std::same_as<void>; };

      requires requires (GObject* so, GAsyncResult* ar, GError** er)
        { { T::finish (so, ar, er) } -> std::same_as<typename T::end_details::return_type>; };
    };
}