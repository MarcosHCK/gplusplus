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
#include <gplusplus/common/constexprstring.h>
#include <gplusplus/object/objectclassconcepts.h>
#include <glib-object.h>

namespace gplusplus::object
{

  namespace details
    {

      struct property_tag_base { };

      template<typename T>
      concept property_tag = std::is_base_of_v<property_tag_base, T>
                            && ! std::is_same_v<property_tag_base, T>;
    }

  namespace property_tag
    {

      struct construct: details::property_tag_base
        { static inline constexpr GParamFlags flags = G_PARAM_CONSTRUCT; };

      struct construct_only: details::property_tag_base
        { static inline constexpr GParamFlags flags = G_PARAM_CONSTRUCT_ONLY; };

      struct explicit_notify: details::property_tag_base
        { static inline constexpr GParamFlags flags = G_PARAM_EXPLICIT_NOTIFY; };

      struct lax_validation: details::property_tag_base
        { static inline constexpr GParamFlags flags = G_PARAM_LAX_VALIDATION; };

      struct normal: details::property_tag_base
        { static inline constexpr auto flags = (GParamFlags) 0; };

      namespace details
        {

          template<object::details::property_tag Tag1,
                   object::details::property_tag Tag2>
          struct or_
            {
              struct type: object::details::property_tag_base
                { static inline constexpr auto flags = (GParamFlags) (Tag1::flags | Tag2::flags); };
            };

          template<object::details::property_tag Tag1,
                   GParamFlags Flags>
          struct or2_
            {
              struct type: object::details::property_tag_base
                { static inline constexpr auto flags = (GParamFlags) (Tag1::flags | Flags); };
            };
        }

      template<object::details::property_tag Tag1,
               object::details::property_tag Tag2>
      using or_ = details::or_<Tag1, Tag2>::type;
    }

  template<typename InstanceType,
           details::invocable_r<GParamSpec*, GParamFlags> auto CreateSpec,
           details::property_tag PropertyTag = property_tag::normal,
           details::invocable_r_or_null<void, InstanceType*, GValue*, GParamSpec*> auto GetProperty = nullptr,
           details::invocable_r_or_null<void, InstanceType*, const GValue*, GParamSpec*> auto SetProperty = nullptr>
  struct object_class_property
    {

      static inline constexpr auto create_spec = CreateSpec;
      static inline constexpr auto get_property = GetProperty;
      static inline constexpr auto set_property = SetProperty;

      using Tag = std::conditional_t<!std::same_as<std::nullptr_t, decltype (GetProperty)>
                                  && !std::same_as<std::nullptr_t, decltype (SetProperty)>,
                    typename property_tag::details::or2_<PropertyTag, (GParamFlags) (G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)>::type,
                  std::conditional_t<!std::same_as<std::nullptr_t, decltype (GetProperty)>,
                    typename property_tag::details::or2_<PropertyTag, (GParamFlags) (G_PARAM_STATIC_STRINGS | G_PARAM_READABLE)>::type,
                  std::conditional_t<!std::same_as<std::nullptr_t, decltype (SetProperty)>,
                    typename property_tag::details::or2_<PropertyTag, (GParamFlags) (G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE)>::type,
                    typename property_tag::details::or2_<PropertyTag, (GParamFlags) (G_PARAM_STATIC_STRINGS)>::type>>>;
    };
}