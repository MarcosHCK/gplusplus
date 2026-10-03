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
#include <cstring>
#include <gplusplus/common/boxingvariant.h>

class boxing::variant_iter
{

  variant _curr;
  GVariantIter _iter;

  static constexpr auto null_variant = variant::null ();
public:

  typedef std::ptrdiff_t difference_type;
  typedef const variant& reference;
  typedef const variant* pointer;
  typedef const variant value_type;

  using iterator_category = std::input_iterator_tag;

  inline constexpr variant_iter () noexcept: _curr (null_variant), _iter ({})
    { }

  variant_iter (GVariant* variant);

  inline variant_iter (const variant_iter& o) noexcept: _curr (o._curr), _iter (o._iter)
    { }

  inline const variant& operator* () const noexcept
    {
      return nullptr != _curr ? _curr : null_variant;
    }

  inline variant_iter& operator= (const variant_iter& o) noexcept
    {
      _curr = o._curr;
      _iter = o._iter;
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

  friend inline bool operator== (const variant_iter& a, const variant_iter& b) noexcept
    {

      if (a._curr == b._curr)

        return true;
      else
        return 0 == std::memcmp (&a._iter, &b._iter, sizeof (GVariantIter));
    }
};