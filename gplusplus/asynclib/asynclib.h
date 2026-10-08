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
#include <gplusplus/asynclib/asyncfunction.h>
#include <gplusplus/asynclib/asyncoffload.h>
#include <gplusplus/asynclib/asynctask.h>
#include <gplusplus/asynclib/asynctaskall.h>
#include <gplusplus/asynclib/asynctaskawaitable.h>
#include <gplusplus/asynclib/asynctaskcoroutine.h>
#include <gplusplus/asynclib/asynctaskfunction.h>
#include <functional>

namespace gplusplus::asynclib
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

  template<details::async_task_type... Tasks>
    requires ((std::same_as<void, typename Tasks::end_details::return_type> && ...)
           || ((! std::same_as<void, typename Tasks::end_details::return_type>) && ...))
  static inline constexpr auto all (Tasks&&... tasks) noexcept (std::is_nothrow_invocable_v<details::async_task_all_builder<Tasks ...>, std::tuple<std::remove_cvref_t<Tasks> ...>>)
    {
      using task_tuple = std::tuple<std::remove_cvref_t<Tasks> ...>;
      return details::async_task_all_builder<Tasks...>::build (task_tuple (std::forward<Tasks> (tasks) ...));
    }
}