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
#include <gio++/common/boxing.h>
using namespace boxing;

namespace boxing::details
{

  GError* _g_error_copy (GError* error) noexcept
    {
      return NULL == error ? NULL : g_error_copy (error);
    }

  void _g_error_free (GError* error) noexcept
    {
      if (G_LIKELY (NULL != error))
        g_error_free (error);
    }
}

static constexpr const gchar* override_warn =
  "GError set over the top of a previous GError or uninitialized memory.\n" \
  "This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n" \
  "The overwriting error message was: %s";

error::error (GError* error) noexcept: shared_ptr<GError, details::_g_error_copy, details::_g_error_free> (error)
{ }

error::error (error&& o) noexcept: shared_ptr<GError, details::_g_error_copy, details::_g_error_free> (std::move (o))
{ }

error::error (const error& o) noexcept: shared_ptr<GError, details::_g_error_copy, details::_g_error_free> (copy (o.get ()))
{ }

error& error::operator= (GError* error) noexcept
{

  if (G_UNLIKELY (NULL != (*this)))
    g_warning (override_warn, (*this)->message);
return (shared_ptr<GError, details::_g_error_copy, details::_g_error_free>::operator= (error), *this);
}

error& error::operator= (error&& error_) noexcept
{

  if (G_UNLIKELY (this != &error_))
    {
      if (G_UNLIKELY (NULL != (*this)))
        g_warning (override_warn, (*this)->message);

      shared_ptr<GError, details::_g_error_copy, details::_g_error_free>::operator= (std::move (error_));
    }
return *this;
}

error& error::operator= (const error& error_) noexcept
{

  if (G_UNLIKELY (this != &error_))
    {
      if (G_UNLIKELY (NULL != (*this)))
        g_warning (override_warn, (*this)->message);

      shared_ptr<GError, details::_g_error_copy, details::_g_error_free>::operator= (static_cast<const shared_ptr&> (error_));
    }
return *this;
}

error error::literal (GQuark domain, int code, const char* message) noexcept (std::is_nothrow_constructible_v<boxing::error, GError*>)
{
  return error (g_error_new_literal (domain, code, message));
}

error error::printf (GQuark domain, int code, const char* format, ...) noexcept (std::is_nothrow_constructible_v<boxing::error, GError*>)
{

  va_list l;
  va_start (l, format);

  auto error_ = error (g_error_new_valist (domain, code, format, l));
return (va_end (l), error_);
}