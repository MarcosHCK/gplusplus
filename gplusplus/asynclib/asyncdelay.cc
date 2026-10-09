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
#include <gplusplus/asynclib/asyncdelay.h>
#include <gplusplus/common/boxing.h>

namespace boxing
{
  using source = unique_ptr<GSource, g_source_unref>;
}

static gboolean cancellable_source_func (GCancellable*, GTask* task) noexcept
{

  g_task_return_new_error_literal (task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "cancelled");
return G_SOURCE_REMOVE;
}

static gboolean timeout_source_func (GTask* task) noexcept
{

  if (! g_task_had_error (task))
    g_task_return_boolean (task, TRUE);

return G_SOURCE_REMOVE;
}

void gpp_asynclib_async_delay (guint interval, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data) noexcept
{

  boxing::object task = g_task_new (NULL, cancellable, callback, user_data);

  g_task_set_priority (task, io_priority);
  g_task_set_static_name (task, "gpp_asynclib_async_delay");
  (g_task_set_source_tag) (task, (gpointer) gpp_asynclib_async_delay);

  boxing::source cancellable_source = g_cancellable_source_new (cancellable);
  boxing::source timeout_source = g_timeout_source_new (interval);

  g_source_set_priority (cancellable_source, io_priority);
  g_source_set_priority (timeout_source, io_priority);

  g_source_add_child_source (timeout_source, cancellable_source);

  g_source_set_callback (cancellable_source, (GSourceFunc) cancellable_source_func, g_object_ref (*task), g_object_unref);
  g_source_set_callback (timeout_source, (GSourceFunc) timeout_source_func, g_object_ref (*task), g_object_unref);

  g_source_set_static_name (cancellable_source, "gpp_asynclib_async_delay");
  g_source_set_static_name (timeout_source, "gpp_asynclib_async_delay");

  g_source_attach (timeout_source, g_task_get_context (task));
}

bool gpp_asynclib_async_delay_finish (GAsyncResult* async_result, GError** error) noexcept
{
  return g_task_propagate_boolean ((GTask*) async_result, error);
}