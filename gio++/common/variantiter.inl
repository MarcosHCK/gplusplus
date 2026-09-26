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
#include <glib.h>
#include <iterator>

class boxing::variant_iter
{

  variant _curr;
  GVariantIter _iter;
public:

  typedef std::ptrdiff_t difference_type;
  typedef const variant& reference;
  typedef const variant* pointer;
  typedef const variant value_type;

  using iterator_category = std::input_iterator_tag;

  inline variant_iter () noexcept: _curr (nullptr), _iter ({ })
    {
    }

  inline variant_iter (GVariant* variant): variant_iter ()
    {

      if (G_UNLIKELY (NULL == variant))
        {

          throw std::runtime_error ("invalid variant (null)");
        }

      const auto vtype = g_variant_get_type (variant);

      if (G_UNLIKELY (FALSE == g_variant_type_is_container (vtype)))
        {

          const auto str_b = g_variant_type_peek_string (vtype);
          const auto str_l = g_variant_type_get_string_length (vtype);

          throw std::runtime_error (std::string ("invalid variant type '") + std::string_view (str_b, str_l) + "'");
        }

      g_variant_iter_init (&_iter, variant);

      _curr = g_variant_iter_next_value (&_iter);
    }

  inline variant_iter (const variant_iter& o) noexcept: _curr (o._curr)
    {
      std::memcpy (&_iter, &o._iter, sizeof (GVariantIter));
    }

  inline variant operator* () const noexcept
    {
    return nullptr != _curr ? _curr : variant ();
    }

  inline variant_iter& operator= (const variant_iter& o) noexcept
    {
      _curr = o._curr;
      std::memcpy (&_iter, &o._iter, sizeof (GVariantIter));
    return *this;
    }

  inline variant_iter& operator++ () noexcept
    {
      _curr = g_variant_iter_next_value (&_iter);
    return *this;
    }

  inline variant_iter operator++ (int) noexcept
    {
      auto n = *this;
      _curr = g_variant_iter_next_value (&_iter);
    return n;
    }

  inline bool operator== (const variant_iter& o) const noexcept
    {

      if (_curr == o._curr)

        return true;
      else
        return 0 == std::memcmp (&_iter, &o._iter, sizeof (GVariantIter));
    }
};