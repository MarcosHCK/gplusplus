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
#include <gplusplus/common/boxingconcepts.h>
#include <memory>

namespace boxing
{

  template<typename T>
  using box_copy_func = T* (*) (T*);

  template<typename T>
  using box_free_func = void (*) (T*);

  template<typename T,
           details::_free_func<T> auto _free_func = nullptr>
  struct function_deleter;

  template<typename T,
           details::_copy_func<T> auto _copy_func = nullptr,
           details::_free_func<T> auto _free_func = nullptr>
     requires (nullptr != _copy_func)
  class shared_ptr;

  template<typename T,
           details::_free_func<T> auto _free_func = nullptr>
  class unique_ptr;

  class bytes;
  class error;
  template<typename T> class freeable;
  template<typename T> class object;
  class variant;
}

template<typename T,
         boxing::details::_free_func<T> auto _free_func>
struct boxing::function_deleter
{

  inline constexpr function_deleter () noexcept = default;

  template<typename U,
           boxing::details::_free_func<U> auto _U_free_func>
    requires (std::is_convertible_v<U*, T*>)
  inline constexpr function_deleter (const function_deleter<U, _U_free_func>& o) noexcept
    { }

  inline constexpr void operator() (T* ptr) const
      noexcept (std::is_nothrow_invocable_r_v<void, decltype (_free_func), T*>)
    {

      if constexpr (nullptr != _free_func)
        _free_func (ptr);

      static_assert (std::is_void<T>::value == false, "can not delete a pointer to incomplete type");
    }
};

template<typename T,
         boxing::details::_copy_func<T> auto _copy_func,
         boxing::details::_free_func<T> auto _free_func>
   requires (nullptr != _copy_func)
class boxing::shared_ptr: public std::unique_ptr<T, boxing::function_deleter<T, _free_func>>
{
public:

  inline constexpr shared_ptr (T* value = nullptr) noexcept:
      std::unique_ptr<T, boxing::function_deleter<T, _free_func>> (value)
    { }

  inline constexpr shared_ptr (shared_ptr<T, _copy_func, _free_func>&& o)
      noexcept (std::is_nothrow_move_constructible_v<std::unique_ptr<T, boxing::function_deleter<T, _free_func>>>):
      std::unique_ptr<T, boxing::function_deleter<T, _free_func>> ((std::unique_ptr<T, boxing::function_deleter<T, _free_func>>&&) std::move (o))
    { }

  inline constexpr shared_ptr (const shared_ptr& o) noexcept (std::is_nothrow_invocable_r_v<T*, decltype (_copy_func), T*>)
      : std::unique_ptr<T, boxing::function_deleter<T, _free_func>> (_copy_func (o.get ()))
    { }

  inline constexpr T* operator* () const noexcept { return this->get (); }
  inline constexpr operator T* () const noexcept { return this->get (); }

  inline constexpr auto& operator= (const shared_ptr& o) noexcept (std::is_nothrow_copy_constructible_v<decltype (*this)>)
    {
      shared_ptr<T, _copy_func, _free_func> tmp (o);
    return (this->swap (tmp), *this);
    }

  inline constexpr auto& operator= (shared_ptr<T, _copy_func, _free_func>&& o) noexcept (std::is_nothrow_move_assignable_v<std::unique_ptr<T, boxing::function_deleter<T, _free_func>>>)
    {
      std::unique_ptr<T, boxing::function_deleter<T, _free_func>>::operator= (std::move (o));
    return *this;
    }

protected:

  static inline T* copy (T* value) noexcept (std::is_nothrow_invocable_r_v<T*, decltype (_copy_func), T*>)
    {
      return _copy_func (value);
    }
};

template<typename T,
         boxing::details::_free_func<T> auto _free_func>
class boxing::unique_ptr: public std::unique_ptr<T, boxing::function_deleter<T, _free_func>>
{
public:

  inline constexpr unique_ptr (T* value = nullptr) noexcept:
      std::unique_ptr<T, boxing::function_deleter<T, _free_func>> (value)
    { }

  inline constexpr unique_ptr (unique_ptr<T, _free_func>&& o)
      noexcept (std::is_nothrow_move_constructible_v<std::unique_ptr<T, boxing::function_deleter<T, _free_func>>>):
      std::unique_ptr<T, boxing::function_deleter<T, _free_func>> ((std::unique_ptr<T, boxing::function_deleter<T, _free_func>>&&) std::move (o))
    { }

  inline constexpr T* operator* () const noexcept { return this->get (); }
  inline constexpr operator T* () const noexcept { return this->get (); }
};