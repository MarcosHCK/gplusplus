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

  variant_iter& operator++ () noexcept;
  variant_iter operator++ (int) noexcept;
  bool operator== (const variant_iter& o) const noexcept;
};