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
#include <type_traits>

namespace traits
{

  template<typename... Types>
  struct type_list { };

  template<typename,typename>
  struct type_list_contains;

  template<typename Type, typename... Types>
  struct type_list_contains<Type, type_list<Types ...>>
    { static inline constexpr bool value = (std::is_same_v<Type, Types> || ...); };

  template<typename Type, typename Tuple>
  static inline constexpr bool type_list_contains_v = type_list_contains<Type, Tuple>::value;

  template<typename... Lists>
  struct type_list_combine;

  template<>
  struct type_list_combine<>
    { using type = type_list<>; };

  template<typename... Types>
  struct type_list_combine<type_list<Types ...>>
    { using type = type_list<Types ...>; };

  template<typename... Types1, typename... Types2>
  struct type_list_combine<type_list<Types1 ...>, type_list<Types2 ...>>
    { using type = type_list<Types1 ..., Types2 ...>; };

  template<typename... Types1, typename... Types2, typename... Rest>
  struct type_list_combine<type_list<Types1 ...>, type_list<Types2 ...>, Rest ...>
    { using type = typename type_list_combine<type_list<Types1 ..., Types2 ...>, Rest ...>::type; };

  template<typename... Lists>
  using type_list_combine_t = typename type_list_combine<Lists ...>::type;

  template<typename... Types>
  struct type_list_of_unique;

  template<>
  struct type_list_of_unique<>
    { using type = type_list<>; };

  template<typename Type, typename... Rest>
  struct type_list_of_unique<Type, Rest ...>
    { using type = std::conditional_t<type_list_contains_v<Type, type_list<Rest ...>>,
        typename type_list_of_unique<Rest ...>::type, type_list_combine_t<type_list<Type>, typename type_list_of_unique<Rest ...>::type>>; };

  template<typename... Types>
  using type_list_of_unique_t = type_list_of_unique<Types ...>::type;

  template<template<typename...> typename Container,
           typename...>
  struct type_list_to_container;

  template<template<typename...> typename Container,
           typename... Types>
  struct type_list_to_container<Container, type_list<Types ...>>
    { using type = Container<Types ...>; };

  template<template<typename...> typename Container,
           typename List>
  using type_list_to_container_t = typename type_list_to_container<Container, List>::type;
}