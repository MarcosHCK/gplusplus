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
#include <gplusplus/object/objectclassconcepts.h>
#include <glib-object.h>

namespace gplusplus::object
{

  template<typename IfaceType,
           details::invocable_r<GType> auto GetType,
           details::invocable_r<void, gpointer, gpointer> auto IfaceInit>
  struct object_class_interface
    {

      typedef IfaceType Iface;

      static inline constexpr auto get_type = GetType;
      static inline constexpr auto iface_init = IfaceInit;
    };
}