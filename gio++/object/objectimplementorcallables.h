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
#include <gio++/object/objectimplementorstructs.h>

namespace gioplusplus::object::details
{

  /* { base_class_init } */

  template<typename Implementor>
  concept has_base_init = requires (build_class::class_struct<Implementor>* c)
    { Implementor::base_class_init (c); };

  template<typename Implementor, typename>
  struct base_init_callable { static inline constexpr auto value = nullptr; };

  template<has_base_init Implementor, typename ClassType>
  struct base_init_callable<Implementor, ClassType> { static inline constexpr auto value = [](gpointer c) noexcept -> void
    { Implementor::base_class_init ((ClassType*) c); }; };

  /* { base_class_fini } */

  template<typename Implementor>
  concept has_base_fini = requires (build_class::class_struct<Implementor>* c)
    { Implementor::base_class_fini (c); };

  template<typename Implementor, typename>
  struct base_fini_callable { static inline constexpr auto value = nullptr; };

  template<has_base_fini Implementor, typename ClassType>
  struct base_fini_callable<Implementor, ClassType> { static inline constexpr auto value = [](gpointer c) noexcept -> void
    { Implementor::base_class_fini ((ClassType*) c); }; };

  /* { class_init } */

  template<typename Implementor>
  concept has_class_init = requires (build_class::class_struct<Implementor>* c, gpointer d)
    { Implementor::class_init (c, d); };

  template<typename Implementor, typename>
  struct class_init_callable { static inline constexpr auto value = nullptr; };

  template<has_class_init Implementor, typename ClassType>
  struct class_init_callable<Implementor, ClassType> { static inline constexpr auto value = [](gpointer c, gpointer d) noexcept -> void
    { Implementor::class_init ((ClassType*) c, d); }; };

  /* { class_fini } */

  template<typename Implementor>
  concept has_class_fini = requires (build_class::class_struct<Implementor>* c, gpointer d)
    { Implementor::class_fini (c, d); };

  template<typename Implementor, typename>
  struct class_fini_callable { static inline constexpr auto value = nullptr; };

  template<has_class_fini Implementor, typename ClassType>
  struct class_fini_callable<Implementor, ClassType> { static inline constexpr auto value = [](gpointer c, gpointer d) noexcept -> void
    { Implementor::class_fini ((ClassType*) c, d); }; };

  /* { instance_init } */

  template<typename Implementor>
  concept has_instance_init = requires (build_class::instance_struct<Implementor>* s,
                                        build_class::class_struct<Implementor>* c)
    { Implementor::instance_init (s, c); };

  template<typename Implementor, typename, typename>
  struct instance_init_callable { static inline constexpr auto value = nullptr; };

  template<has_instance_init Implementor, typename ClassType, typename InstanceType>
  struct instance_init_callable<Implementor, ClassType, InstanceType> { static inline constexpr auto value = [](GTypeInstance* i, gpointer c) noexcept -> void
    { Implementor::instance_init ((InstanceType*) i, (ClassType*) c); }; };
}