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
#include <array>
#include <gplusplus/common/constexprregistry.h>
#include <gplusplus/object/objectimplementorbase.h>
#include <gplusplus/object/objectimplementorstructs.h>
#include <tuple>
#include <utility>

namespace gplusplus::object::details::build_class
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

      /* { vfuncs } */

      template<auto Func>
      static void accessor (auto* self, guint property_id, auto value, GParamSpec* pspec) noexcept
        {

          if constexpr (! std::same_as<std::nullptr_t, decltype (Func)>)

            Func (self, value, pspec);
          else
            G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), property_id, pspec);
        }

      static void get_property (GObject* p_self, guint property_id, GValue* value, GParamSpec* pspec) noexcept
        {
          get_property_ (p_self, property_id, value, pspec, std::make_integer_sequence<size_t, n_properties> { });
        }

      template<size_t... Is>
      static void get_property_ (auto* p_self, guint property_id, GValue* value, GParamSpec* pspec,
          std::integer_sequence<size_t, Is ...> const&) noexcept
        {

          using fn = void (*) (build_class::instance_struct<Implementor>*, guint, GValue*, GParamSpec*);

          static constexpr fn table [] =
            {

              [] (auto self, guint property_id, GValue* value, GParamSpec* pspec)
                { G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), property_id, pspec); },

              [] (auto self, guint property_id, GValue* value, GParamSpec* pspec)
                { accessor<std::tuple_element_t<Is, decltype (List)>::get_property> (self, property_id, value, pspec); }
              ...
            };

          if (auto self = (build_class::instance_struct<Implementor>*) p_self; 0 < property_id && n_properties >= property_id)

            table [property_id] (self, property_id, value, pspec);
          else
            G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (p_self), property_id, pspec);
        }

      static void set_property (GObject* p_self, guint property_id, const GValue* value, GParamSpec* pspec) noexcept
        {
          set_property_ (p_self, property_id, value, pspec, std::make_integer_sequence<size_t, n_properties> { });
        }

      template<size_t... Is>
      static void set_property_ (auto* p_self, guint property_id, const GValue* value, GParamSpec* pspec,
          std::integer_sequence<size_t, Is ...> const&) noexcept
        {

          using fn = void (*) (build_class::instance_struct<Implementor>*, guint, const GValue*, GParamSpec*);

          static constexpr fn table [] =
            {

              [] (auto self, guint property_id, const GValue* value, GParamSpec* pspec)
                { G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (self), property_id, pspec); },

              [] (auto self, guint property_id, const GValue* value, GParamSpec* pspec)
                { accessor<std::tuple_element_t<Is, decltype (List)>::set_property> (self, property_id, value, pspec); }
              ...
            };

          if (auto self = (build_class::instance_struct<Implementor>*) p_self; 0 < property_id && n_properties >= property_id)

            table [property_id] (self, property_id, value, pspec);
          else
            G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (p_self), property_id, pspec);
        }
    };
}