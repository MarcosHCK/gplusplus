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

  static inline GError* _g_error_copy (GError* error) noexcept
    { return NULL == error ? NULL : g_error_copy (error); }

  static inline void _g_error_free (GError* error) noexcept
    { (NULL == error) ? NULL : (error = (g_error_free (error), nullptr)); }
}

class boxing::error: public boxing::shared_ptr<GError, boxing::details::_g_error_copy, boxing::details::_g_error_free>
{
public:

  inline error (GError* error = nullptr) noexcept:
      shared_ptr<GError, boxing::details::_g_error_copy, boxing::details::_g_error_free> (error)
    { }

  inline error (error&& o) noexcept:
      shared_ptr<GError, boxing::details::_g_error_copy, boxing::details::_g_error_free> (std::move (o))
    { }

  inline error (const error& o) noexcept:
      shared_ptr<GError, boxing::details::_g_error_copy, boxing::details::_g_error_free> (details::_g_error_copy (o.get ()))
    { }

  error& operator= (GError* error) noexcept;
  error& operator= (error&& error_) noexcept;
  error& operator= (const error& error_) noexcept;

  static inline boxing::error literal (GQuark domain, int code, const char* message) noexcept (std::is_nothrow_constructible_v<error, GError*>)
    {
      return error (g_error_new_literal (domain, code, message));
    }

  static inline boxing::error printf (GQuark domain, int code, const char* format, ...) noexcept (std::is_nothrow_constructible_v<error, GError*>)
      G_GNUC_PRINTF (3, 4)
    {
      va_list l;
      va_start (l, format);

      auto error_ = error (g_error_new_valist (domain, code, format, l));
    return (va_end (l), error_);
    }
};
