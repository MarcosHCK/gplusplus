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
#pragma once
#include <algorithm>
#include <cstring>
#include <exception>
#include <glib.h>
#include <utility>
#include <vector>

namespace testing::details
{

  template<typename Functor>
  struct __g_test_add_box
    {

      const char* domain;
      const char* file;
      const char* func;
      Functor functor;
      int line;
    };

  template<typename Functor>
  static void __g_test_add_callback (gconstpointer p_box) noexcept
    {

      auto& box = *(__g_test_add_box<Functor>*) p_box;

      try
        { box.functor (); }
      catch (std::exception& e)
        { g_assertion_message (box.domain, box.file, box.line, box.func, e.what ()); }
      catch (...)
        { g_assertion_message (box.domain, box.file, box.line, box.func, "unknown exception type"); }
    }

  template<typename Functor>
  static void __g_test_add_destroy (gpointer p_box) noexcept
    {

      ((__g_test_add_box<Functor>*) p_box)->~__g_test_add_box ();
      g_slice_free1 (sizeof (__g_test_add_box<Functor>), p_box);
    }

  template<typename T>
  concept __g_test_add_dtor = std::is_destructible_v<T>;

  template<typename T>
  concept __g_test_add_ctor_dtor = std::is_default_constructible_v<T> && std::is_destructible_v<T>;

  template<typename Fn, typename T>
  concept __g_test_add_fixture_ctor = std::is_invocable_r_v<T, Fn>;

  template<typename Fn, typename... Args>
  concept __g_test_add_function = std::is_invocable_r_v<void, Fn, Args ...>;
}

namespace testing
{

  #define g_assert_cmpstrview(view1,op,view2) G_STMT_START { \
 ; \
    std::string_view __view1 = ((view1)); \
    std::string_view __view2 = ((view2)); \
 ; \
    if (false == (__view1 op __view2)) \
      { \
        gchar* __str1 = g_strndup (__view1.data (), __view1.size ()); \
        gchar* __str2 = g_strndup (__view2.data (), __view2.size ()); \
 ; \
        g_assertion_message_cmpstr (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
                                    #view1 " " #op " " #view2, __str1, #op, __str2); \
          g_free (__str1); g_free (__str2); \
      } \
  } G_STMT_END

  template<details::__g_test_add_function Fn,
           typename Functor = std::remove_cvref_t<Fn>>
  static inline void g_test_add_action (const char* domain, const char* file, int line, const char* func, const gchar* testpath, Fn&& action)
      noexcept (std::is_nothrow_constructible_v<Functor, Fn>)
    {
      details::__g_test_add_box<Functor> data = { .domain = domain, .file = file, .func = func, .functor = std::forward<Functor> (action), .line = line };
      details::__g_test_add_box<Functor>* pdata = new (g_slice_alloc0 (sizeof (decltype (data)))) decltype (data) (std::move (data));

    return g_test_add_data_func_full (testpath, pdata, details::__g_test_add_callback<Functor>, details::__g_test_add_destroy<Functor>);
    }

  template<details::__g_test_add_ctor_dtor T,
           details::__g_test_add_function<T&> Fn,
           typename Functor = std::remove_cvref_t<Fn>>
  static inline void g_test_add_action_with_fixture (const char* domain, const char* file, int line, const char* func, const gchar* testpath, Fn&& action)
      noexcept (std::is_nothrow_constructible_v<Functor, Fn> && std::is_nothrow_default_constructible_v<T>)
    {

      g_test_add_action (domain, file, line, func, testpath, [action = std::forward<Fn> (action)]
        { auto fixture = T (); action (fixture); });
    }

  template<details::__g_test_add_dtor T,
           details::__g_test_add_function<T&> Fn,
           details::__g_test_add_fixture_ctor<T> Ac,
           typename Functor = std::remove_cvref_t<Fn>>
  static inline void g_test_add_action_with_fixture (const char* domain, const char* file, int line, const char* func, const gchar* testpath, Fn&& action, Ac&& ctor)
      noexcept (std::is_nothrow_constructible_v<Functor, Fn> && std::is_nothrow_invocable_r_v<T, Ac>)
    {

      g_test_add_action (domain, file, line, func, testpath, [action = std::forward<Fn> (action), ctor = std::forward<Ac> (ctor)]
        { auto fixture = ctor (); action (fixture); });
    }

# define g_test_add_action(testpath,...) (g_test_add_action (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, testpath, __VA_ARGS__))
# define g_test_add_action_with_fixture(testpath,type,...) (g_test_add_action_with_fixture< type > (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, testpath, __VA_ARGS__))

  void g_test_analyze_times (const std::vector<gdouble>& times) noexcept;

  template<std::ranges::view Range>
    requires (std::convertible_to<std::ranges::range_value_t<Range>, gdouble>)
  [[gnu::always_inline]]
  static inline void g_test_analyze_times_unsorted (Range&& range)
    {

      std::vector<gdouble> sorted;

      sorted.assign_range (std::move (range));
      std::sort (sorted.begin (), sorted.end ());

    return g_test_analyze_times (sorted);
    }

  template<unsigned N>
  static inline void g_test_rand_data (guint8 (&ar) [N]) noexcept
    {

      using int_type = decltype (g_test_rand_int ());

      if constexpr (sizeof (int_type) / N > 0) for (gsize i = 0; i < (sizeof (int_type) / N); ++i)
        {
          ((int_type*) ar) [i] = g_test_rand_int ();
        }

      if constexpr (sizeof (int_type) % N > 0)
        {

          auto int_ = g_test_rand_int ();
          std::memcpy (&(((int_type*) ar) [sizeof (int_type) / N]), &int_, sizeof (int_type) % N);
        }
    }

  static inline guint64 g_test_rand_uint64 () noexcept
    {
      union { guint64 u64; guint8 u8 [sizeof (guint64)]; } d;
    return (g_test_rand_data (d.u8), d.u64);
    }

  void g_test_save_times (const std::vector<gdouble>& times) noexcept;
}