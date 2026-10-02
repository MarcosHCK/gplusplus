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
#include <gio++/object/objectclassconcepts.h>
#include <glib-object.h>

namespace gioplusplus::object
{

  template<typename ClassType = GObjectClass,
           typename InstanceType = GObject,
           details::invocable_r<GType> auto GetType = g_object_get_type>
  struct object_class_ancestor
    {

      typedef ClassType Class;
      typedef InstanceType Instance;

      static inline constexpr auto get_type = GetType;
    };
}