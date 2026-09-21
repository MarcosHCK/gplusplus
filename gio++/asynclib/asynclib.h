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
#include <gio++/asynclib/asyncfunction.h>
#include <gio++/asynclib/asyncoffload.h>
#include <gio++/asynclib/asynctask.h>
#include <gio++/asynclib/asynctaskawaitable.h>
#include <gio++/asynclib/asynctaskcoroutine.h>
#include <gio++/asynclib/asynctaskfunction.h>
#include <functional>

namespace gioplusplus::asynclib
{

  template<details::async_function_begin auto _Begin,
           details::async_function_end auto _End>
  using async_function = details::async_function<_Begin, _End>;

  template<details::async_task_promise_target Return>
  using task = details::async_task<details::async_task_promise<Return, std::move_only_function<void (GAsyncReadyCallback, gpointer)>>::_Begin,
                                   details::async_task_promise<Return, std::move_only_function<void (GAsyncReadyCallback, gpointer)>>::_End,
                                   std::move_only_function<void (GAsyncReadyCallback, gpointer)>>;

  template<details::async_task_function auto Function>
  using task_function = details::async_task_function_operable<Function>;

  template<details::async_task_function auto Function>
  static inline constexpr auto task_function_finish = task_function<Function>::finish;
}