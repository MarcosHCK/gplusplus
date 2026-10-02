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

  struct properties_probe { };
  struct bare_probe { };

  struct read_write_tag: gioplusplus::object::details::property_tag_base
    { static inline constexpr GParamFlags flags = G_PARAM_READWRITE; };

  struct writable_tag: gioplusplus::object::details::property_tag_base
    { static inline constexpr GParamFlags flags = G_PARAM_WRITABLE; };

  using enabled_tag = gioplusplus::object::property_tag::or_<
    gioplusplus::object::property_tag::or_<gioplusplus::object::property_tag::construct,
                                           gioplusplus::object::property_tag::explicit_notify>,
    read_write_tag>;

  using label_tag = gioplusplus::object::property_tag::or_<
    gioplusplus::object::property_tag::construct_only,
    writable_tag>;

  static void property_getter (GObject*, guint, GValue*, GParamSpec*) noexcept
    { }

  static void property_setter (GObject*, guint, const GValue*, GParamSpec*) noexcept
    { }

  static GParamSpec* find_own_property (GObjectClass* object_class, GType owner_type, const gchar* name)
    {

      auto* spec = g_object_class_find_property (object_class, name);
      g_assert_nonnull (spec);
      g_assert_cmpuint (spec->owner_type, ==, owner_type);
      return spec;
    }

  static GValue get_default_value (GParamSpec* spec)
    {

      GValue value = G_VALUE_INIT;
      g_value_init (&value, spec->value_type);
      g_param_value_set_default (spec, &value);
      return value;
    }
}

GPP_IMPLEMENT_FINAL (properties_probe, properties_probe,
{

  GPP_IMPLEMENT_PROPERTY (boolean, "enabled", "Enabled", "Whether this object is enabled", TRUE,
    enabled_tag);

  GPP_IMPLEMENT_PROPERTY (int, "priority", "Priority", "Processing priority", -10, 10, 3,
    read_write_tag);

  GPP_IMPLEMENT_PROPERTY (string, "label", "Label", "Object label", "default label",
    label_tag);

  GPP_IMPLEMENT_CLASS_INIT (
    {
      auto* object_class = G_OBJECT_CLASS (klass);
      object_class->get_property = property_getter;
      object_class->set_property = property_setter;
      GPP_IMPLEMENT_INSTALL_PROPERTIES
    })
});

GPP_IMPLEMENT_FINAL (bare_probe, bare_probe,
{

  GPP_IMPLEMENT_CLASS_INIT (
    {
      GPP_IMPLEMENT_INSTALL_PROPERTIES
    })
});

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/declared_count", []
    {

      auto type = properties_probe_get_type ();
      auto* object_class = G_OBJECT_CLASS (g_type_class_ref (type));
      guint listed_count = 0;
      auto** specs = g_object_class_list_properties (object_class, &listed_count);
      guint own_count = 0;

      for (guint i = 0; i < listed_count; ++i)
        own_count += specs [i]->owner_type == type;

      g_assert_cmpuint (own_count, ==, 3);
      g_free (specs);
      g_type_class_unref (object_class);
    });

  g_test_add_action (TESTPATHROOT "/names_types_and_order", []
    {

      auto type = properties_probe_get_type ();
      auto* object_class = G_OBJECT_CLASS (g_type_class_ref (type));

      auto* enabled = find_own_property (object_class, type, "enabled");
      auto* priority = find_own_property (object_class, type, "priority");
      auto* label = find_own_property (object_class, type, "label");

      g_assert_true (G_IS_PARAM_SPEC_BOOLEAN (enabled));
      g_assert_true (G_IS_PARAM_SPEC_INT (priority));
      g_assert_true (G_IS_PARAM_SPEC_STRING (label));

      g_assert_cmpuint (enabled->param_id, ==, 1);
      g_assert_cmpuint (priority->param_id, ==, 2);
      g_assert_cmpuint (label->param_id, ==, 3);

      g_type_class_unref (object_class);
    });

  g_test_add_action (TESTPATHROOT "/defaults", []
    {

      auto type = properties_probe_get_type ();
      auto* object_class = G_OBJECT_CLASS (g_type_class_ref (type));

      auto enabled = get_default_value (find_own_property (object_class, type, "enabled"));
      auto priority = get_default_value (find_own_property (object_class, type, "priority"));
      auto label = get_default_value (find_own_property (object_class, type, "label"));

      g_assert_true (g_value_get_boolean (&enabled));
      g_assert_cmpint (g_value_get_int (&priority), ==, 3);
      g_assert_cmpstr (g_value_get_string (&label), ==, "default label");

      g_value_unset (&enabled);
      g_value_unset (&priority);
      g_value_unset (&label);
      g_type_class_unref (object_class);
    });

  g_test_add_action (TESTPATHROOT "/flags_and_metadata", []
    {

      auto type = properties_probe_get_type ();
      auto* object_class = G_OBJECT_CLASS (g_type_class_ref (type));

      auto* enabled = find_own_property (object_class, type, "enabled");
      auto* priority = find_own_property (object_class, type, "priority");
      auto* label = find_own_property (object_class, type, "label");

      g_assert_true (enabled->flags & G_PARAM_STATIC_STRINGS);
      g_assert_true (enabled->flags & G_PARAM_CONSTRUCT);
      g_assert_true (enabled->flags & G_PARAM_EXPLICIT_NOTIFY);
      g_assert_true (priority->flags & G_PARAM_STATIC_STRINGS);
      g_assert_cmpuint (priority->flags & (G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY), ==, 0);
      g_assert_true (label->flags & G_PARAM_STATIC_STRINGS);
      g_assert_true (label->flags & G_PARAM_CONSTRUCT_ONLY);

      g_assert_cmpstr (g_param_spec_get_nick (enabled), ==, "Enabled");
      g_assert_cmpstr (g_param_spec_get_blurb (enabled), ==, "Whether this object is enabled");
      g_assert_cmpint (G_PARAM_SPEC_INT (priority)->minimum, ==, -10);
      g_assert_cmpint (G_PARAM_SPEC_INT (priority)->maximum, ==, 10);
      g_assert_cmpstr (g_param_spec_get_nick (label), ==, "Label");
      g_assert_cmpstr (g_param_spec_get_blurb (label), ==, "Object label");

      g_type_class_unref (object_class);
    });

  g_test_add_action (TESTPATHROOT "/no_properties", []
    {

      auto type = bare_probe_get_type ();
      auto* object_class = G_OBJECT_CLASS (g_type_class_ref (type));
      guint listed_count = 0;
      auto** specs = g_object_class_list_properties (object_class, &listed_count);
      guint own_count = 0;

      for (guint i = 0; i < listed_count; ++i)
        own_count += specs [i]->owner_type == type;

      g_assert_cmpuint (own_count, ==, 0);
      g_free (specs);
      g_type_class_unref (object_class);
    });

return g_test_run ();
}