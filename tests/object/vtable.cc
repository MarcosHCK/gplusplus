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
#include <gio++/object/objectimplementor.h>
#include <tests/testing.h>
using namespace testing;

namespace
{

  struct base_value { };
  struct derived_value { };

  static int inherited_value = -1;

  static int base_get_value () noexcept { return 1; }
  static int derived_get_value () noexcept { return 2; }
}

GType base_probe_get_type () noexcept G_GNUC_CONST;

GPP_IMPLEMENT (base_value, base_probe,
{

  GPP_IMPLEMENT_CLASS_VTABLE ({ int (*get_value) (); })

  GPP_IMPLEMENT_CLASS_INIT (
    {
      klass->get_value = base_get_value;
      G_OBJECT_CLASS (klass)->finalize = class_finalize;
    })

  GPP_IMPLEMENT_DEFAULT_INSTANCE_INIT ()
  GPP_IMPLEMENT_DEFAULT_FINALIZE ({ })
});

using base_info = __gioplusplus_implementor_info_base_value;
using BaseProbe = gioplusplus::object::details::build_class::class_struct<base_info>;
using BaseProbeClass = gioplusplus::object::details::build_class::instance_struct<base_info>;
using base_class_struct = gioplusplus::object::details::build_class::class_struct<base_info>;

GPP_IMPLEMENT_FINAL (derived_value, derived_probe,
{

  /* GPP_IMPLEMENT_ANCESTOR's current argument order is ClassType, InstanceType. */
  GPP_IMPLEMENT_ANCESTOR (BaseProbe, base_probe)

  GPP_IMPLEMENT_CLASS_INIT (
    {
      /* GObject copies the base class before calling this initializer. */
      inherited_value = klass->parent_class.get_value ();
      G_OBJECT_CLASS (klass)->finalize = class_finalize;
      klass->parent_class.get_value = derived_get_value;
    })

  GPP_IMPLEMENT_DEFAULT_INSTANCE_INIT ()
  GPP_IMPLEMENT_DEFAULT_FINALIZE ({ })
});

using derived_info = __gioplusplus_implementor_info_derived_value;
using derived_class_struct = gioplusplus::object::details::build_class::class_struct<derived_info>;

static_assert (sizeof (derived_class_struct) >= sizeof (base_class_struct));

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/inherited_copy_and_override", []
    {

      auto base_type = base_probe_get_type ();
      auto derived_type = derived_probe_get_type ();
      auto* base_class = (base_class_struct*) g_type_class_ref (base_type);
      auto* derived_class = (derived_class_struct*) g_type_class_ref (derived_type);
      auto* derived_as_base = (base_class_struct*) derived_class;

      g_assert_cmpint (inherited_value, ==, 1);
      g_assert_cmpint (base_class->get_value (), ==, 1);
      g_assert_cmpint (derived_as_base->get_value (), ==, 2);
      g_assert_true (base_class->get_value != derived_as_base->get_value);

      g_type_class_unref ((GTypeClass*) derived_class);
      g_type_class_unref ((GTypeClass*) base_class);
    });

  g_test_add_action (TESTPATHROOT "/instance_dispatch", []
    {

      auto* base_object = (GObject*) g_object_new (base_probe_get_type (), NULL);
      auto* derived_object = (GObject*) g_object_new (derived_probe_get_type (), NULL);
      auto* base_class = (base_class_struct*) G_OBJECT_GET_CLASS (base_object);
      auto* derived_as_base = (base_class_struct*) G_OBJECT_GET_CLASS (derived_object);

      g_assert_cmpint (base_class->get_value (), ==, 1);
      g_assert_cmpint (derived_as_base->get_value (), ==, 2);

      g_object_unref (base_object);
      g_object_unref (derived_object);
    });

  g_test_add_action (TESTPATHROOT "/override_persists", []
    {

      auto type = derived_probe_get_type ();
      auto* first = (derived_class_struct*) g_type_class_ref (type);

      g_assert_cmpint (((base_class_struct*) first)->get_value (), ==, 2);
      g_type_class_unref ((GTypeClass*) first);

      auto* second = (derived_class_struct*) g_type_class_ref (type);

      g_assert_cmpint (((base_class_struct*) second)->get_value (), ==, 2);

      g_type_class_unref ((GTypeClass*) second);
    });

return g_test_run ();
}
