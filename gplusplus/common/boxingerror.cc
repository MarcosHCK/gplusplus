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
#include <config.h>
#include <gplusplus/common/boxingerror.h>
using namespace boxing;

static constexpr const gchar* override_warn =
  "GError set over the top of a previous GError or uninitialized memory.\n" \
  "This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n" \
  "The overwriting error message was: %s";

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