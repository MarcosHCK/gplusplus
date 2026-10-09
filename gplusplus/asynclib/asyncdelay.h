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
#include <chrono>
#include <cmath>
#include <gplusplus/asynclib/asyncfunction.h>
#include <limits>

namespace gplusplus::asynclib::details
{

  template<class T>
  struct is_duration: std::false_type { };

  template<class Rep, class Period>
  struct is_duration<std::chrono::duration<Rep, Period>>: std::true_type { };

  template<class T>
  static inline constexpr bool is_duration_v = is_duration<T>::value;

  template<typename T>
  concept time_like = is_duration_v<std::remove_cvref_t<T>>;

  template<time_like T>
  static inline std::pair<bool, guint> normalize_delay (T interval) noexcept
    {

      const auto count = static_cast<long double> (interval.count ());

      if (G_UNLIKELY (! std::isfinite (count) || 0 >= count))
        return { false, 0 };

      const auto scale = static_cast<long double> (T::period::num)
                       / static_cast<long double> (T::period::den)
                       * 1000.0L;

      if (G_UNLIKELY (! std::isfinite (scale) || 0 >= scale))
        return { false, 0 };

      /* Preserve the positive-delay behavior even on platforms where
       * converting a tiny positive duration to long double underflows. */
      constexpr auto maximum = static_cast<long double> (std::numeric_limits<guint>::max ());

      if (G_UNLIKELY (count > maximum / scale))
        return { false, 0 };

      if (const auto milliseconds = count * scale; 0 >= milliseconds)
        return { true, milliseconds };

      else if (const auto rounded = std::ceil (milliseconds); G_UNLIKELY (rounded > maximum))
        return { false, 0 };

      else
        return { true, static_cast<guint> (rounded) };
    }
}

extern "C"
{
  void gpp_asynclib_async_delay (guint interval, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data) noexcept;
  bool gpp_asynclib_async_delay_finish (GAsyncResult* async_result, GError** error) noexcept;
}

namespace gplusplus::asynclib::details
{

  template<details::time_like T>
  static inline void _asynclib_async_delay (T interval, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data) noexcept
    {

      if (auto [ valid, ms ] = normalize_delay (interval); G_LIKELY (valid))

        gpp_asynclib_async_delay (ms, io_priority, cancellable, callback, user_data);
      else
        g_task_report_new_error (NULL, callback, user_data, NULL, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, "invalid duration");
    }
}