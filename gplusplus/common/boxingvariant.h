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
#include <gplusplus/common/boxingbase.h>

namespace boxing
{

  class variant_dict;
  class variant_iter;
}

namespace boxing::details
{

  static inline GVariant* _g_variant_ref (GVariant* variant) noexcept
    {
      return NULL == variant ? NULL : g_variant_ref (variant);
    }
}

class boxing::variant: public boxing::shared_ptr<GVariant, details::_g_variant_ref, g_variant_unref>
{

  typedef struct _null_variant_tag { } null_variant_tag;

public:

  inline variant (GVariant* variant = nullptr) noexcept: shared_ptr<GVariant, details::_g_variant_ref, g_variant_unref> (variant)
    {
      if (nullptr != variant)
        g_variant_take_ref (variant);
    }

  static constexpr variant null () noexcept
    {
      return variant (null_variant_tag { });
    }

private:

  inline constexpr variant (null_variant_tag) noexcept: shared_ptr<GVariant, details::_g_variant_ref, g_variant_unref> (nullptr)
    { }
};