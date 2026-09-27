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
#include <gio++/asynclib/error.h>
using namespace gioplusplus::asynclib;

struct GioPlusPlusAsynclibCppErrorPrivate
{
  std::exception_ptr exception_ptr;
};

#define ERROR (gioplusplus_asynclib_cpp_error_quark ())
static GQuark gioplusplus_asynclib_cpp_error_quark (void) G_GNUC_CONST;

using Private = GioPlusPlusAsynclibCppErrorPrivate;

static void gioplusplus_asynclib_cpp_error_private_clear (Private* priv) noexcept
{
  (&priv->exception_ptr)->~exception_ptr ();
}

static void gioplusplus_asynclib_cpp_error_private_copy (const Private* src, Private* dst) noexcept
{
  new (&dst->exception_ptr) std::exception_ptr (src->exception_ptr);
}

static void gioplusplus_asynclib_cpp_error_private_init (Private* dst) noexcept
{
  new (&dst->exception_ptr) std::exception_ptr ();
}

G_DEFINE_EXTENDED_ERROR (GioPlusPlusAsynclibCppError, gioplusplus_asynclib_cpp_error)

[[gnu::always_inline]]
static inline auto _gioplusplus_asynclib_cpp_error_get_ptr (struct _GError* error) noexcept
{

  auto priv = gioplusplus_asynclib_cpp_error_get_private (error);
  auto eptr = std::exception_ptr (priv->exception_ptr);
return eptr;
}

[[gnu::always_inline]]
static inline GError* _gioplusplus_asynclib_cpp_error_new (std::exception_ptr exception_ptr) noexcept
{

  auto error = g_error_new_literal (ERROR, 0, "c++ exception thrown");
  auto priv = gioplusplus_asynclib_cpp_error_get_private (error);
return (priv->exception_ptr = exception_ptr, error);
}

std::exception_ptr gioplusplus::asynclib::details::error::from_glib_error (GError* error_) noexcept
{

  g_return_val_if_fail (NULL != error_, std::exception_ptr { });

  if (std::exception_ptr ptr; ERROR != error_->domain)

    return std::make_exception_ptr (error (error_));
  else
    return (ptr = _gioplusplus_asynclib_cpp_error_get_ptr (error_), g_error_free (error_), ptr);
}

GError* gioplusplus::asynclib::details::error::to_glib_error (std::exception_ptr ptr) noexcept
{

  g_return_val_if_fail (nullptr != ptr, NULL);

  try
    { std::rethrow_exception (ptr); }
  catch (boxing::error& error)
    { return error.release (); }
  catch (...)
    { return _gioplusplus_asynclib_cpp_error_new (std::current_exception ()); }
}

const char* gioplusplus::asynclib::details::error::what () const _GLIBCXX_TXN_SAFE_DYN noexcept
{
  return nullptr == get () ? "" : get ()->message;
}