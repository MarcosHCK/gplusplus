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
#include <gplusplus/object/objectimplementorbase.h>

namespace gplusplus::object::details::build_class
{

  template<typename Implementor>
  struct class_struct: public object_class_structs<typename Implementor::Type, ancestor_or_default<Implementor>::value>::class_struct,
                       public vtable_or_default<Implementor>::type
    {
    };

  template<typename Implementor>
  struct instance_struct: public object_class_structs<typename Implementor::Type, ancestor_or_default<Implementor>::value>::instance_struct
    {
    };
}