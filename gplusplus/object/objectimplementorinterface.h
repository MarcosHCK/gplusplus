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
#include <gplusplus/object/objectclassinterface.h>

namespace gplusplus::object::details
{

  template<typename Type_,
           typename TypeIface_,
           details::invocable_r<GType> auto GetType,
           details::invocable_r<void, TypeIface_*, gpointer> auto IfaceInit,
           details::invocable_r_or_null<void, TypeIface_*, gpointer> auto IfaceFini = nullptr>
  struct object_class_interface_
    {

      static inline constexpr auto iface_init = [](gpointer iface, gpointer iface_data) noexcept
        { IfaceInit ((TypeIface_*) iface, iface_data); };

      template<typename T = decltype (IfaceFini)>
      struct iface_fini
        {

          static inline constexpr auto value = [](gpointer iface, gpointer iface_data) noexcept
            { IfaceFini ((TypeIface_*) iface, iface_data); };
        };

      template<>
      struct iface_fini<std::nullptr_t> { static inline constexpr auto value = nullptr; };

      using type = object::object_class_interface<Type_, TypeIface_, GetType, iface_init, iface_fini<>::value>;
    };
}