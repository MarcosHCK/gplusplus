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
  struct default_property_probe { int count; };

  static int count_get_calls = 0;
  static int count_set_calls = 0;
  static GParamSpec* last_count_get_pspec = nullptr;
  static GParamSpec* last_count_set_pspec = nullptr;
  static gpointer last_count_get_instance = nullptr;
  static gpointer last_count_set_instance = nullptr;

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

  using construct_read_write_tag = gioplusplus::object::property_tag::or_<
    gioplusplus::object::property_tag::construct,
    read_write_tag>;

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

  static void reset_default_property_state () noexcept
    {
      count_get_calls = 0;
      count_set_calls = 0;
      last_count_get_pspec = nullptr;
      last_count_set_pspec = nullptr;
      last_count_get_instance = nullptr;
      last_count_set_instance = nullptr;
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

GPP_IMPLEMENT_FINAL (default_property_probe, default_property_probe,
{

  GPP_IMPLEMENT_PROPERTY (int, "count", "Count", "A value with custom accessors", 0, 100, 5,
    construct_read_write_tag,
    [] (auto* p_self, GValue* value, GParamSpec* pspec) noexcept
      {
        ++count_get_calls;
        last_count_get_pspec = pspec;
        last_count_get_instance = p_self;
        g_value_set_int (value, p_self->self.count);
      },
    [] (auto* p_self, const GValue* value, GParamSpec* pspec) noexcept
      {
        ++count_set_calls;
        last_count_set_pspec = pspec;
        last_count_set_instance = p_self;
        p_self->self.count = g_value_get_int (value);
      });

  GPP_IMPLEMENT_PROPERTY (int, "unhandled", "Unhandled", "A property without accessors", 0, 100, 0,
    read_write_tag);

  GPP_IMPLEMENT_CLASS_INIT (
    {
      auto* object_class = G_OBJECT_CLASS (klass);
      object_class->get_property = GPP_IMPLEMENT_DEFAULT_GET_PROPERTY
      object_class->set_property = GPP_IMPLEMENT_DEFAULT_SET_PROPERTY
      GPP_IMPLEMENT_INSTALL_PROPERTIES
    })

  GPP_IMPLEMENT_INSTANCE_INIT (
    {
      new (&p_self->self) Type { 0 };
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

  g_test_add_action (TESTPATHROOT "/default_vfuncs/construct_and_roundtrip", []
    {

      reset_default_property_state ();

      auto* object = (GObject*) g_object_new (default_property_probe_get_type (), NULL);
      auto* object_class = G_OBJECT_GET_CLASS (object);
      auto* count_spec = g_object_class_find_property (object_class, "count");
      gint count = 0;

      g_assert_nonnull (count_spec);
      g_assert_cmpint (count_set_calls, ==, 1);
      g_assert_cmpint (last_count_set_pspec == count_spec, ==, TRUE);
      g_assert_true (last_count_set_instance == object);

      g_object_get (object, "count", &count, NULL);
      g_assert_cmpint (count, ==, 5);
      g_assert_cmpint (count_get_calls, ==, 1);
      g_assert_true (last_count_get_pspec == count_spec);
      g_assert_true (last_count_get_instance == object);

      g_object_set (object, "count", 37, NULL);
      g_assert_cmpint (count_set_calls, ==, 2);
      g_assert_true (last_count_set_pspec == count_spec);
      g_assert_true (last_count_set_instance == object);

      g_object_get (object, "count", &count, NULL);
      g_assert_cmpint (count, ==, 37);
      g_assert_cmpint (count_get_calls, ==, 2);
      g_assert_cmpint (last_count_get_pspec == count_spec, ==, TRUE);
      g_assert_true (last_count_get_instance == object);

      g_object_unref (object);
    });

  g_test_add_action (TESTPATHROOT "/default_vfuncs/missing_getter_warns", []
    {

      reset_default_property_state ();

      if (g_test_subprocess ())
        {
          auto* object = (GObject*) g_object_new (default_property_probe_get_type (), NULL);
          gint count = 0;
          g_object_get (object, "unhandled", &count, NULL);
          g_object_unref (object);
          return;
        }

      g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
      g_test_trap_assert_failed ();
      g_test_trap_assert_stderr ("*invalid property id*");
    });

  g_test_add_action (TESTPATHROOT "/default_vfuncs/missing_setter_warns", []
    {

      reset_default_property_state ();

      if (g_test_subprocess ())
        {
          auto* object = (GObject*) g_object_new (default_property_probe_get_type (), NULL);
          g_object_set (object, "unhandled", 9, NULL);
          g_object_unref (object);
          return;
        }

      g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
      g_test_trap_assert_failed ();
      g_test_trap_assert_stderr ("*invalid property id*");
    });

  g_test_add_action (TESTPATHROOT "/default_vfuncs/invalid_getter_id_warns", []
    {

      reset_default_property_state ();

      if (g_test_subprocess ())
        {
          auto* object = (GObject*) g_object_new (default_property_probe_get_type (), NULL);
          auto* object_class = G_OBJECT_GET_CLASS (object);
          auto* count_spec = g_object_class_find_property (object_class, "count");
          GValue value = G_VALUE_INIT;
          g_value_init (&value, G_TYPE_INT);
          object_class->get_property (object, 0, &value, count_spec);
          g_value_unset (&value);
          g_object_unref (object);
          return;
        }

      g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
      g_test_trap_assert_failed ();
      g_test_trap_assert_stderr ("*invalid property id*");
    });

  g_test_add_action (TESTPATHROOT "/default_vfuncs/invalid_setter_id_warns", []
    {

      reset_default_property_state ();

      if (g_test_subprocess ())
        {
          auto* object = (GObject*) g_object_new (default_property_probe_get_type (), NULL);
          auto* object_class = G_OBJECT_GET_CLASS (object);
          auto* count_spec = g_object_class_find_property (object_class, "count");
          GValue value = G_VALUE_INIT;
          g_value_init (&value, G_TYPE_INT);
          object_class->set_property (object, 3, &value, count_spec);
          g_value_unset (&value);
          g_object_unref (object);
          return;
        }

      g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
      g_test_trap_assert_failed ();
      g_test_trap_assert_stderr ("*invalid property id*");
    });

return g_test_run ();
}