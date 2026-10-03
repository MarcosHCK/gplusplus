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
#include <gplusplus/common/boxingvariantiter.h>
using namespace boxing;

variant_iter::variant_iter (GVariant* variant): _curr (nullptr), _iter ({ })
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