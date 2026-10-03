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
#include <cmath>
#include <gio/gio.h>
#include <numeric>
#include <string_view>
#include <tests/testing.h>

template<unsigned N, typename T>
[[gnu::always_inline]]
static inline T pow_n (T value) noexcept G_GNUC_PURE;

template<unsigned N, typename T> [[gnu::always_inline]] static inline T pow_n (T value) noexcept
{

  if constexpr (1 == N)

    return value;
  else
    return pow_n<(N >> 1)> (value) * pow_n<N - (N >> 1)> (value);
}

[[gnu::always_inline]]
static inline gdouble kurtosis (const std::vector<gdouble>& data, gdouble mean, gdouble stddev) noexcept
{

  gdouble sum = 0;

  if (0 == stddev)
    return 0;

  for (gdouble value: data)
    sum += pow_n<4> ((value - mean) / stddev);

return sum / data.size () - 3.;
}

[[gnu::always_inline]]
static inline gdouble skewness (const std::vector<gdouble>& data, gdouble mean, gdouble stddev) noexcept
{

  gdouble sum = 0;

  if (0 == stddev)
    return 0;

  for (gdouble value: data)
    sum += pow_n<3> ((value - mean) / stddev);

return sum / (gdouble) data.size ();
}

[[gnu::always_inline]]
static inline gdouble percentile (const std::vector<gdouble>& sorted, double per) noexcept
{

  if (sorted.empty ())
    return 0;

  auto rank = (per / 100.) * (gdouble) sorted.size ();
  rank = std::clamp (rank, 0., (gdouble) (sorted.size () - 1));

  auto lower_index = (size_t) std::floor (rank);
  auto upper_index = (size_t) std::ceil (rank);

  if (lower_index == upper_index)
    return sorted [lower_index];

  auto weight = rank - lower_index;
  auto value = sorted [lower_index] * (1 - weight) + sorted [upper_index] * weight;
return value;
}

void testing::g_test_analyze_times (const std::vector<gdouble>& times) noexcept
{

  if (times.empty ())
    return (g_test_message ("no timing data collected"), void ());

  auto g_str = g_string_sized_new (256);
  auto count = times.size ();

  double mean, sum, q1, q2, q3;

  g_string_append_printf (g_str, "min: %lf\n", times.front ());
  g_string_append_printf (g_str, "max: %lf\n", times.back ());
  g_string_append_printf (g_str, "range: %lf\n", times.back () - times.front ());

  sum = std::accumulate (times.begin (), times.end (), 0.);

  g_string_append_printf (g_str, "mean: %lf\n", mean = sum / (double) count);
  g_string_append_printf (g_str, "median: %lf\n", q2 = percentile (times, 50.));

  g_string_append_printf (g_str, "quartiles\n");
  g_string_append_printf (g_str, "  " "75%%: %lf\n", q3 = percentile (times, 75.));
  g_string_append_printf (g_str, "  " "50%%: %lf\n", q2 = percentile (times, 50.));
  g_string_append_printf (g_str, "  " "25%%: %lf\n", q1 = percentile (times, 25.));
  g_string_append_printf (g_str, "  " "iqr: %lf\n", q3 - q1);

  g_string_append_printf (g_str, "percentiles\n");
  g_string_append_printf (g_str, "  " "95%%: %lf\n", percentile (times, 95.));
  g_string_append_printf (g_str, "  " "90%%: %lf\n", percentile (times, 90.));
  g_string_append_printf (g_str, "  " "80%%: %lf\n", percentile (times, 80.));
  g_string_append_printf (g_str, "  " "10%%: %lf\n", percentile (times, 10.));

  sum = 0;

  for (gdouble value: times)
    sum += pow_n<2> (value - mean);

  auto variance = sum / (double) (count - 1);
  auto stddev = (count > 1) ? std::sqrt (variance) : 0.;
  auto variance_p = sum / (double) count;
  auto stddev_p = (count > 0) ? std::sqrt (variance_p) : 0.;

  g_string_append_printf (g_str, "variance: %lf\n", variance);
  g_string_append_printf (g_str, "standard deviation: %lf\n", stddev);

  g_string_append_printf (g_str, "population variance: %lf\n", variance_p);
  g_string_append_printf (g_str, "population standard deviation: %lf\n", stddev_p);

  g_string_append_printf (g_str, "skewness: %lf\n", skewness (times, mean, stddev));
  g_string_append_printf (g_str, "kurtosis: %lf\n", kurtosis (times, mean, stddev));

  g_test_message ("%.*s", (int) g_str->len, g_str->str);
  g_string_free (g_str, TRUE);
}

[[gnu::always_inline]]
static inline gchar* canonical_file_name (std::string_view relative) noexcept
{

  relative = '/' != relative[0] ? relative : relative.substr (1);
  size_t pos;

  auto prefix = relative.substr (0, pos = relative.find_first_of ('/'));
  auto suffix = relative.substr (pos + 1);

return g_strdup_printf ("%.*s.p/%.*s.json", (int) prefix.length (), prefix.data (), (int) suffix.length (), suffix.data ());
}

[[gnu::always_inline]]
static inline gchar* relative_file_name () noexcept
{

  if (std::string_view path = g_test_get_dir (G_TEST_BUILT); ! path.starts_with (BUILD_DIR))
    g_error ("malformed test path");

  else if (std::string_view name = g_test_get_path (); ! name.starts_with (TESTPATHROOT))
    g_error ("malformed test path");

  else if (std::string_view prefix1 = path.substr (sizeof (BUILD_DIR) - 1),
                            prefix2 = name.substr (sizeof (TESTPATHROOT) - 1); ! prefix2.starts_with (prefix1))
    g_error ("malformed test path");
  else

return canonical_file_name (prefix2.substr (prefix1.length ()));
}

void testing::g_test_save_times (const std::vector<gdouble>& times) noexcept
{

  auto current = g_file_new_for_path (g_test_get_dir (G_TEST_BUILT));
  auto relative = relative_file_name ();
  auto file = g_file_resolve_relative_path (current, relative);

  g_object_unref (current);
  g_free (relative);

  GError* tmperr = NULL;
  GFile* parent = g_file_get_parent (file);

  if ((g_file_make_directory_with_parents (parent, NULL, &tmperr), g_object_unref (parent));
       G_UNLIKELY (NULL != tmperr) && G_UNLIKELY (! g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_EXISTS)))
    g_error ("g_file_make_directory_with_parents()!: %s: %u: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);

  g_clear_error (&tmperr);

  GOutputStream* stream = (GOutputStream*) g_file_replace (file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &tmperr);

  if (g_object_unref (file); G_UNLIKELY (NULL != tmperr))
    g_error ("g_file_create()!: %s: %u: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);

  if (g_output_stream_printf (stream, NULL, NULL, &tmperr, "{\n  \"data\": ["); G_UNLIKELY (NULL != tmperr))
    g_error ("g_output_stream_printf()!: %s: %u: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);

  std::string_view bit;
  bool first = true;

  for (gchar buffer [4 + G_ASCII_DTOSTR_BUF_SIZE] = { ' ', ' ', ' ', ' ' }; const double n: times)
    {

      if (! first)

        { bit = ",\n"; }
      else
        { bit =  "\n"; first = false; }

      if (g_output_stream_write_all (stream, bit.data (), bit.length (), NULL, NULL, &tmperr); G_UNLIKELY (NULL != tmperr))
        g_error ("g_output_stream_write_all()!: %s: %u: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);

      std::string_view view = (g_ascii_dtostr (&buffer [4], sizeof (buffer), n), buffer);

      if (g_output_stream_write_all (stream, view.data (), view.length (), NULL, NULL, &tmperr); G_UNLIKELY (NULL != tmperr))
        g_error ("g_output_stream_write_all()!: %s: %u: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);
    }

  if (g_output_stream_printf (stream, NULL, NULL, &tmperr, "\n  ]\n}\n"); G_UNLIKELY (NULL != tmperr))
    g_error ("g_output_stream_printf()!: %s: %u: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);

  g_output_stream_close (stream, NULL, NULL);
  g_object_unref (stream);
}