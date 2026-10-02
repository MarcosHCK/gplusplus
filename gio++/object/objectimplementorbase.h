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
#include <gio++/object/objectclass.h>

namespace gioplusplus::object::details
{

  template<typename T>
  struct store_self { using Self = T; };

  /* { Ancestor } */

  template<typename Implementor>
  concept has_ancestor = requires () { Implementor::Ancestor; };

  template<typename Implementor>
  struct ancestor_or_default { static inline constexpr auto value = object_class_ancestor (); };

  template<has_ancestor Implementor>
  struct ancestor_or_default<Implementor> { static inline constexpr auto value = Implementor::Ancestor; };

  /* { VTable } */

  template<typename Implementor>
  concept has_vtable = requires () { typename Implementor::VTable; };

  template<typename Implementor>
  struct vtable_or_default { struct type { }; };

  template<has_vtable Implementor>
  struct vtable_or_default<Implementor> { struct type: public Implementor::VTable { }; };
}