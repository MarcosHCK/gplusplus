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
#include <gio++/common/boxing.h>

template<typename T>
class boxing::object: public shared_ptr<T, details::_g_object_ref<T>, details::_g_object_unref<T>>
{
public:

  inline constexpr object (T* value = nullptr) noexcept:
      shared_ptr<T, details::_g_object_ref<T>, details::_g_object_unref<T>> (value)
    { }
};