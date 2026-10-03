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
#include <gplusplus/object/objectimplementor.h>
#include <tests/testing.h>
using namespace testing;

namespace
{

  struct alpha_interfaceIface
    {
      GTypeInterface parent_iface;
      gint (*get_value) (GObject* object);
    };

  struct beta_interfaceIface
    {
      GTypeInterface parent_iface;
      gboolean (*is_ready) (GObject* object);
    };

  struct alpha_interface { };
  struct beta_interface { };

  static guint alpha_init_calls = 0;
  static guint alpha_fini_calls = 0;
  static guint beta_init_calls = 0;
  static guint other_alpha_init_calls = 0;
  static guint multi_alpha_init_calls = 0;
  static gint alpha_method_calls = 0;
  static GObject* last_alpha_object = nullptr;

  static GType alpha_interface_get_type () noexcept
    {

      static gsize type_id = 0;

      if (g_once_init_enter (&type_id))
        {
          GTypeInfo info = { };
          info.class_size = sizeof (alpha_interfaceIface);
          auto type = g_type_register_static (G_TYPE_INTERFACE, "GppTestAlphaInterface", &info, G_TYPE_FLAG_NONE);
          g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
          g_once_init_leave (&type_id, type);
        }
      return (GType) type_id;
    }

  static GType beta_interface_get_type () noexcept
    {

      static gsize type_id = 0;

      if (g_once_init_enter (&type_id))
        {
          GTypeInfo info = { };
          info.class_size = sizeof (beta_interfaceIface);
          auto type = g_type_register_static (G_TYPE_INTERFACE, "GppTestBetaInterface", &info, G_TYPE_FLAG_NONE);
          g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
          g_once_init_leave (&type_id, type);
        }
      return (GType) type_id;
    }

  static gint alpha_get_value (GObject* object) noexcept
    {
      ++alpha_method_calls;
      last_alpha_object = object;
      return 42;
    }

  static gint other_alpha_get_value (GObject* object) noexcept
    {
      ++alpha_method_calls;
      last_alpha_object = object;
      return 84;
    }

  static gint multi_alpha_get_value (GObject* object) noexcept
    {
      ++alpha_method_calls;
      last_alpha_object = object;
      return 126;
    }

  static gboolean beta_is_ready (GObject*) noexcept
    {
      return TRUE;
    }

  static void alpha_probe_iface_init (alpha_interfaceIface* iface, gpointer) noexcept
    {
      ++alpha_init_calls;
      iface->get_value = alpha_get_value;
    }

  static void alpha_probe_iface_fini (alpha_interfaceIface*, gpointer) noexcept
    {
      ++alpha_fini_calls;
    }

  static void other_probe_iface_init (alpha_interfaceIface* iface, gpointer) noexcept
    {
      ++other_alpha_init_calls;
      iface->get_value = other_alpha_get_value;
    }

  static void multi_probe_alpha_iface_init (alpha_interfaceIface* iface, gpointer) noexcept
    {
      ++multi_alpha_init_calls;
      iface->get_value = multi_alpha_get_value;
    }

  static void multi_probe_beta_iface_init (beta_interfaceIface* iface, gpointer) noexcept
    {
      ++beta_init_calls;
      iface->is_ready = beta_is_ready;
    }

  struct alpha_probe { };
  struct multi_probe { };
  struct other_probe { };
  struct bare_iface_probe { };
}

GPP_IMPLEMENT_FINAL (alpha_probe, alpha_probe,
{
  GPP_IMPLEMENT_INTERFACE (alpha_interface, alpha_interface, alpha_probe_iface_init, alpha_probe_iface_fini);
});

GPP_IMPLEMENT_FINAL (multi_probe, multi_probe,
{
  GPP_IMPLEMENT_INTERFACE (alpha_interface, alpha_interface, multi_probe_alpha_iface_init);
  GPP_IMPLEMENT_INTERFACE (beta_interface, beta_interface, multi_probe_beta_iface_init);
});

GPP_IMPLEMENT_FINAL (other_probe, other_probe,
{
  GPP_IMPLEMENT_INTERFACE (alpha_interface, alpha_interface, other_probe_iface_init);
});

GPP_IMPLEMENT_FINAL (bare_iface_probe, bare_iface_probe, { });

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/single_interface_and_dispatch", []
    {

      auto type = alpha_probe_get_type ();
      auto interface_type = alpha_interface_get_type ();

      g_assert_true (g_type_is_a (type, interface_type));

      auto* klass = g_type_class_ref (type);
      g_assert_cmpuint (alpha_init_calls, ==, 1);

      auto* iface = (alpha_interfaceIface*) g_type_interface_peek (klass, interface_type);
      g_assert_nonnull (iface);
      g_assert_nonnull (iface->get_value);

      auto* object = (GObject*) g_object_new (type, NULL);
      g_assert_cmpint (iface->get_value (object), ==, 42);
      g_assert_true (last_alpha_object == object);
      g_assert_cmpint (alpha_method_calls, ==, 1);

      g_object_unref (object);
      g_type_class_unref (klass);
    });

  g_test_add_action (TESTPATHROOT "/multiple_interfaces", []
    {

      auto type = multi_probe_get_type ();
      auto alpha_type = alpha_interface_get_type ();
      auto beta_type = beta_interface_get_type ();

      g_assert_true (g_type_is_a (type, alpha_type));
      g_assert_true (g_type_is_a (type, beta_type));

      auto* klass = g_type_class_ref (type);
      g_assert_cmpuint (multi_alpha_init_calls, ==, 1);
      g_assert_cmpuint (beta_init_calls, ==, 1);

      auto* alpha_iface = (alpha_interfaceIface*) g_type_interface_peek (klass, alpha_type);
      auto* beta_iface = (beta_interfaceIface*) g_type_interface_peek (klass, beta_type);
      g_assert_nonnull (alpha_iface);
      g_assert_nonnull (beta_iface);

      auto* object = (GObject*) g_object_new (type, NULL);
      g_assert_cmpint (alpha_iface->get_value (object), ==, 126);
      g_assert_true (beta_iface->is_ready (object));
      g_object_unref (object);
      g_type_class_unref (klass);
    });

  g_test_add_action (TESTPATHROOT "/same_interface_on_multiple_types", []
    {

      auto type = other_probe_get_type ();
      auto interface_type = alpha_interface_get_type ();
      g_assert_true (g_type_is_a (type, interface_type));

      auto* klass = g_type_class_ref (type);
      g_assert_cmpuint (other_alpha_init_calls, ==, 1);

      auto* iface = (alpha_interfaceIface*) g_type_interface_peek (klass, interface_type);
      auto* object = (GObject*) g_object_new (type, NULL);
      g_assert_cmpint (iface->get_value (object), ==, 84);
      g_object_unref (object);
      g_type_class_unref (klass);
    });

  g_test_add_action (TESTPATHROOT "/no_interfaces", []
    {

      auto type = bare_iface_probe_get_type ();
      g_assert_false (g_type_is_a (type, alpha_interface_get_type ()));
      g_assert_false (g_type_is_a (type, beta_interface_get_type ()));
      auto* object = (GObject*) g_object_new (type, NULL);
      g_assert_nonnull (object);
      g_object_unref (object);
    });

  g_test_add_action (TESTPATHROOT "/interface_finalize_trampoline", []
    {

      using implementation = gplusplus::object::details::object_class_interface_<
        alpha_probe, alpha_interfaceIface, alpha_interface_get_type,
        alpha_probe_iface_init, alpha_probe_iface_fini>::type;
      alpha_interfaceIface iface = { };

      g_assert_nonnull (implementation::iface_fini);
      g_assert_cmpuint (alpha_fini_calls, ==, 0);
      implementation::iface_fini (&iface, nullptr);
      g_assert_cmpuint (alpha_fini_calls, ==, 1);
    });

return g_test_run ();
}
