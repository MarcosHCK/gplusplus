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
#include <gplusplus/common/boxingvariant.h>
#include <gplusplus/common/boxingvariantiter.h>

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

  inline ~variant_dict () noexcept
    {
      g_variant_dict_clear (&_dict);
    }

  variant_dict (GVariant* variant);

  inline const variant_iter begin () const noexcept { return const_iterator (_cont); }
  inline variant_iter begin () noexcept { return iterator (_cont); }
  inline const variant_iter cbegin () const noexcept { return const_iterator (_cont); }

  inline bool empty () const noexcept { return 0 == size (); }

  inline const variant_iter end () const noexcept { return const_iterator (); }
  inline variant_iter end () noexcept { return iterator (); }
  inline const variant_iter cend () const noexcept { return const_iterator (); }

  inline size_type size () const noexcept { return g_variant_n_children (_cont); }

  const variant at (const variant_dict::key_type& key, const GVariantType* expected_vtype = nullptr) const
    {

      if (auto v = g_variant_dict_lookup_value (const_cast<GVariantDict*> (&_dict), key, expected_vtype); nullptr != v)

        return v;
      else
        throw std::out_of_range (key);
    }

  friend inline bool operator== (const variant_dict& a, const variant_dict& b) noexcept
    {
      auto a_ = a._cont.get ();
      auto b_ = b._cont.get ();
    return (nullptr == a_ || nullptr == b_) ? a_ == b_ : g_variant_equal (a_, b_);
    }
};