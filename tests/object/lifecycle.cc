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

  struct lifecycle_state
    {
      int order = 0;
      int ctor_calls = 0;
      int ctor_order = 0;
      int dtor_calls = 0;
      int dtor_order = 0;
      int constructed_calls = 0;
      int constructed_order = 0;
      int dispose_calls = 0;
      int dispose_order = 0;
      int finalize_body_calls = 0;
      int finalize_body_order = 0;
      int constructed_value = 0;
      bool class_pointer_matches = false;
    };

  static lifecycle_state state;

  struct lifecycle_value
    {
      int value;

      explicit lifecycle_value (int value_): value (value_)
        {
          ++state.ctor_calls;
          state.ctor_order = ++state.order;
        }

      ~lifecycle_value ()
        {
          ++state.dtor_calls;
          state.dtor_order = ++state.order;
        }
    };
}

GType lifecycle_probe_get_type () G_GNUC_CONST;

GPP_IMPLEMENT_FINAL (lifecycle_value, lifecycle_probe,
{

  GPP_IMPLEMENT_CLASS_INIT (
    {
      auto* object_class = G_OBJECT_CLASS (klass);
      object_class->constructed = class_constructed;
      object_class->dispose = class_dispose;
      object_class->finalize = class_finalize;
    })

  GPP_IMPLEMENT_INSTANCE_INIT (
    {
      state.class_pointer_matches = (gpointer) klass == g_type_class_peek (lifecycle_probe_get_type ());
      new (&p_self->self) Type (17);
    })

  GPP_IMPLEMENT_CONSTRUCTED (
    {
      ++state.constructed_calls;
      state.constructed_order = ++state.order;
      state.constructed_value = self.value;
    })

  GPP_IMPLEMENT_DISPOSE (
    {
      ++state.dispose_calls;
      state.dispose_order = ++state.order;
    })

  GPP_IMPLEMENT_DEFAULT_FINALIZE (
    {
      ++state.finalize_body_calls;
      state.finalize_body_order = ++state.order;
    })
});

using info = __gplusplus_implementor_info_lifecycle_value;
using instance_struct = gplusplus::object::details::build_class::instance_struct<info>;

static void reset_state () noexcept
  { state = { }; }

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/instance_init_constructed_finalize", []
    {

      reset_state ();
      auto* object = (GObject*) g_object_new (lifecycle_probe_get_type (), NULL);
      auto* instance = (instance_struct*) object;

      g_assert_nonnull (object);
      g_assert_cmpint (state.ctor_calls, ==, 1);
      g_assert_true (state.class_pointer_matches);
      g_assert_cmpint (instance->self.value, ==, 17);
      g_assert_cmpint (state.constructed_calls, ==, 1);
      g_assert_cmpint (state.constructed_value, ==, 17);
      g_assert_cmpint (state.ctor_order, <, state.constructed_order);

      g_object_unref (object);

      g_assert_cmpint (state.dispose_calls, ==, 1);
      g_assert_cmpint (state.finalize_body_calls, ==, 1);
      g_assert_cmpint (state.dtor_calls, ==, 1);
      g_assert_cmpint (state.dispose_order, <, state.finalize_body_order);
      g_assert_cmpint (state.finalize_body_order, <, state.dtor_order);
    });

  g_test_add_action (TESTPATHROOT "/last_unref", []
    {

      reset_state ();
      auto* object = (GObject*) g_object_new (lifecycle_probe_get_type (), NULL);

      object = g_object_ref (object);
      g_object_unref (object);

      g_assert_cmpint (state.dispose_calls, ==, 0);
      g_assert_cmpint (state.finalize_body_calls, ==, 0);
      g_assert_cmpint (state.dtor_calls, ==, 0);

      g_object_unref (object);

      g_assert_cmpint (state.dispose_calls, ==, 1);
      g_assert_cmpint (state.finalize_body_calls, ==, 1);
      g_assert_cmpint (state.dtor_calls, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/multiple_instances", []
    {

      reset_state ();
      auto* first = (GObject*) g_object_new (lifecycle_probe_get_type (), NULL);
      auto* second = (GObject*) g_object_new (lifecycle_probe_get_type (), NULL);

      g_assert_cmpint (state.ctor_calls, ==, 2);
      g_assert_cmpint (state.constructed_calls, ==, 2);

      g_object_unref (first);
      g_object_unref (second);

      g_assert_cmpint (state.dispose_calls, ==, 2);
      g_assert_cmpint (state.finalize_body_calls, ==, 2);
      g_assert_cmpint (state.dtor_calls, ==, 2);
    });

  g_test_add_action (TESTPATHROOT "/repeat_dispose", []
    {

      reset_state ();
      auto* object = (GObject*) g_object_new (lifecycle_probe_get_type (), NULL);

      g_object_run_dispose (object);
      g_object_run_dispose (object);

      g_assert_cmpint (state.dispose_calls, ==, 2);
      g_assert_cmpint (state.finalize_body_calls, ==, 0);
      g_assert_cmpint (state.dtor_calls, ==, 0);

      g_object_unref (object);

      g_assert_cmpint (state.finalize_body_calls, ==, 1);
      g_assert_cmpint (state.dtor_calls, ==, 1);
    });

return g_test_run ();
}
