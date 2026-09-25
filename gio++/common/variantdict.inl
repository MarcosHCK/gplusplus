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
#include <utility>

class boxing::variant_dict
{

  variant _cont;
  GVariantDict _dict;
public:

  typedef const gchar* key_type;
  typedef const variant_iter const_iterator;
  typedef variant_iter iterator;
  typedef const variant mapped_type;
  typedef const std::pair<key_type, mapped_type>& const_reference;
  typedef std::pair<key_type, mapped_type>& reference;
  typedef gsize size_type;
  typedef std::pair<key_type, mapped_type> value_type;

  inline ~variant_dict () noexcept
    {
      g_variant_dict_clear (&_dict);
    }

  inline variant_dict (GVariant* variant): _cont (g_variant_ref_sink (variant)),
                                           _dict (G_VARIANT_DICT_INIT (variant))
    {

      if (G_UNLIKELY (NULL == variant))
        {
          throw std::runtime_error ("invalid variant (null)");
        }

      const auto etype = G_VARIANT_TYPE ("a{sv}");
      const auto vtype = g_variant_get_type (variant);

      if (G_UNLIKELY (FALSE == g_variant_type_equal (etype, vtype)))
        {

          const auto str_b = g_variant_type_peek_string (vtype);
          const auto str_l = g_variant_type_get_string_length (vtype);

          throw std::runtime_error (std::string ("invalid variant type '") + std::string_view (str_b, str_l) + "'");
        }
    }

  inline const_iterator begin () const noexcept { return const_iterator (_cont); }
  inline iterator begin () noexcept { return iterator (_cont); }
  inline const_iterator cbegin () const noexcept { return const_iterator (_cont); }

  inline const_iterator end () const noexcept { return const_iterator (); }
  inline iterator end () noexcept { return iterator (); }
  inline const_iterator cend () const noexcept { return const_iterator (); }

  inline bool empty () const noexcept { return 0 == size (); }
  inline size_type size () const noexcept { return g_variant_n_children (_cont); }

  inline bool operator== (const variant_dict& o) const noexcept
    {
    return _cont == o._cont || 0 == g_variant_compare (_cont, o._cont);
    }

  inline mapped_type at (const key_type& key, GVariantType* expected_vtype = nullptr)
    {

      if (auto v = g_variant_dict_lookup_value (&_dict, key, expected_vtype); nullptr != v)

        return v;
      else
        throw std::out_of_range (key);
    }
};