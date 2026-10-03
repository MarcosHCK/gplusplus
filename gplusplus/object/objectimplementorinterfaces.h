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
#include <gplusplus/common/constexprregistry.h>
#include <gplusplus/object/objectclassinterface.h>
#include <gplusplus/object/objectimplementorbase.h>
#include <tuple>

namespace gplusplus::object::details::build_class
{

  template<typename Implementor,
           std::size_t T>
  struct interfaces_installer
    {

      using FirstTag = typename Implementor::interface_first_adl_tag;
      using PropTag = typename Implementor::interface_adl_tag;

      static inline constexpr auto First = constexpr_registry::guess_last<FirstTag, T, 0> ();
      static inline constexpr auto List = constexpr_registry::collect<PropTag, T, First> ();

      static inline constexpr auto n_interfaces = std::tuple_size_v<decltype (List)>;
    };
}