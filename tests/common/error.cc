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
#include <gio/gio.h>
#include <gio++/common/boxing.h>
#include <tests/testing.h>
using namespace testing;

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/literal", []
    {

      auto error = boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, "boom");
      g_assert_cmpuint (error->domain, ==, G_IO_ERROR);
      g_assert_cmpint (error->code, ==, G_IO_ERROR_FAILED);
      g_assert_cmpstr (error->message, ==, "boom");
    });

  g_test_add_action (TESTPATHROOT "/printf", []
    {

      auto message = g_strdup_printf ("formatted %i %s", 42, "value");
      auto error = boxing::error::printf (G_IO_ERROR, G_IO_ERROR_FAILED, "formatted %i %s", 42, "value");
      g_assert_cmpstr (error->message, ==, message);
      g_free (message);
    });

  g_test_add_action (TESTPATHROOT "/copy", []
    {

      auto e1 = boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, "copy me");
      auto e2 = boxing::error (e1);

      /* boxing::error copies deep: the pointer is distinct, the content equal */
      g_assert_cmpuint ((guintptr) *e1, !=, (guintptr) *e2);
      g_assert_cmpstr (e2->message, ==, "copy me");

      /* copying a null error stays null */
      auto nothing = boxing::error ();
      auto copy = boxing::error (nothing);
      g_assert_true (nullptr == *copy);
    });

  g_test_add_action (TESTPATHROOT "/assign", []
    {

      /* NOTE: overwriting a non-null error deliberately emits a g_warning
       * (mirroring the GLib "error must be NULL before set" rule). That path is
       * intentionally not exercised here because a g_warning logged from within
       * a g_test process aborts the test (and neither g_test_expect_message nor
       * a fatal handler intercepts it with structured logging enabled). */
      auto e1 = boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, "old");
      auto e2 = boxing::error::literal (G_IO_ERROR, G_IO_ERROR_CANCELLED, "new");

      /* assigning onto a null error does not warn */
      auto empty = boxing::error ();
      empty = e2;
      g_assert_cmpstr (empty->message, ==, "new");

      /* assigning a copy onto a null error deep-copies */
      auto empty2 = boxing::error ();
      empty2 = e1;
      g_assert_cmpuint ((guintptr) *empty2, !=, (guintptr) *e1);
      g_assert_cmpstr (empty2->message, ==, "old");
    });

  g_test_add_action (TESTPATHROOT "/self_assign", []
    {

      auto error = boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, "self");

      /* self assignment is a no-op (and must not warn) */
      error = error;
      g_assert_cmpstr (error->message, ==, "self");

      /* adopting a fresh GError onto a null box does not warn either */
      auto raw = g_error_new_literal (G_IO_ERROR, G_IO_ERROR_FAILED, "adopted");
      auto fresh = boxing::error ();
      fresh = raw;
      g_assert_cmpuint ((guintptr) *fresh, ==, (guintptr) raw);
      g_assert_cmpstr (fresh->message, ==, "adopted");
    });

  g_test_add_action (TESTPATHROOT "/move", []
    {

      auto e1 = boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, "moved");
      auto raw = (gpointer) *e1;
      auto e2 = boxing::error ();

      /* moving transfers ownership instead of deep-copying */
      e2 = std::move (e1);

      g_assert_cmpuint ((guintptr) *e1, ==, 0);
      g_assert_cmpuint ((guintptr) *e2, ==, (guintptr) raw);
      g_assert_cmpstr (e2->message, ==, "moved");
    });

  g_test_add_action (TESTPATHROOT "/adopt", []
    {

      auto raw = g_error_new_literal (G_IO_ERROR, G_IO_ERROR_FAILED, "adopted");
      auto error = boxing::error ();

      error = raw;
      g_assert_cmpuint ((guintptr) *error, ==, (guintptr) raw);
      g_assert_cmpstr (error->message, ==, "adopted");
    });

return g_test_run ();
}