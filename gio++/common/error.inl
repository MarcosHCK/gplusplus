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

namespace boxing::details
{

  GError* _g_error_copy (GError* error) noexcept;
  void _g_error_free (GError* error) noexcept;
}

class boxing::error: public boxing::shared_ptr<GError, boxing::details::_g_error_copy, boxing::details::_g_error_free>
{
public:

  error (GError* error = nullptr) noexcept;
  error (error&& o) noexcept;
  error (const error& o) noexcept;

  error& operator= (GError* error) noexcept;
  error& operator= (error&& error_) noexcept;
  error& operator= (const error& error_) noexcept;

  static boxing::error literal (GQuark domain, int code, const char* message)
      noexcept (std::is_nothrow_constructible_v<error, GError*>);

  static boxing::error printf (GQuark domain, int code, const char* format, ...)
      noexcept (std::is_nothrow_constructible_v<error, GError*>) G_GNUC_PRINTF (3, 4);
};
