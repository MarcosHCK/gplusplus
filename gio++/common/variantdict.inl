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
#include <gio++/common/variantiter.inl>
#include <glib.h>

class boxing::variant_dict
{

  variant _cont;
  GVariantDict _dict;
public:

  typedef const gchar* key_type;
  typedef const variant_iter const_iterator;
  typedef variant_iter iterator;
  typedef const variant mapped_type;
  typedef const variant& const_reference;
  typedef const variant& reference;
  typedef gsize size_type;
  typedef const variant value_type;

  ~variant_dict () noexcept;
  variant_dict (GVariant* variant);

  inline const variant_iter begin () const noexcept { return const_iterator (_cont); }
  inline variant_iter begin () noexcept { return iterator (_cont); }
  inline const variant_iter cbegin () const noexcept { return const_iterator (_cont); }

  inline bool empty () const noexcept { return 0 == size (); }

  inline const variant_iter end () const noexcept { return const_iterator (); }
  inline variant_iter end () noexcept { return iterator (); }
  inline const variant_iter cend () const noexcept { return const_iterator (); }

  inline size_type size () const noexcept { return g_variant_n_children (_cont); }

  mapped_type at (const key_type& key, const GVariantType* expected_vtype = nullptr) const;

  bool operator== (const variant_dict& o) const noexcept;
};