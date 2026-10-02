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
#include <gio++/object/objectimplementorbase.h>
#include <gio++/object/objectimplementorcallables.h>
#include <gio++/object/objectimplementorstructs.h>

namespace gioplusplus::object::details::build_class
{

  template<typename Implementor>
  struct object_class_
    {

      using Type = typename Implementor::Type;
      using TypeTag = typename Implementor::TypeTag;

    struct Implementor_: public Implementor
      {

        static inline constexpr auto Ancestor = ancestor_or_default<Implementor>::value;

        using VTable = typename vtable_or_default<Implementor>::type;

        using ClassType = typename build_class::class_struct<Implementor>;
        using InstanceType = typename build_class::instance_struct<Implementor>;

        static inline constexpr auto base_class_init = base_init_callable<Implementor, ClassType>::value;
        static inline constexpr auto base_class_fini = base_fini_callable<Implementor, ClassType>::value;
        static inline constexpr auto class_init = class_init_callable<Implementor, ClassType>::value;
        static inline constexpr auto class_fini = class_fini_callable<Implementor, ClassType>::value;
        static inline constexpr auto instance_init = instance_init_callable<Implementor, ClassType, InstanceType>::value;
      };

      static inline constexpr auto Name = Implementor::ClassName;

      using type = object_class<typename Implementor_::ClassType, typename Implementor_::InstanceType,
                                Name, TypeTag,
                                Implementor_::base_class_init, Implementor_::base_class_fini,
                                Implementor_::class_init, Implementor_::class_fini, Implementor_::instance_init,
                                Implementor_::Ancestor>;
    };

  template<typename Implementor>
  using object_class = typename object_class_<Implementor>::type;
}