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
#include <cstring>
#include <gio++/common/boxing.h>

namespace boxing::details
{

  GBytes* _g_bytes_ref (GBytes* bytes) noexcept;
}

class boxing::bytes: public shared_ptr<GBytes, details::_g_bytes_ref, g_bytes_unref>
{
public:

  bytes (GBytes* value = nullptr) noexcept;
  std::pair<const void*, size_t> data () const noexcept;

  template<typename Other>
    requires std::same_as<boxing::bytes, Other>
  inline bool operator== (const Other& o) const noexcept
    {
      auto a = get ();
      auto b = o.get ();
    return (nullptr == a || nullptr == b) ? a == b : g_bytes_equal (a, b);
    }

  template<typename Other>
    requires std::same_as<std::string_view, Other>
  inline bool operator== (const Other& view) const noexcept
    {
      auto [ data, size ] = this->data ();
    return size == view.size () && (0 == size || 0 == std::memcmp (data, view.data (), size));
    }
};

template<> struct std::hash<boxing::bytes>
{
public:

  std::size_t operator() (boxing::bytes bytes) const noexcept;
};