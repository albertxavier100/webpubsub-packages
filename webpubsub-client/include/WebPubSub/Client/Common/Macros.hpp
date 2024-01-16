#pragma once

#define _WPS_ASSERT_INTERFACE_IMPLEMENTATION(Interface, Type)                  \
  static_assert(std::is_base_of<Interface<Type>, Type>::value,                 \
                #Type " does not implement the interface " #Interface ".");

#define _WPS_ADD_TMEM(Type, Interface, Member)                                 \
  _WPS_ASSERT_INTERFACE_IMPLEMENTATION(Interface, Type)                        \
  Type Member;

#define _WPS_ADD_TREF(Type, Interface, Member)                                 \
  _WPS_ASSERT_INTERFACE_IMPLEMENTATION(Interface, Type)                        \
  Type &Member;

#define _WPS_ADD_TCREF(Type, Interface, Member)                                \
  _WPS_ASSERT_INTERFACE_IMPLEMENTATION(Interface, Type)                        \
  const Type &Member;