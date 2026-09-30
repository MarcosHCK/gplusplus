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
#include <array>
#include <config.h>
#include <gio++/object/objectimplementor.h>
#include <tests/testing.h>
using namespace testing;

namespace
{

  enum class event
    {
      base_instance_init,
      derived_instance_init,
      base_constructed,
      derived_constructed,
      derived_dispose,
      base_dispose,
      derived_finalize_body,
      derived_destructor,
      base_finalize_body,
      base_destructor,
    };

  struct event_log
    {
      std::array<event, 16> events = { };
      std::size_t count = 0;
      int base_ctor_calls = 0;
      int base_dtor_calls = 0;
      int derived_ctor_calls = 0;
      int derived_dtor_calls = 0;

      void push (event value) noexcept
        {
          g_assert_cmpuint (count, <, events.size ());
          events [count++] = value;
        }
    };

  static event_log log;

  struct base_value
    {
      base_value () { ++log.base_ctor_calls; }
      ~base_value () { ++log.base_dtor_calls; log.push (event::base_destructor); }
    };

  struct derived_value
    {
      derived_value () { ++log.derived_ctor_calls; }
      ~derived_value () { ++log.derived_dtor_calls; log.push (event::derived_destructor); }
    };
}

GPP_IMPLEMENT (base_value, base_probe,
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
      log.push (event::base_instance_init);
      new (&p_self->self) Type ();
    })

  GPP_IMPLEMENT_CONSTRUCTED (
    {
      log.push (event::base_constructed);
    })

  GPP_IMPLEMENT_DISPOSE (
    {
      log.push (event::base_dispose);
    })

  GPP_IMPLEMENT_DEFAULT_FINALIZE (
    {
      log.push (event::base_finalize_body);
    })
});

using base_info = __gioplusplus_implementor_info_base_value;
using BaseProbe = gioplusplus::object::details::build_class::class_struct<base_info>;
using BaseProbeClass = gioplusplus::object::details::build_class::instance_struct<base_info>;

GType base_probe_get_type () noexcept G_GNUC_CONST;

GPP_IMPLEMENT_FINAL (derived_value, derived_probe,
{

  GPP_IMPLEMENT_ANCESTOR (BaseProbe, base_probe)

  GPP_IMPLEMENT_CLASS_INIT (
    {
      auto* object_class = G_OBJECT_CLASS (klass);
      object_class->constructed = class_constructed;
      object_class->dispose = class_dispose;
      object_class->finalize = class_finalize;
    })

  GPP_IMPLEMENT_INSTANCE_INIT (
    {
      log.push (event::derived_instance_init);
      new (&p_self->self) Type ();
    })

  GPP_IMPLEMENT_CONSTRUCTED (
    {
      log.push (event::derived_constructed);
    })

  GPP_IMPLEMENT_DISPOSE (
    {
      log.push (event::derived_dispose);
    })

  GPP_IMPLEMENT_DEFAULT_FINALIZE (
    {
      log.push (event::derived_finalize_body);
    })
});

using derived_info = __gioplusplus_implementor_info_derived_value;
using base_class_struct = gioplusplus::object::details::build_class::class_struct<base_info>;
using derived_class_struct = gioplusplus::object::details::build_class::class_struct<derived_info>;

static_assert (sizeof (derived_class_struct) >= sizeof (base_class_struct));

static void reset_log () noexcept
  { log = { }; }

static void assert_event_sequence (const auto& expected) noexcept
  {
    g_assert_cmpuint (log.count, ==, expected.size ());

    for (std::size_t i = 0; i < expected.size (); ++i)
      g_assert_cmpint ((int) log.events [i], ==, (int) expected [i]);
  }

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/ancestor", []
    {

      auto base_type = base_probe_get_type ();
      auto derived_type = derived_probe_get_type ();

      g_assert_cmpuint (g_type_parent (derived_type), ==, base_type);
      g_assert_true (g_type_is_a (derived_type, base_type));
      g_assert_true (g_type_is_a (derived_type, G_TYPE_OBJECT));

      auto* derived_class = g_type_class_ref (derived_type);
      auto* base_class = g_type_class_peek (base_type);

      g_assert_true (derived_info::parent_class == base_class);
      g_assert_true (g_type_class_peek_parent (derived_class) == base_class);

      g_type_class_unref (derived_class);
    });

  g_test_add_action (TESTPATHROOT "/instance_init_chain", []
    {

      reset_log ();
      auto* object = (GObject*) g_object_new (derived_probe_get_type (), NULL);

      g_assert_nonnull (object);
      g_assert_cmpint (log.base_ctor_calls, ==, 1);
      g_assert_cmpint (log.derived_ctor_calls, ==, 1);
      g_assert_cmpint (log.base_dtor_calls, ==, 0);
      g_assert_cmpint (log.derived_dtor_calls, ==, 0);

      constexpr std::array expected =
        { event::base_instance_init, event::derived_instance_init,
          event::base_constructed, event::derived_constructed };
      assert_event_sequence (expected);

      g_object_unref (object);
    });

  g_test_add_action (TESTPATHROOT "/constructed_chain", []
    {

      reset_log ();
      auto* object = (GObject*) g_object_new (derived_probe_get_type (), NULL);

      constexpr std::array expected =
        { event::base_instance_init, event::derived_instance_init,
          event::base_constructed, event::derived_constructed };
      assert_event_sequence (expected);

      g_object_unref (object);
    });

  g_test_add_action (TESTPATHROOT "/dispose_finalize_chain", []
    {

      reset_log ();
      auto* object = (GObject*) g_object_new (derived_probe_get_type (), NULL);
      g_object_unref (object);

      constexpr std::array expected =
        { event::base_instance_init, event::derived_instance_init,
          event::base_constructed, event::derived_constructed,
          event::derived_dispose, event::base_dispose,
          event::derived_finalize_body, event::derived_destructor,
          event::base_finalize_body, event::base_destructor };
      assert_event_sequence (expected);

      g_assert_cmpint (log.base_ctor_calls, ==, 1);
      g_assert_cmpint (log.base_dtor_calls, ==, 1);
      g_assert_cmpint (log.derived_ctor_calls, ==, 1);
      g_assert_cmpint (log.derived_dtor_calls, ==, 1);
    });

return g_test_run ();
}
