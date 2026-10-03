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
#include <gplusplus/common/boxing.h>
#include <set>
#include <string>
#include <tests/testing.h>
#include <unordered_map>
using namespace testing;

class variant_asv: public boxing::variant
{

  std::unordered_map<std::string, std::string> _samples;

  static inline auto build ()
    {

      auto builder = GVariantBuilder G_VARIANT_BUILDER_INIT (G_VARIANT_TYPE_VARDICT);
      auto samples = std::unordered_map<std::string, std::string> ();

      for (unsigned i = 0; i < (unsigned) g_test_rand_int_range (10, 100); ++i)
        {
          build_entry (samples, builder, g_test_rand_string (), g_test_rand_string ());
        }

    return std::make_pair (samples, g_variant_take_ref (g_variant_builder_end (&builder)));
    }

  static inline void build_entry (std::unordered_map<std::string, std::string>& samples, GVariantBuilder& builder, std::string key, std::string value)
    {

      std::pair<const std::string, std::string>* pair = nullptr;

      if (auto [ it, done ] = samples.emplace (key, value); (pair = &*it, !done))
        return;

      g_variant_builder_open (&builder, G_VARIANT_TYPE ("{sv}"));
      g_variant_builder_add_value (&builder, g_variant_new_string (pair->first.c_str ()));
      g_variant_builder_add_value (&builder, g_variant_new_variant (g_variant_new_string (pair->second.c_str ())));
      g_variant_builder_close (&builder);
    }

public:

  inline variant_asv (): boxing::variant (nullptr)
    {

      auto [ samples, variant ] = build ();

      static_cast<boxing::variant&> (*this) = variant;
      _samples = std::move (samples);
    }

  inline constexpr const auto& get_samples () const noexcept { return _samples; }
};

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action_with_fixture (TESTPATHROOT "/new", variant_asv, [](const variant_asv& asv)
    {
      boxing::variant_dict dict = boxing::variant_dict (asv);
    });

  g_test_add_action_with_fixture (TESTPATHROOT "/lookup", variant_asv, [](const variant_asv& asv)
    {

      auto dict = boxing::variant_dict (asv);
      auto missing = g_test_rand_string ();
      auto& samples = asv.get_samples ();

      for (decltype (samples.find (std::string ())) it; (it = samples.find (missing)) != samples.end ();)
        missing = g_test_rand_string ();

      g_assert_not_throws (({ g_assert_true (nullptr != dict.at (asv.get_samples ().begin ()->first.c_str ())); }));
      g_assert_throws (std::out_of_range, ({ g_assert_true (nullptr == dict.at (missing.c_str ())); }));
    });

  g_test_add_action_with_fixture (TESTPATHROOT "/size", variant_asv, [](const variant_asv& asv)
    {

      auto dict = boxing::variant_dict (asv);

      g_assert_cmpuint (dict.size (), ==, asv.get_samples ().size ());
      g_assert_false (dict.empty ());
    });

  g_test_add_action_with_fixture (TESTPATHROOT "/iteration", variant_asv, [](const variant_asv& asv)
    {

      auto dict = boxing::variant_dict (asv);
      auto& samples = asv.get_samples ();

      gsize count = 0;
      auto found = std::set<std::string> ();

      for (auto entry: dict)
        {

          g_assert_true (g_variant_is_of_type (entry, G_VARIANT_TYPE ("{sv}")));

          auto key_value = boxing::variant (g_variant_get_child_value (entry, 0));
          auto key = boxing::freeable<gchar> (g_variant_dup_string (key_value, NULL));
          found.emplace ((const gchar*) key);
          ++count;
        }

      g_assert_cmpuint (count, ==, samples.size ());
      for (auto& [ key, value ]: samples)
        g_assert_true (found.count (key) == 1);
    });

  g_test_add_action (TESTPATHROOT "/equality", []
    {

      GVariantBuilder a_builder = G_VARIANT_BUILDER_INIT (G_VARIANT_TYPE ("a{sv}"));
      g_variant_builder_add (&a_builder, "{sv}", "key", g_variant_new_string ("value"));
      auto a = boxing::variant (g_variant_builder_end (&a_builder));

      GVariantBuilder b_builder = G_VARIANT_BUILDER_INIT (G_VARIANT_TYPE ("a{sv}"));
      g_variant_builder_add (&b_builder, "{sv}", "key", g_variant_new_string ("value"));
      auto b = boxing::variant (g_variant_builder_end (&b_builder));

      /* distinct GVariant objects, equal content */
      g_assert_true (a != b);

      auto dict_a = boxing::variant_dict (a);
      auto dict_b = boxing::variant_dict (b);
      g_assert_true (dict_a == dict_b);
      g_assert_true (dict_a == dict_a);

      /* Regression: operator== used g_variant_compare(), which is undefined for
       * container types (a{sv}): it tripped a GLib assertion, returned 0 and
       * made two DIFFERENT dicts compare equal. */
      GVariantBuilder c_builder = G_VARIANT_BUILDER_INIT (G_VARIANT_TYPE ("a{sv}"));
      g_variant_builder_add (&c_builder, "{sv}", "other", g_variant_new_int32 (5));
      auto c = boxing::variant (g_variant_builder_end (&c_builder));

      auto dict_c = boxing::variant_dict (c);
      g_assert_false (dict_a == dict_c);
      g_assert_false (dict_c == dict_b);
    });

  g_test_add_action_with_fixture (TESTPATHROOT "/at_type_mismatch", variant_asv, [](const variant_asv& asv)
    {

      auto dict = boxing::variant_dict (asv);
      auto& samples = asv.get_samples ();
      auto key = samples.begin ()->first;

      g_assert_not_throws (({ g_assert_true (nullptr != dict.at (key.c_str ())); }));

      /* asking for a value type that does not match must report out_of_range */
      g_assert_throws (std::out_of_range, ({ dict.at (key.c_str (), G_VARIANT_TYPE ("i")); }));
    });

  g_test_add_action_with_fixture (TESTPATHROOT "/at_const", variant_asv, [](const variant_asv& asv)
    {

      auto dict = boxing::variant_dict (asv);
      const boxing::variant_dict& const_dict = dict;
      auto& samples = asv.get_samples ();
      auto key = samples.begin ()->first;

      g_assert_not_throws (({ g_assert_true (nullptr != const_dict.at (key.c_str ())); }));
    });

return g_test_run ();
}