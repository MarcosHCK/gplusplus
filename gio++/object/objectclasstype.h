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
#include <glib-object.h>
#include <type_traits>

namespace gioplusplus::object
{

  namespace details
    {
      struct type_tag_base { };

      template<typename T>
      concept type_tag = std::is_base_of_v<type_tag_base, T>
                        && ! std::is_same_v<type_tag_base, T>;
    }

  namespace type_tag
    {

      struct abstract: details::type_tag_base
        { static inline constexpr GTypeFlags flags = G_TYPE_FLAG_ABSTRACT; };

      struct final: details::type_tag_base
        { static inline constexpr GTypeFlags flags = G_TYPE_FLAG_FINAL; };

      struct normal: details::type_tag_base
        { static inline constexpr GTypeFlags flags = G_TYPE_FLAG_NONE; };
    }
}