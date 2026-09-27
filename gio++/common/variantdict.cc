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
#include <glib.h>
using namespace boxing;

variant_dict::~variant_dict () noexcept
{
  g_variant_dict_clear (&_dict);
}

variant_dict::variant_dict (GVariant* variant): _cont (g_variant_ref_sink (variant)),
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

const variant variant_dict::at (const variant_dict::key_type& key, const GVariantType* expected_vtype) const
{

  if (auto v = g_variant_dict_lookup_value (const_cast<GVariantDict*> (&_dict), key, expected_vtype); nullptr != v)

    return v;
  else
    throw std::out_of_range (key);
}

bool variant_dict::operator== (const variant_dict& o) const noexcept
{

  /* NOTE: g_variant_compare() cannot be used here: it is only defined for
   * non-container types and would emit a GLib critical warning (and return
   * 0) for a{sv} dicts, making unequal dicts compare as equal. */
  auto a = _cont.get ();
  auto b = o._cont.get ();
return (nullptr == a || nullptr == b) ? a == b : g_variant_equal (a, b);
}