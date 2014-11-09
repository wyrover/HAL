/**
 * JavaScriptCoreCPP
 * Author: Matthew D. Langston
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#include "JavaScriptCoreCPP/detail/JSUtil.hpp"
#include "JavaScriptCoreCPP/JSString.hpp"
#include "JavaScriptCoreCPP/JSValue.hpp"

#include <bitset>
#include <sstream>
#include <stdexcept>

#include <JavaScriptCore/JavaScript.h>


namespace JavaScriptCoreCPP { namespace detail {
  
  void ThrowLogicError(const std::string& internal_component_name, const std::string& message) {
    JAVASCRIPTCORECPP_LOG_ERROR(internal_component_name, ": ", message);
    throw std::logic_error(message);
  }
  
  void ThrowLogicError(const std::string& internal_component_name, const JSValue& exception) {
    const auto exception_message = to_string(exception);
    JAVASCRIPTCORECPP_LOG_ERROR(internal_component_name, ": ", exception_message);
    throw std::logic_error(exception_message);
  }
  
  void ThrowRuntimeError(const std::string& internal_component_name, const std::string& message) {
    JAVASCRIPTCORECPP_LOG_ERROR(internal_component_name, ": ", message);
    throw std::runtime_error(message);
  }
  
  void ThrowRuntimeError(const std::string& internal_component_name, const JSValue& exception) {
    const auto exception_message = to_string(exception);
    JAVASCRIPTCORECPP_LOG_ERROR(internal_component_name, ": ", exception_message);
    throw std::runtime_error(exception_message);
  }
  
  void ThrowInvalidArgument(const std::string& internal_component_name, const std::string& message) {
    JAVASCRIPTCORECPP_LOG_ERROR(internal_component_name, ": ", message);
    throw std::invalid_argument(message);
  }
  
  
  std::vector<JSValue> to_vector(const JSContext& js_context, size_t count, const JSValueRef js_value_ref_array[]) {
    std::vector<JSValue> js_value_vector;
    std::transform(js_value_ref_array,
                   js_value_ref_array + count,
                   std::back_inserter(js_value_vector),
                   [&js_context](JSValueRef value_ref) { return JSValue(js_context, value_ref); });
    return js_value_vector;
  }
  
  std::vector<JSValue> to_vector(const JSContext& js_context, const std::vector<JSString>& js_string_vector) {
    std::vector<JSValue> js_value_vector;
    std::transform(js_string_vector.begin(),
                   js_string_vector.end(),
                   std::back_inserter(js_value_vector),
                   [&js_context](const JSString& js_string) { return js_context.CreateString(js_string); });
    return js_value_vector;
  }
  
  std::vector<JSValueRef> to_vector(const std::vector<JSValue>& js_value_vector) {
    std::vector<JSValueRef> js_value_ref_vector;
    std::transform(js_value_vector.begin(),
                   js_value_vector.end(),
                   std::back_inserter(js_value_ref_vector),
                   [](const JSValue& js_value) { return static_cast<JSValueRef>(js_value); });
    return js_value_ref_vector;
  }
  
  std::vector<JSStringRef> to_vector(const std::vector<JSString>& js_string_vector) {
    std::vector<JSStringRef> js_string_ref_vector;
    std::transform(js_string_vector.begin(),
                   js_string_vector.end(),
                   std::back_inserter(js_string_ref_vector),
                   [](const JSString& js_string) { return static_cast<JSStringRef>(js_string); });
    return js_string_ref_vector;
  }
  
  JSPropertyAttributes ToJSPropertyAttributes(const std::unordered_set<JSPropertyAttribute>& attributes) noexcept {
    JSPropertyAttributes result = kJSPropertyAttributeNone;
    for (auto attribute : attributes) {
      switch (attribute) {
        case JSPropertyAttribute::None:
          result |= kJSPropertyAttributeNone;
          break;
          
        case JSPropertyAttribute::ReadOnly:
          result |= kJSPropertyAttributeReadOnly;
          break;
          
        case JSPropertyAttribute::DontEnum:
          result |= kJSPropertyAttributeDontEnum;
          break;
          
        case JSPropertyAttribute::DontDelete:
          result |= kJSPropertyAttributeDontDelete;
          break;
      }
    }
    
    return result;
  }
  
  std::unordered_set<JSPropertyAttribute> FromJSPropertyAttributes(::JSPropertyAttributes attributes) noexcept {
    std::unordered_set<JSPropertyAttribute> attribute_set;
    static_cast<void>(attributes == kJSPropertyAttributeNone       && attribute_set.emplace(JSPropertyAttribute::None).second);
    static_cast<void>(attributes &  kJSPropertyAttributeReadOnly   && attribute_set.emplace(JSPropertyAttribute::ReadOnly).second);
    static_cast<void>(attributes &  kJSPropertyAttributeDontEnum   && attribute_set.emplace(JSPropertyAttribute::DontEnum).second);
    static_cast<void>(attributes &  kJSPropertyAttributeDontDelete && attribute_set.emplace(JSPropertyAttribute::DontDelete).second);
    return attribute_set;
  }
  
  std::string to_string(JSPropertyAttribute attribute) noexcept {
    switch (attribute) {
      case JSPropertyAttribute::None:
        return "None";
        break;
        
      case JSPropertyAttribute::ReadOnly:
        return "ReadOnly";
        break;
        
      case JSPropertyAttribute::DontEnum:
        return "DontEnum";
        break;
        
      case JSPropertyAttribute::DontDelete:
        return "DontDelete";
        break;
    }
  }
  
  std::string to_string(const std::unordered_set<JSPropertyAttribute>& attributes) noexcept {
    std::string result;
    for (auto attribute : {JSPropertyAttribute::None, JSPropertyAttribute::ReadOnly, JSPropertyAttribute::DontEnum, JSPropertyAttribute::DontDelete}) {
      auto position = attributes.find(attribute);
      if (position != attributes.end()) {
        if (!result.empty()) {
          result += ", ";
        }
        result += to_string(*position);
      }
    }
    
    return result;
  }
  
  std::string to_string_JSPropertyAttributes(::JSPropertyAttributes attributes) noexcept {
    return to_string(FromJSPropertyAttributes(attributes));
  }
  
  unsigned ToJSClassAttribute(JSClassAttribute attribute) noexcept {
    JSClassAttributes attributes = kJSClassAttributeNone;
    switch (attribute) {
      case JSClassAttribute::None:
        attributes = kJSClassAttributeNone;
        break;
        
      case JSClassAttribute::NoAutomaticPrototype:
        attributes = kJSClassAttributeNone;
        break;
    }
    
    return attributes;
  }
  
  std::unordered_set<JSClassAttribute> FromJSClassAttributes(::JSClassAttributes attributes) noexcept {
    std::unordered_set<JSClassAttribute> attribute_set;
    static_cast<void>(attributes == kJSClassAttributeNone                 && attribute_set.emplace(JSClassAttribute::None).second);
    static_cast<void>(attributes &  kJSClassAttributeNoAutomaticPrototype && attribute_set.emplace(JSClassAttribute::NoAutomaticPrototype).second);
    return attribute_set;
  }
  
  
  std::string to_string(JSClassAttribute attribute) noexcept {
    switch (attribute) {
      case JSClassAttribute::None:
        return "None";
        break;
        
      case JSClassAttribute::NoAutomaticPrototype:
        return "NoAutomaticPrototype";
        break;
    }
  }
  
  std::string to_string(const std::unordered_set<JSClassAttribute>& attributes) noexcept {
    std::string result;
    for (auto attribute : {JSClassAttribute::None, JSClassAttribute::NoAutomaticPrototype}) {
      auto position = attributes.find(attribute);
      if (position != attributes.end()) {
        if (!result.empty()) {
          result += ", ";
        }
        result += to_string(*position);
      }
    }
    
    return result;
  }
  
  std::string to_string_JSClassAttributes(::JSClassAttributes attributes) noexcept {
    return to_string(FromJSClassAttributes(attributes));
  }
  
  JSValue::Type ToJSValueType(JSType type) noexcept {
    switch (type) {
      case kJSTypeUndefined:
        return JSValue::Type::Undefined;
      case kJSTypeNull:
        return JSValue::Type::Null;
      case kJSTypeBoolean:
        return JSValue::Type::Boolean;
      case kJSTypeNumber:
        return JSValue::Type::Number;
      case kJSTypeString:
        return JSValue::Type::String;
      case kJSTypeObject:
        return JSValue::Type::Object;
    }
  }
  
//  std::string to_string(JSValue::Type js_value_type) noexcept {
//    switch (js_value_type) {
//      case JSValue::Type::Undefined:
//        return "Undefined";
//      case JSValue::Type::Null:
//        return "Null";
//      case JSValue::Type::Boolean:
//        return "Boolean";
//      case JSValue::Type::Number:
//        return "Number";
//      case JSValue::Type::String:
//        return "String";
//      case JSValue::Type::Object:
//        return "Object";
//    }
//  }
  
  // The bitwise_cast and to_int32_t code was copied from
  // WebKit/Source/WTF/wtf/StdLibExtras.h and came with these terms and
  // conditions:
  
  /*
   * Copyright (C) 2008 Apple Inc. All Rights Reserved.
   * Copyright (C) 2013 Patrick Gansterer <paroga@paroga.com>
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions
   * are met:
   * 1. Redistributions of source code must retain the above copyright
   *    notice, this list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright
   *    notice, this list of conditions and the following disclaimer in the
   *    documentation and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
   * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
   * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
   * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   */
  
  // C++'s idea of a reinterpret_cast lacks sufficient cojones.
  template<typename ToType, typename FromType>
  inline ToType bitwise_cast(FromType from) {
    static_assert(sizeof(FromType) == sizeof(ToType), "bitwise_cast size of FromType and ToType must be equal!");
    union {
      FromType from;
      ToType to;
    } u;
    u.from = from;
    return u.to;
  }
  
  
  // This in the ToInt32 operation as defined in section 9.5 of the
  // ECMA-262 spec. Note that this operation is identical to ToUInt32
  // other than to interpretation of the resulting bit-pattern (as
  // such this method is also called to implement ToUInt32).
  //
  // The operation can be descibed as round towards zero, then select
  // the 32 least bits of the resulting value in 2s-complement
  // representation.
  int32_t to_int32_t(double number) {
    int64_t bits = bitwise_cast<int64_t>(number);
    int32_t exp = (static_cast<int32_t>(bits >> 52) & 0x7ff) - 0x3ff;
    
    // If exponent < 0 there will be no bits to the left of the
    // decimal point after rounding; if the exponent is > 83 then no
    // bits of precision can be left in the low 32-bit range of the
    // result (IEEE-754 doubles have 52 bits of fractional precision).
    // Note this case handles 0, -0, and all infinite, NaN, & denormal
    // value.
    if (exp < 0 || exp > 83) {
      return 0;
    }
    
    // Select the appropriate 32-bits from the floating point
    // mantissa.  If the exponent is 52 then the bits we need to
    // select are already aligned to the lowest bits of the 64-bit
    // integer representation of tghe number, no need to shift.  If
    // the exponent is greater than 52 we need to shift the value left
    // by (exp - 52), if the value is less than 52 we need to shift
    // right accordingly.
    int32_t result = (exp > 52)
    ? static_cast<int32_t>(bits << (exp - 52))
    : static_cast<int32_t>(bits >> (52 - exp));
    
    // IEEE-754 double precision values are stored omitting an
    // implicit 1 before the decimal point; we need to reinsert this
    // now.  We may also the shifted invalid bits into the result that
    // are not a part of the mantissa (the sign and exponent bits from
    // the floatingpoint representation); mask these out.
    if (exp < 32) {
      int32_t missingOne = 1 << exp;
      result &= missingOne - 1;
      result += missingOne;
    }
    
    // If the input value was negative (we could test either 'number'
    // or 'bits', but testing 'bits' is likely faster) invert the
    // result appropriately.
    return bits < 0 ? -result : result;
  }
  
}} // namespace JavaScriptCoreCPP { namespace detail {
