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
#include <gplusplus/common/constexprregistry.h>
#include <gplusplus/object/objectclass.h>
#include <gplusplus/object/objectimplementorbase.h>
#include <gplusplus/object/objectimplementorcallables.h>
#include <gplusplus/object/objectimplementorclass.h>
#include <gplusplus/object/objectimplementorinterface.h>
#include <gplusplus/object/objectimplementorproperties.h>
#include <gplusplus/object/objectimplementorstructs.h>

# define _GPP_IMPLEMENT(TypeName,type_name,tag,...) \
 ; \
  struct __gplusplus_implementor_info_base_##TypeName \
    { \
 ; \
      using Type = TypeName; \
      using TypeTag = tag; \
 ; \
      struct interface_adl_tag { }; \
      struct interface_first_adl_tag { }; \
 ; \
      struct property_adl_tag { }; \
      struct property_first_adl_tag { }; \
 ; \
      static gpointer parent_class; \
      static inline constexpr auto ClassName = constexpr_string (#TypeName); \
    }; \
 ; \
    gpointer __gplusplus_implementor_info_base_##TypeName::parent_class = nullptr; \
 ; \
  struct __gplusplus_implementor_info_##TypeName: public __gplusplus_implementor_info_base_##TypeName, \
                                                    public gplusplus::object::details::store_self<__gplusplus_implementor_info_##TypeName> \
    __VA_ARGS__; \
  using __gplusplus_implementor_type_##TypeName = \
    gplusplus::object::details::build_class::object_class<__gplusplus_implementor_info_##TypeName, __COUNTER__>; \
 ; \
  GType type_name##_get_type () G_GNUC_CONST; \
  GType type_name##_get_type () { return __gplusplus_implementor_type_##TypeName::get_type (); }

# define GPP_IMPLEMENT_ABSTRACT(TypeName,type_name,...) _GPP_IMPLEMENT(TypeName, type_name, gplusplus::object::type_tag::abstract, __VA_ARGS__)

# define GPP_IMPLEMENT_FINAL(TypeName,type_name,...) _GPP_IMPLEMENT(TypeName, type_name, gplusplus::object::type_tag::final, __VA_ARGS__)

# define GPP_IMPLEMENT(TypeName,type_name,...) _GPP_IMPLEMENT(TypeName, type_name, gplusplus::object::type_tag::normal, __VA_ARGS__)

# define GPP_IMPLEMENT_ANCESTOR(TypeName,type_name) \
  static inline constexpr auto Ancestor = (gplusplus::object::object_class_ancestor<TypeName##Class, TypeName, type_name##_get_type> ());

# define GPP_IMPLEMENT_INTERFACE(TypeName,type_name,...) \
  static constexpr constexpr_registry::install_once<interface_first_adl_tag, \
    1 + __COUNTER__, __COUNTER__> G_GNUC_UNUSED G_PASTE (_reg_id_, __COUNTER__) {}; \
  static constexpr constexpr_registry::register_<interface_adl_tag, __COUNTER__ - 2, \
    gplusplus::object::details::object_class_interface_<TypeName, TypeName##Iface, type_name##_get_type, __VA_ARGS__>::type {}> G_GNUC_UNUSED G_PASTE (_reg_prop_, __COUNTER__) {};

# define GPP_IMPLEMENT_CLASS_VTABLE(...) struct VTable __VA_ARGS__;

# define GPP_IMPLEMENT_BASE_CLASS_INIT(...) \
  static inline void base_class_init (gplusplus::object::details::build_class::class_struct<Self>* klass) noexcept \
    __VA_ARGS__;

# define GPP_IMPLEMENT_BASE_CLASS_FINI(...) \
  static inline void base_class_fini (gplusplus::object::details::build_class::class_struct<Self>* klass) noexcept \
    __VA_ARGS__;

# define GPP_IMPLEMENT_CLASS_INIT(...) \
  static inline void class_init (gplusplus::object::details::build_class::class_struct<Self>* klass, gpointer class_data) noexcept \
    { \
      parent_class = g_type_class_peek_parent ((gpointer) klass); \
      ([](decltype (klass) klass) noexcept -> void __VA_ARGS__) (klass); \
    }

# define GPP_IMPLEMENT_CLASS_FINI(...) \
  static inline void class_fini (gplusplus::object::details::build_class::class_struct<Self>* klass, \
      gpointer class_data) noexcept \
    __VA_ARGS__;

# define GPP_IMPLEMENT_INSTANCE_INIT(...) \
  static inline void instance_init (gplusplus::object::details::build_class::instance_struct<Self>* p_self, \
                                    gplusplus::object::details::build_class::class_struct<Self>* klass) noexcept \
    { \
      ([](decltype (p_self) p_self, Type& self, decltype (klass) klass) noexcept -> void __VA_ARGS__) (p_self, p_self->self, klass); \
    }

# define GPP_IMPLEMENT_DEFAULT_INSTANCE_INIT(...) GPP_IMPLEMENT_INSTANCE_INIT({ new (&p_self->self) Type (__VA_ARGS__); })

# define GPP_IMPLEMENT_CONSTRUCTED(...) \
  static inline void class_constructed (GObject* p_self) noexcept \
    { using instance_struct = gplusplus::object::details::build_class::instance_struct<Self>; \
      G_OBJECT_CLASS (Self::parent_class)->constructed (p_self); \
      ([](instance_struct* p_self, Type& self) noexcept -> void __VA_ARGS__) ((instance_struct*) p_self, ((instance_struct*) p_self)->self); \
    }

# define GPP_IMPLEMENT_DISPOSE(...) \
  static inline void class_dispose (GObject* p_self) noexcept \
    { using instance_struct = gplusplus::object::details::build_class::instance_struct<Self>; \
      ([](instance_struct* p_self, Type& self) noexcept -> void __VA_ARGS__) ((instance_struct*) p_self, ((instance_struct*) p_self)->self); \
      G_OBJECT_CLASS (Self::parent_class)->dispose (p_self); \
    }

# define GPP_IMPLEMENT_FINALIZE(...) \
  static inline void class_finalize (GObject* p_self) noexcept \
    { using instance_struct = gplusplus::object::details::build_class::instance_struct<Self>; \
      ([](instance_struct* p_self, Type& self) noexcept -> void __VA_ARGS__) ((instance_struct*) p_self, ((instance_struct*) p_self)->self); \
      G_OBJECT_CLASS (Self::parent_class)->finalize (p_self); \
    }

# define GPP_IMPLEMENT_DEFAULT_FINALIZE(...) GPP_IMPLEMENT_FINALIZE ( \
  { \
    ([](decltype (p_self) p_self, Type& self) noexcept -> void __VA_ARGS__) (p_self, p_self->self); \
    (&p_self->self)->~Type (); \
  })

# define GPP_IMPLEMENT_PROPERTY(type,...) \
  static constexpr constexpr_registry::install_once<property_first_adl_tag, \
    1 + __COUNTER__, __COUNTER__> G_GNUC_UNUSED G_PASTE (_reg_id_, __COUNTER__) {}; \
  static constexpr constexpr_registry::register_<property_adl_tag, __COUNTER__ - 2, \
    gplusplus::object::object_class_property_##type <gplusplus::object::details::build_class::instance_struct<Self>, __VA_ARGS__> {}> G_GNUC_UNUSED G_PASTE (_reg_prop_, __COUNTER__) {};

# define GPP_IMPLEMENT_INSTALL_PROPERTIES \
  gplusplus::object::details::build_class::properties_installer<Self, __COUNTER__>::install (klass);

# define GPP_IMPLEMENT_DEFAULT_GET_PROPERTY \
  gplusplus::object::details::build_class::properties_installer<Self, __COUNTER__>::get_property;

# define GPP_IMPLEMENT_DEFAULT_SET_PROPERTY \
  gplusplus::object::details::build_class::properties_installer<Self, __COUNTER__>::set_property;