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

  struct layout_probe { };

  static void layout_marker () noexcept { }
}

GPP_IMPLEMENT_FINAL (layout_probe, layout_probe,
{

  GPP_IMPLEMENT_CLASS_VTABLE ({ void (*marker) (); })

  GPP_IMPLEMENT_CLASS_INIT (
    {
      klass->marker = layout_marker;
      G_OBJECT_CLASS (klass)->finalize = class_finalize;
    })

  GPP_IMPLEMENT_DEFAULT_INSTANCE_INIT ()
  GPP_IMPLEMENT_DEFAULT_FINALIZE ({ })
});

using info = __gioplusplus_implementor_info_layout_probe;
using class_struct = gioplusplus::object::details::build_class::class_struct<info>;
using instance_struct = gioplusplus::object::details::build_class::instance_struct<info>;

static_assert (gioplusplus::object::details::has_class_init<info>);
static_assert (sizeof (class_struct) >= sizeof (GObjectClass) + sizeof (void (*) ()));
static_assert (sizeof (instance_struct) >= sizeof (GObject));

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/size_and_prefix", []
    {

      GTypeQuery query = { };
      auto type = layout_probe_get_type ();

      g_type_query (type, &query);

      g_assert_cmpuint (query.class_size, ==, sizeof (class_struct));
      g_assert_cmpuint (query.instance_size, ==, sizeof (instance_struct));
      g_assert_cmpuint (query.class_size, >=, sizeof (GObjectClass));
      g_assert_cmpuint (query.instance_size, >=, sizeof (GObject));

      auto* gobject_class = G_OBJECT_CLASS (g_type_class_ref (type));
      auto* klass = (class_struct*) gobject_class;

      g_assert_cmpuint (G_TYPE_FROM_CLASS (gobject_class), ==, type);
      g_assert_true ((gpointer) &klass->parent_class == (gpointer) gobject_class);
      g_assert_cmpuint ((guintptr) &klass->marker - (guintptr) klass, >=, sizeof (GObjectClass));

      g_type_class_unref (gobject_class);
    });

  g_test_add_action (TESTPATHROOT "/vtable_slot", []
    {

      auto* klass = (class_struct*) g_type_class_ref (layout_probe_get_type ());

      g_assert_true (klass->marker == layout_marker);

      g_type_class_unref ((GTypeClass*) klass);
    });

  g_test_add_action (TESTPATHROOT "/class_init_hook", []
    {

      auto* klass = G_OBJECT_CLASS (g_type_class_ref (layout_probe_get_type ()));

      g_assert_true (klass->finalize == info::class_finalize);

      g_type_class_unref ((GTypeClass*) klass);
    });

  g_test_add_action (TESTPATHROOT "/parent_class", []
    {

      auto type = layout_probe_get_type ();
      auto* klass = g_type_class_ref (type);
      auto* parent_class = g_type_class_peek_parent (klass);

      g_assert_true (info::parent_class == parent_class);
      g_assert_cmpuint (G_TYPE_FROM_CLASS (parent_class), ==, G_TYPE_OBJECT);

      g_type_class_unref (klass);
    });

return g_test_run ();
}
