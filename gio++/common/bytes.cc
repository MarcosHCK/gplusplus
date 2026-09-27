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
#include <config.h>
#include <functional>
#include <gio++/common/boxing.h>
#include <gio++/common/hashing.h>
using namespace boxing;

namespace boxing::details
{

  GBytes* _g_bytes_ref (GBytes* bytes) noexcept
    {
      return NULL == bytes ? NULL : g_bytes_ref (bytes);
    }
}

bytes::bytes (GBytes* value) noexcept
    : shared_ptr<GBytes, details::_g_bytes_ref, g_bytes_unref> (value)
  { }

std::pair<const void*, size_t> bytes::data () const noexcept
{

  GBytes* bytes_;

  auto size = (gsize) 0;
  auto data = NULL == (bytes_ = *this) ? NULL : g_bytes_get_data (bytes_, &size);
return { data, size };
}

std::size_t std::hash<boxing::bytes>::operator() (boxing::bytes bytes) const noexcept
{

  GBytes* bytes_;

  gsize size = 0;
  const auto data = nullptr == (bytes_ = bytes.get ()) ? nullptr : (guint8*) g_bytes_get_data (bytes_, &size);

return hashing::fnv_1a<std::size_t, std::uint8_t> (std::ranges::subrange (data, data + size));
}