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
#include <gplusplus/asynclib/asynclib.h>
#include <gplusplus/common/boxing.h>
#include <tests/asynclib/server.h>
#include <ranges>
#include <tests/testing.h>
#include <variant>
using namespace testing;

static void io_work_native (const gchar* hostname, guint16 port, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data);
static std::pair<gsize, gchar*> io_work_native_finish (GAsyncResult* result, GError** error);
static gplusplus::asynclib::task<std::pair<gsize, gchar*>> io_work_ours (const gchar* hostname, guint16 port, GCancellable* cancellable);

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  socket_server server (8000, "1MiB");
  constexpr guint tries = 500;

  g_test_add_action (TESTPATHROOT "/native", [&server]
    {

      gdouble took = 0;
      auto times = std::vector<gdouble> ();

      times.reserve (tries);

      for (guint i = 0; i < tries; ++i)
        {

          gchar* digest = NULL;
          gsize total = 0;
          guint ready = 0;

          using pair_t = decltype (io_work_native_finish (NULL, NULL));

          auto complete = std::function<void (pair_t&)> ([&] (pair_t& pair) noexcept
            { took = g_test_timer_elapsed ();
              total = pair.first; digest = pair.second; g_atomic_int_set (&ready, 1); });

          g_test_timer_start ();

          io_work_native ("localhost", 8000, NULL, [](GObject*, GAsyncResult* result, gpointer user_data)
            {

              GError* tmperr = NULL;

              auto pair = io_work_native_finish (result, &tmperr);
              g_assert_no_error (tmperr);

              (*(decltype (complete)*) user_data) (pair);
            }, &complete);

          for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
            g_main_context_iteration (context, FALSE);

          g_assert_cmpstr (digest, ==, server.get_hash ().data ());

          g_free (digest);
          times.push_back (took);
        }

      g_test_analyze_times_unsorted (std::views::all (times));
      g_test_save_times (times);
    });

  g_test_add_action (TESTPATHROOT "/ours", [&server]
    {

      gdouble took = 0;
      auto times = std::vector<gdouble> ();

      times.reserve (tries);

      for (guint i = 0; i < tries; ++i)
        {

          gchar* digest = NULL;
          gsize total = 0;
          guint ready = 0;

          auto task = io_work_ours ("localhost", 8000, NULL);
          struct D { decltype (digest)* digest; decltype (ready)* ready; decltype (took)* took; decltype (total)* total; }
               d = { .digest = &digest, .ready = &ready, .took = &took, .total = &total };
          g_test_timer_start ();

          task.begin ([](GObject*, GAsyncResult* result, gpointer user_data)
            {

              auto p = (D*) user_data;
              *p->took = g_test_timer_elapsed ();
              auto e = (GError*) nullptr;
              auto r = gplusplus::asynclib::task_function<io_work_ours> ().finish (result, &e);

              g_assert_no_error (e);
              (void) (*p->digest = r.second, *p->total = r.first);
              g_atomic_int_set (p->ready, 1);
            }, &d);

          for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
            g_main_context_iteration (context, FALSE);

          g_assert_cmpstr (digest, ==, server.get_hash ().data ());

          g_free (digest);
          times.push_back (took);
        }

      g_test_analyze_times_unsorted (std::views::all (times));
      g_test_save_times (times);
    });

return g_test_run ();
}

/* native */

#define _g_checksum_free0(var) ((NULL == var) ? NULL : (var = (g_checksum_free (var), nullptr)))
#define _g_resolver_free_addresses0(var) ((NULL == var) ? NULL : (var = (g_resolver_free_addresses (var), nullptr)))
#define _g_object_unref0(var) ((NULL == var) ? NULL : (var = (g_object_unref (var), nullptr)))

struct io_work_native_data
{

  struct stage_0
    {
      guint16 port;
    };

  struct stage_1
    {

      GList* addresses = nullptr;
      GList* link = nullptr;
      guint16 port = 0;
      GSocketClient* socket_client = nullptr;

      inline ~stage_1 ()
        {
          _g_resolver_free_addresses0 (addresses);
          _g_object_unref0 (socket_client);
        }

      inline stage_1 (stage_1&& o) noexcept:
          addresses (o.addresses), link (o.link), port (o.port), socket_client (o.socket_client)
        { o.addresses = NULL; o.link = NULL; o.socket_client = NULL; }  

      inline stage_1 (GList* _addresses, guint16 _port, GSocketClient* _socket_client) noexcept:
          addresses (_addresses), link (_addresses), port (_port), socket_client (_socket_client)
        { }                     
    };

  struct stage_2
    {

      stage_1 stack;
    };

  struct stage_3
    {

      GIOStream* stream = nullptr;

      inline ~stage_3 ()
        { _g_object_unref0 (stream); }

      inline stage_3 (GIOStream* _stream) noexcept:
          stream (_stream)
        { }
    };

  struct stage_4
    {

      gchar buffer [2048];
      GChecksum* checksum = nullptr;
      GIOStream* stream = nullptr;

      inline ~stage_4 ()
        {
          _g_checksum_free0 (checksum);
          _g_object_unref0 (stream);
        }

      inline stage_4 (stage_4&& o) noexcept:
          checksum (o.checksum), stream (o.stream)
        { o.checksum = NULL; o.stream = NULL; }

      inline stage_4 (GChecksum* _checksum, GIOStream* _stream) noexcept:
          checksum (_checksum), stream (_stream)
        { }
    };

  struct stage_5
    {

      stage_4 stack;
      gsize total = 0;
    };

  using stage_n = std::variant<stage_0, stage_1, stage_2, stage_3, stage_4, stage_5>;
  stage_n _stage;

  template<typename T>
    requires (std::is_constructible_v<stage_n, T>)
  inline constexpr T& get_stage () noexcept
    { return std::get<T> (_stage); }

  inline constexpr unsigned get_stage_n () const noexcept
    { return _stage.index (); }

  template<typename T>
    requires (std::is_constructible_v<stage_n, T>)
  inline constexpr void set_stage (T&& stage) noexcept (std::is_nothrow_constructible_v<stage_n, T>)
    { _stage.emplace<T> (std::forward<T> (stage)); }
};

static void io_work_native_co (GObject* source_object, GAsyncResult* result, gpointer user_data);

static void io_work_native (const gchar* hostname, guint16 port, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data)
{

  auto resolver = g_resolver_get_default ();
  auto task = g_task_new (NULL, cancellable, callback, user_data);

  auto task_data = new (g_slice_alloc0 (sizeof (io_work_native_data))) io_work_native_data ();
  task_data->set_stage (io_work_native_data::stage_0 { .port = port });

  g_task_set_task_data (task, task_data, [](gpointer data)
    {

      ((io_work_native_data*) data)->~io_work_native_data ();
      g_slice_free1 (sizeof (io_work_native_data), data);
    });

  g_resolver_lookup_by_name_async (resolver, hostname, cancellable, io_work_native_co, task);
}

static std::pair<gsize, gchar*> io_work_native_finish (GAsyncResult* result, GError** error)
{

  GError* tmperr = NULL;

  if (auto pair = (std::pair<gsize, gchar*>*) g_task_propagate_pointer ((GTask*) result, &tmperr); G_LIKELY (NULL == tmperr))

    return std::move (*pair);
  else
    return (g_propagate_error (error, tmperr), std::make_pair<gsize, gchar*> (0, NULL));
}

static void io_work_native_co (GObject* source_object, GAsyncResult* result, gpointer user_data)
{

  GTask* task = (GTask*) user_data;
  GError* tmperr = NULL;

  switch (auto& stage_data = *(io_work_native_data*) g_task_get_task_data (task); stage_data.get_stage_n ())
    {

    case 0: { auto& data = stage_data.get_stage<io_work_native_data::stage_0> ();

        GList* addresses;

        if (addresses = g_resolver_lookup_by_name_finish ((GResolver*) source_object, result, &tmperr); G_UNLIKELY (NULL != tmperr))
          return (g_task_return_error (task, tmperr), g_object_unref (task));

        auto socket_client = g_socket_client_new ();

        g_socket_client_set_protocol (socket_client, G_SOCKET_PROTOCOL_TCP);
        g_socket_client_set_socket_type (socket_client, G_SOCKET_TYPE_STREAM);
        g_socket_client_set_tls (socket_client, FALSE);

        stage_data.set_stage (io_work_native_data::stage_1 (addresses, data.port, socket_client));
        return io_work_native_co (source_object, result, user_data);
      }

    case 1: { auto& data = stage_data.get_stage<io_work_native_data::stage_1> ();

        if (NULL == data.link)
          return (g_task_return_new_error_literal (task, G_IO_ERROR, G_IO_ERROR_FAILED, "could not connect to any resolved address"), g_object_unref (task));

        auto inet_address = (GInetAddress*) data.link->data;
        data.link = data.link->next;

        auto cancellable = g_task_get_cancellable (task);
        auto socket_address = g_inet_socket_address_new (inet_address, data.port);
        auto socket_connectable = G_SOCKET_CONNECTABLE (socket_address);

        stage_data.set_stage (io_work_native_data::stage_2 { .stack = std::move (data) });

        g_socket_client_connect_async (data.socket_client, socket_connectable, cancellable, io_work_native_co, user_data);
        g_object_unref (socket_address);
        break;
      }

    case 2: { auto& data = stage_data.get_stage<io_work_native_data::stage_2> ();

        GSocketConnection* connection;

        if (connection = g_socket_client_connect_finish ((GSocketClient*) source_object, result, &tmperr); G_UNLIKELY (NULL != tmperr))

          { g_clear_error (&tmperr);
            stage_data.set_stage (io_work_native_data::stage_1 (std::move (data.stack))); }
        else
          /* Take an explicit reference for the duration of the pipeline: some GLib
           * versions only keep the connection returned by
           * g_socket_client_connect_finish() alive until the completion callback
           * returns (the reference is owned by the client-side connect data). */
          stage_data.set_stage (io_work_native_data::stage_3 ((GIOStream*) g_object_ref (connection)));

        return io_work_native_co (source_object, result, user_data);
      }

    case 3: { auto& data = stage_data.get_stage<io_work_native_data::stage_3> ();

        constexpr GChecksumType checksum_type = G_CHECKSUM_SHA256;

        constexpr const gchar request [] = "GET / HTTP/1.1\r\n"
                                           "Connection: close\r\n"
                                           "\r\n";

        auto cancellable = g_task_get_cancellable (task);
        auto output_stream = g_io_stream_get_output_stream (data.stream);

        stage_data.set_stage (io_work_native_data::stage_4 (g_checksum_new (checksum_type), g_object_ref (data.stream)));

        g_output_stream_write_all_async (output_stream, request, sizeof (request) - 1, G_PRIORITY_DEFAULT, cancellable, io_work_native_co, user_data);
        break;
      }

    case 4: { auto& data = stage_data.get_stage<io_work_native_data::stage_4> ();

        g_assert (G_IS_IO_STREAM (data.stream));
        auto written = (gsize) 0;

        if (g_output_stream_write_all_finish ((GOutputStream*) source_object, result, &written, &tmperr); G_UNLIKELY (NULL != tmperr))
          return (g_task_return_error (task, tmperr), g_object_unref (task));

        auto cancellable = g_task_get_cancellable (task);
        auto input_stream = g_io_stream_get_input_stream (data.stream);

        stage_data.set_stage (io_work_native_data::stage_5 { .stack = std::move (data), .total = 0 });

        auto buffer = stage_data.get_stage<io_work_native_data::stage_5> ().stack.buffer;

        g_input_stream_read_async (input_stream, buffer, sizeof (data.buffer), G_PRIORITY_DEFAULT, cancellable, io_work_native_co, user_data);
        break;
      }

    case 5: { auto& data = stage_data.get_stage<io_work_native_data::stage_5> ();
              auto& stack = data.stack;

        auto cancellable = g_task_get_cancellable (task);
        auto input_stream = g_io_stream_get_input_stream (stack.stream);

        if (auto read = g_input_stream_read_finish (input_stream, result, &tmperr); G_UNLIKELY (NULL != tmperr))
          return (g_task_return_error (task, tmperr), g_object_unref (task));

        else if (data.total += read; read > 0)
          {
            g_checksum_update (stack.checksum, (guchar*) stack.buffer, read);
            g_input_stream_read_async (input_stream, stack.buffer, sizeof (stack.buffer), G_PRIORITY_DEFAULT, cancellable, io_work_native_co, user_data);
            break;
          }

        auto sum = g_strdup (g_checksum_get_string (stack.checksum));
        auto pair = std::make_pair (data.total, sum);

        auto pointer = new (g_slice_alloc0 (sizeof (std::pair<gsize, gchar*>))) std::pair<gsize, gchar*> (std::move (pair));

        g_task_return_pointer (task, pointer, [](gpointer mem)
          {
            ((std::pair<gsize, gchar*>*) mem)->~pair<gsize, gchar*> ();
            g_slice_free1 (sizeof (std::pair<gsize, gchar*>), mem);
          });

        g_object_unref (task);
        break;
      }

    default: g_assert_not_reached ();
    }
}

/* ours */

static inline std::pair<gboolean, gsize> g_output_stream_write_all_finish_ (GOutputStream* stream, GAsyncResult* result, GError** error)
{

  gsize written;
  gboolean success = g_output_stream_write_all_finish (stream, result, &written, error);

return std::make_pair (success, written);
}

/* ours */

gplusplus::asynclib::async_function<g_socket_client_connect_async, g_socket_client_connect_finish> g_socket_client_connect_task;

static gplusplus::asynclib::task<GIOStream*> reach_any_task (GList* addresses, guint16 port, GCancellable* cancellable)
{

  auto socket_client = g_socket_client_new ();
  g_socket_client_set_protocol (socket_client, G_SOCKET_PROTOCOL_TCP);
  g_socket_client_set_socket_type (socket_client, G_SOCKET_TYPE_STREAM);
  g_socket_client_set_tls (socket_client, FALSE);

  for (auto link = addresses; NULL != link; link = link->next)
    {

      auto socket_address = g_inet_socket_address_new ((GInetAddress*) link->data, port);

      try
        { auto socket_connectable = G_SOCKET_CONNECTABLE (socket_address);
          auto io_stream = co_await g_socket_client_connect_task (socket_client, socket_connectable, cancellable);
          co_return (g_object_unref (socket_client), g_object_unref (socket_address), (GIOStream*) io_stream); }
      catch (const boxing::error&)
        { g_object_unref (socket_address); }
    }

  g_object_unref (socket_client);

throw boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, "could not connect to any resolved address");
}

gplusplus::asynclib::async_function<g_input_stream_read_async, g_input_stream_read_finish> g_input_stream_read_task;
gplusplus::asynclib::async_function<g_resolver_lookup_by_name_async, g_resolver_lookup_by_name_finish> g_resolver_lookup_by_name_task;
gplusplus::asynclib::async_function<g_output_stream_write_all_async, g_output_stream_write_all_finish_> g_output_stream_write_all_task;

static gplusplus::asynclib::task<std::pair<gsize, gchar*>> io_work_ours (const gchar* hostname, guint16 port, GCancellable* cancellable)
{

  auto resolver = g_resolver_get_default ();
  auto addresses = co_await g_resolver_lookup_by_name_task (resolver, hostname, cancellable);

  GIOStream* connection; try
    { connection = co_await reach_any_task (addresses, port, cancellable);
      g_resolver_free_addresses (addresses); }
  catch (const boxing::error&)
    { g_resolver_free_addresses (addresses); throw; }

  constexpr const gchar request [] = "GET / HTTP/1.1\r\n"
                                     "Connection: close\r\n"
                                     "\r\n";

  try
    { auto output_stream = g_io_stream_get_output_stream (connection);
      co_await g_output_stream_write_all_task (output_stream, request, G_N_ELEMENTS (request) - 1, G_PRIORITY_DEFAULT, cancellable); }
  catch (const boxing::error&)
    { g_object_unref (connection); throw; }

  auto checksum = g_checksum_new (G_CHECKSUM_SHA256);
  auto total = (gsize) 0;

  try
    {

      char buffer [2048];
      gssize read;

      for (auto input_stream = g_io_stream_get_input_stream (connection);
           0 < (read = co_await g_input_stream_read_task (input_stream, buffer, sizeof (buffer), G_PRIORITY_DEFAULT, cancellable));)
        g_checksum_update (checksum, (guchar*) buffer, (total += read, read));
      g_object_unref (connection);
    }
  catch (boxing::error)
    { g_checksum_free (checksum); g_object_unref (connection); throw; }

  auto pair = std::make_pair (total, g_strdup (g_checksum_get_string (checksum)));
co_return (g_checksum_free (checksum), pair);
}
