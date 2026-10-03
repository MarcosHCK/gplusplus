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
#include <gplusplus/common/boxingbase.h>

namespace boxing::details
{
  template<typename T>
  static void _g_free (T* object) noexcept { return g_free ((void*) object); }
}

template<typename T>
class boxing::freeable: public unique_ptr<T, details::_g_free<T>>
{
public:

  inline constexpr freeable (T* value = nullptr) noexcept: unique_ptr<T, details::_g_free<T>> (value)
    { }
};