// -*- mode: c++ -*-
//
//  Author: Matt Langston
//  Copyright (c) 2014 Appcelerator. All rights reserved.
//

#include "JavaScriptCoreCPP/JSValue.h"
#include <cassert>

std::atomic<long> JSValue::ctorCounter_ { 0 };
std::atomic<long> JSValue::dtorCounter_ { 0 };

/*
 * C++'s idea of a reinterpret_cast lacks sufficient cojones.
 */
template<typename ToType, typename FromType>
inline ToType bitwise_cast(FromType from)
{
  static_assert(sizeof(FromType) == sizeof(ToType), "bitwise_cast size of FromType and ToType must be equal!");
  union {
    FromType from;
    ToType to;
  } u;
  u.from = from;
  return u.to;
}

JSValue_ptr_t
JSValue::valueWithNewRegularExpressionFromPatternAndFlagsInContext(const std::string& pattern, const std::string& flags, const JSContext_ptr_t& context_ptr) {
  JSString patternString { pattern };
  JSString flagsString { flags };
  JSValueRef arguments[2] = { JSValueMakeString(static_cast<::JSGlobalContextRef>(*context_ptr), JSStringRef(patternString)), JSValueMakeString(static_cast<::JSGlobalContextRef>(*context_ptr), JSStringRef(flagsString)) };
  // TODO: should the exception really be a nullptr?
  return JSValue::create(JSObjectMakeRegExp(static_cast<::JSGlobalContextRef>(*context_ptr), 2, arguments, nullptr), context_ptr);
}

JSValue::JSValue(::JSValueRef value, const JSContext_ptr_t& context_ptr) :
    value_(value),
    context_ptr_(context_ptr)
{
  std::clog << "JSValue: ctor called (JSValueRef, JSContext_ptr_t)" << std::endl;
  JSValueProtect(static_cast<::JSGlobalContextRef>(*context_ptr_), value_);
  ++ctorCounter_;
}

JSValue::~JSValue() {
  std::clog << "JSValue: dtor called" << std::endl;
  JSValueUnprotect(static_cast<::JSGlobalContextRef>(*context_ptr_), value_);
  ++dtorCounter_;
}

// Create a copy of another JSValue.
JSValue::JSValue(const JSValue& rhs) :
    JSValue(rhs.value_, rhs.context_ptr_)
{
  std::clog << "JSValue: copy ctor called" << std::endl;
}

// Create a copy of another JSValue by assignment.
JSValue& JSValue::operator=(const JSValue& rhs) {
  std::clog << "JSValue: operator= called" << std::endl;
    
  if ( this == &rhs ) {
    return *this;
  }
    
  // Release the resource we are replacing.
  JSValueUnprotect(static_cast<::JSGlobalContextRef>(*context_ptr_), value_);
    
  value_       = rhs.value_;
  context_ptr_ = rhs.context_ptr_;
    
  // Retain the resource we copying.
  JSValueProtect(static_cast<::JSGlobalContextRef>(*context_ptr_), value_);
    
  return *this;
}

JSValue::operator double() const {
  ::JSValueRef exception = 0;
  double result = JSValueToNumber(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return std::numeric_limits<double>::quiet_NaN();
  }
    
  return result;
}

// This in the ToInt32 operation is defined in section 9.5 of the ECMA-262 spec.
// Note that this operation is identical to ToUInt32 other than to interpretation
// of the resulting bit-pattern (as such this method is also called to implement
// ToUInt32).
//
// The operation can be descibed as round towards zero, then select the 32 least
// bits of the resulting value in 2s-complement representation.
JSValue::operator int32_t() const
{
  const double number(double(*this));
  int64_t bits = bitwise_cast<int64_t>(number);
  int32_t exp = (static_cast<int32_t>(bits >> 52) & 0x7ff) - 0x3ff;
    
  // If exponent < 0 there will be no bits to the left of the decimal point
  // after rounding; if the exponent is > 83 then no bits of precision can be
  // left in the low 32-bit range of the result (IEEE-754 doubles have 52 bits
  // of fractional precision).
  // Note this case handles 0, -0, and all infinite, NaN, & denormal value.
  if (exp < 0 || exp > 83)
    return 0;
    
  // Select the appropriate 32-bits from the floating point mantissa.  If the
  // exponent is 52 then the bits we need to select are already aligned to the
  // lowest bits of the 64-bit integer representation of tghe number, no need
  // to shift.  If the exponent is greater than 52 we need to shift the value
  // left by (exp - 52), if the value is less than 52 we need to shift right
  // accordingly.
  int32_t result = (exp > 52)
                   ? static_cast<int32_t>(bits << (exp - 52))
                   : static_cast<int32_t>(bits >> (52 - exp));
    
  // IEEE-754 double precision values are stored omitting an implicit 1 before
  // the decimal point; we need to reinsert this now.  We may also the shifted
  // invalid bits into the result that are not a part of the mantissa (the sign
  // and exponent bits from the floatingpoint representation); mask these out.
  if (exp < 32) {
    int32_t missingOne = 1 << exp;
    result &= missingOne - 1;
    result += missingOne;
  }
    
  // If the input value was negative (we could test either 'number' or 'bits',
  // but testing 'bits' is likely faster) invert the result appropriately.
  return bits < 0 ? -result : result;
}

JSValue::operator JSString() const {
  ::JSValueRef exception = 0;
  ::JSStringRef result = JSValueToStringCopy(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
        
    // Return the empty string.
    return JSString();
  }
    
  return JSString(result);
}

JSValue_ptr_t JSValue::valueForProperty(const std::string& property) const {
  JSValueRef exception = 0;
  JSObjectRef object = JSValueToObject(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    return context_ptr_ -> valueFromNotifyException(exception);
  }
  
  auto propertyName = JSString(property);
  JSValueRef result = JSObjectGetProperty(static_cast<JSGlobalContextRef>(*context_ptr_), object, static_cast<JSStringRef>(propertyName), &exception);
  if (exception) {
    return context_ptr_ -> valueFromNotifyException(exception);
  }
  
  return JSValue::create(result, context_ptr_);
}

void JSValue::setValueForProperty(const JSValue_ptr_t& value_ptr, const std::string& property) {
  JSValueRef exception = 0;
  JSObjectRef object = JSValueToObject(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return;
  }
  
  auto propertyName = JSString(property);
  JSObjectSetProperty(static_cast<JSGlobalContextRef>(*context_ptr_), object, static_cast<JSStringRef>(propertyName), value_ptr -> value_, 0, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return;
  }
}

bool JSValue::deleteProperty(const std::string& property) {
  JSValueRef exception = 0;
  JSObjectRef object = JSValueToObject(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return false;
  }
  
  auto propertyName = JSString(property);
  bool result = JSObjectDeleteProperty(static_cast<JSGlobalContextRef>(*context_ptr_), object, static_cast<JSStringRef>(propertyName), &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return false;
  }
  
  return result;
}

bool JSValue::hasProperty(const std::string& property) const {
  JSValueRef exception = 0;
  JSObjectRef object = JSValueToObject(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return false;
  }
  
  auto propertyName = JSString(property);
  return JSObjectHasProperty(static_cast<JSGlobalContextRef>(*context_ptr_), object, static_cast<JSStringRef>(propertyName));
}

JSValue_ptr_t JSValue::valueAtIndex(size_t index) const {
  JSValueRef exception = 0;
  JSObjectRef object = JSValueToObject(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    return context_ptr_ -> valueFromNotifyException(exception);
  }
  
  JSValueRef result = JSObjectGetPropertyAtIndex(static_cast<JSGlobalContextRef>(*context_ptr_), object, static_cast<unsigned int>(index), &exception);
  if (exception) {
    return context_ptr_ -> valueFromNotifyException(exception);
  }
  
  return JSValue::create(result, context_ptr_);
}

void JSValue::setValueAtIndex(const JSValue_ptr_t& value_ptr, size_t index) {
  JSValueRef exception = 0;
  JSObjectRef object = JSValueToObject(static_cast<::JSGlobalContextRef>(*context_ptr_), value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return;
  }
  
  JSObjectSetPropertyAtIndex(static_cast<JSGlobalContextRef>(*context_ptr_), object, static_cast<unsigned int>(index), value_ptr -> value_, &exception);
  if (exception) {
    context_ptr_ -> notifyException(exception);
    return;
  }
}

