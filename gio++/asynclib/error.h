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
#include <exception>
#include <gio++/common/boxing.h>

namespace gioplusplus::asynclib::details
{

  class error: public std::exception, public boxing::error
    {
    public:

      inline error () noexcept (std::is_nothrow_constructible_v<std::exception>):
          boxing::error (nullptr)
        { }

      inline error (GError* g_error) noexcept (std::is_nothrow_constructible_v<std::exception>):
          boxing::error (g_error)
        { }

      static std::exception_ptr from_glib_error (GError* error) noexcept;

      static GError* to_glib_error (std::exception_ptr ptr) noexcept;

      virtual const char* what () const _GLIBCXX_TXN_SAFE_DYN noexcept override
        { return nullptr == get () ? "" : get ()->message; }
    };
}