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
#include <gio++/common/constexprregistry.h>
#include <gio++/object/objectimplementorbase.h>
#include <utility>

namespace gioplusplus::object::details::build_class
{

  template<typename Implementor,
           std::size_t T>
  struct properties_installer
    {

      using FirstTag = typename Implementor::property_first_adl_tag;
      using PropTag = typename Implementor::property_adl_tag;

      static inline constexpr auto First = constexpr_registry::guess_last<FirstTag, T, 0> ();
      static inline constexpr auto List = constexpr_registry::collect<PropTag, T, First> ();

      static inline constexpr auto n_properties = std::tuple_size_v<decltype (List)>;

      template<typename Type>
      static inline constexpr auto install (auto property) noexcept
        {
          return Type::create_spec (Type::Tag::flags);
        }

      static inline constexpr auto install (auto* klass) noexcept
        {

          auto specs = install (std::make_integer_sequence<size_t, n_properties> ());

          if constexpr (n_properties > 0)
            g_object_class_install_properties (G_OBJECT_CLASS (klass), specs.size (), specs.begin ());
        }

      template<size_t... Is>
      static inline constexpr auto install (std::integer_sequence<size_t, Is ...> const&) noexcept
        {
          return std::array<GParamSpec*, 1 + std::tuple_size_v<decltype (List)>> {
              nullptr, install<std::tuple_element_t<Is, decltype (List)>> (std::get<Is> (List)) ... };
        }
    };
}