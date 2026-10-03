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
#pragma once
#include <csignal>
#include <gio/gio.h>
#include <gplusplus/asynclib/error.h>
#include <gplusplus/common/boxing.h>
#include <string_view>

namespace testing
{
  class socket_server;
}

#ifndef PYTHON_EXE
# define PYTHON_EXE "python3"
#endif // PYTHON_EXE

class testing::socket_server
{

  gchar* _hash = nullptr;
  GSubprocess* _subprocess = nullptr;

  static void child_setup (gpointer user_data) noexcept;
  void finish_setup (std::string_view line);
public:

  inline ~socket_server ()
    {

      g_free (_hash);

      if (G_UNLIKELY (nullptr == _subprocess))
        return;

      g_subprocess_send_signal (_subprocess, SIGINT);
      g_subprocess_wait (_subprocess, NULL, NULL);
      g_object_unref (_subprocess);
    }

  inline std::string_view get_hash () const noexcept
    { return std::string_view (_hash); }

  inline socket_server (guint16 port = 8000, std::string_view blob_size_ = "1MiB", std::string_view local_address_ = "")
    {

      constexpr GSubprocessFlags flag1 = G_SUBPROCESS_FLAGS_STDERR_SILENCE;
      constexpr GSubprocessFlags flag2 = G_SUBPROCESS_FLAGS_STDOUT_PIPE;
      constexpr GSubprocessFlags flags = (GSubprocessFlags) (flag1 | flag2);
      GError* tmperr = nullptr;

      auto blob_size = boxing::freeable<gchar> (g_strndup (blob_size_.data (), blob_size_.size ()));
      auto local_address = boxing::freeable<gchar> (g_strndup (local_address_.data (), local_address_.size ()));
      auto port_string = boxing::freeable<gchar> (g_strdup_printf ("%i", (int) port));
      auto server_script = boxing::freeable<gchar> (g_build_filename (SOURCE_DIR, "server.py", NULL));

      auto subprocess_launcher = boxing::object<GSubprocessLauncher> (g_subprocess_launcher_new (flags));
      g_subprocess_launcher_set_child_setup (subprocess_launcher, child_setup, NULL, NULL);

      auto subprocess = boxing::object<GSubprocess> (g_subprocess_launcher_spawn (subprocess_launcher, &tmperr,
        PYTHON_EXE, server_script.get (), "-b", local_address.get (), "-p", port_string.get (), "-s", blob_size.get (), NULL));

      if (G_UNLIKELY (NULL != tmperr))
        throw boxing::error (tmperr);

      auto stdout_pipe = boxing::object<GDataInputStream> (g_data_input_stream_new (g_subprocess_get_stdout_pipe (subprocess)));
      auto stdout_size = (gsize) 0;
      g_buffered_input_stream_set_buffer_size ((GBufferedInputStream*) stdout_pipe.get (), 1);

      auto line = g_data_input_stream_read_line_utf8 (stdout_pipe, &stdout_size, NULL, &tmperr);

      if (G_LIKELY (NULL == tmperr && NULL != line))

        { finish_setup (line);

          g_free (line);

          _subprocess = subprocess.release (); }
      else
        {
          /* the server failed to start: kill the child so it does not outlive the test */
          if (G_LIKELY (NULL != subprocess))
            g_subprocess_send_signal (subprocess, SIGINT),
            g_subprocess_wait (subprocess, NULL, NULL);

          throw boxing::error (NULL != tmperr ? tmperr : g_error_new_literal (G_IO_ERROR, G_IO_ERROR_FAILED, "http server didn't started"));
        }
    }
};