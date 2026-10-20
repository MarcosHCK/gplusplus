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
#include <gio/gio.h>
#include <string_view>
#include <tests/testing.h>

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