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
#include <gio++/object/objectclass.h>
#include <gio++/object/objectimplementor.h>
using namespace gioplusplus;
using namespace gioplusplus::object;

GPP_IMPLEMENT (int, _int,
{

  GPP_IMPLEMENT_CLASS_VTABLE ({ int (*some) (); })

  GPP_IMPLEMENT_DEFAULT_FINALIZE (
    {
    })

  GPP_IMPLEMENT_CLASS_INIT (
    {
      klass->some = nullptr;
      G_OBJECT_CLASS (klass)->finalize = class_finalize;
    })

  GPP_IMPLEMENT_DEFAULT_INSTANCE_INIT ()
});

static_assert (details::has_class_init<__gioplusplus_implementor_info_int>);