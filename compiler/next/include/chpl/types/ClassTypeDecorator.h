/*
 * Copyright 2021 Hewlett Packard Enterprise Development LP
 * Other additional copyright holders may be indicated within.
 *
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CHPL_TYPES_CLASS_TYPE_DECORATOR_H
#define CHPL_TYPES_CLASS_TYPE_DECORATOR_H

#include <cassert>
#include <utility>

namespace chpl {
namespace types {


class ClassTypeDecorator final {
 public:
  typedef enum {
    // When updating, make sure that these numbers work with the masks below
    // (last two bits are 0=generic nilable / 1=non-nilable / 2=nilable)
    BORROWED          = 0,
    BORROWED_NONNIL   = 1,
    BORROWED_NILABLE  = 2,
    UNMANAGED         = 4,
    UNMANAGED_NONNIL  = 5,
    UNMANAGED_NILABLE = 6,
    MANAGED           = 8, // owned/shared/etc
    MANAGED_NONNIL    = 9,
    MANAGED_NILABLE   = 10,
    GENERIC           = 12,
    GENERIC_NONNIL    = 13,
    GENERIC_NILABLE   = 14,
  } ClassTypeDecoratorEnum;
  enum {
    NUM_DECORATORS = 12,
    MANAGEMENT_MASK = 0xfc,
    NILABILITY_MASK = 0x03,
  };

  /** Given an integer i with 0 <= i < NUM_DECORATORS
      returns a decorator with that number. This can be used to
      iterate over the possible decorators. */
  static
  ClassTypeDecoratorEnum getIthDecorator(int i) {
    assert(0 <= i && i < NUM_DECORATORS);
    switch (i) {
      case 0:  return BORROWED;
      case 1:  return BORROWED_NONNIL;
      case 2:  return BORROWED_NILABLE;
      case 3:  return UNMANAGED;
      case 4:  return UNMANAGED_NONNIL;
      case 5:  return UNMANAGED_NILABLE;
      case 6:  return MANAGED;
      case 7:  return MANAGED_NONNIL;
      case 8:  return MANAGED_NILABLE;
      case 9:  return GENERIC;
      case 10: return GENERIC_NONNIL;
      case 11: return GENERIC_NILABLE;
    }
    assert(false && "case not handled");
    return BORROWED_NONNIL;
  }
  static
  ClassTypeDecoratorEnum removeNilableFromDecorator(ClassTypeDecoratorEnum d) {
    int tmp = d;
    tmp &= MANAGEMENT_MASK;
    return (ClassTypeDecoratorEnum) tmp;
  }
  static
  ClassTypeDecoratorEnum addNonNilToDecorator(ClassTypeDecoratorEnum d) {
    int tmp = removeNilableFromDecorator(d);
    tmp |= 1;
    return (ClassTypeDecoratorEnum) tmp;
  }
  static
  ClassTypeDecoratorEnum addNilableToDecorator(ClassTypeDecoratorEnum d) {
    int tmp = removeNilableFromDecorator(d);
    tmp |= 2;
    return (ClassTypeDecoratorEnum) tmp;
  }
  static bool isDecoratorUnknownNilability(ClassTypeDecoratorEnum d) {
    return (d & NILABILITY_MASK) == 0;
  }
  static bool isDecoratorNonNilable(ClassTypeDecoratorEnum d) {
    return (d & NILABILITY_MASK) == 1;
  }
  static bool isDecoratorNilable(ClassTypeDecoratorEnum d) {
    return (d & NILABILITY_MASK) == 2;
  }
  static bool isDecoratorBorrowed(ClassTypeDecoratorEnum d) {
    return removeNilableFromDecorator(d) == BORROWED;
  }
  static bool isDecoratorUnmanaged(ClassTypeDecoratorEnum d) {
    return removeNilableFromDecorator(d) == UNMANAGED;
  }
  static bool isDecoratorManaged(ClassTypeDecoratorEnum d) {
    return removeNilableFromDecorator(d) == MANAGED;
  }
  static bool isDecoratorUnknownManagement(ClassTypeDecoratorEnum d) {
    return removeNilableFromDecorator(d) == GENERIC;
  }

  // Information from the formalDecorator will be used,
  // but if it is generic and actualDecorator has a non-generic value,
  // the non-generic component will be combined in.
  static ClassTypeDecoratorEnum
  combineDecorators(ClassTypeDecoratorEnum formalDecorator,
                    ClassTypeDecoratorEnum actualDecorator);

 private:
  ClassTypeDecoratorEnum val_;

 public:
  explicit ClassTypeDecorator(ClassTypeDecoratorEnum val) : val_(val) { }
  ~ClassTypeDecorator() = default;

  /** Returns the ClassTypeDecoratorEnum value */
  ClassTypeDecoratorEnum val() const { return val_; }

  /** Returns a decorator based on this one but with unknown nilability */
  ClassTypeDecorator removeNilable() const {
    return ClassTypeDecorator(removeNilableFromDecorator(val_));
  }
  /** Returns a decorator based on this one but that is non-nilable */
  ClassTypeDecorator addNonNil() const {
    return ClassTypeDecorator(addNonNilToDecorator(val_));
  }
  /** Returns a decorator based on this one but that is nilable */
  ClassTypeDecorator addNilable() const {
    return ClassTypeDecorator(addNilableToDecorator(val_));
  }
  /** Returns true if this decorator has unknown nilability */
  bool isUnknownNilability() const {
    return isDecoratorUnknownNilability(val_);
  }
  /** Returns true if this decorator is non-nilable */
  bool isNonNilable() const {
    return isDecoratorNonNilable(val_);
  }
  /** Returns true if this decorator is nilable */
  bool isNilable() const {
    return isDecoratorNilable(val_);
  }
  /** Returns true if this decorator represents a borrowed type
      (e.g. borrowed C) */
  bool isBorrowed() const {
    return isDecoratorBorrowed(val_);
  }
  /** Returns true if this decorator represents a unmanaged type
      (e.g. unmanaged C) */
  bool isUnmanaged() const {
    return isDecoratorUnmanaged(val_);
  }
  /** Returns true if this decorator represents a managed type
      (e.g. owned or shared) */
  bool isManaged() const {
    return isDecoratorManaged(val_);
  }
  /** Returns true if this decorator has unknown management */
  bool isUnknownManagement() const {
    return isDecoratorUnknownManagement(val_);
  }

  /** Combines decorators assuming that `this` is a formal
      and the passed decorator is an actual */
  ClassTypeDecorator combine(ClassTypeDecorator actual) const {
    return ClassTypeDecorator(combineDecorators(val_, actual.val_));
  }

  bool operator==(ClassTypeDecorator other) const {
    return this->val_ == other.val_;
  }
  bool operator!=(ClassTypeDecorator other) const {
    return !(*this == other);
  }
  void swap(ClassTypeDecorator other) {
    std::swap(this->val_, other.val_);
  }
};


} // end namespace uast
} // end namespace chpl

#endif
