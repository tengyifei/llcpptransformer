// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSROOT_ZIRCON_FIDL_H_
#define SYSROOT_ZIRCON_FIDL_H_

#include <cstddef>
#include <stdint.h>

#if !defined(__cplusplus)
#if defined(__GNUC__) && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40800
// |alignas| and |alignof| were added in C11. GCC added support in version 4.8.
// Testing for __STDC_VERSION__/__cplusplus doesn't work because 4.7 already
// reports support for C11.
#define alignas(x) __attribute__ ((aligned (x)))
#define alignof(x) __alignof__ (x)
#elif defined(_MSC_VER)
#define alignas(x) __declspec(align(x))
#define alignof __alignof
#else
#include <stdalign.h>
#endif
#endif

#define add_overflow(a, b, c) __builtin_add_overflow(a, b, c)

typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

typedef int32_t zx_status_t;

#define ZX_CHANNEL_MAX_MSG_BYTES            ((uint32_t)65536u)

#define ZX_OK (0)
#define ZX_ERR_BAD_STATE (-20)
#define ZX_ERR_INVALID_ARGS (-10)
#define ZX_ERR_BUFFER_TOO_SMALL (-789)

#define FIDL_MAX_SIZE UINT32_MAX

#define ZX_OBJ_TYPE_NONE            ((uint32_t)0u)

// Out of line allocations.

// The fidl wire format represents potential out-of-line allocations
// (corresponding to actual pointer types in the C format) as
// uintptr_t. For allocations that are actually present and that will
// be patched up with pointers during decoding, the FIDL_ALLOC_PRESENT
// value is used. For non-present nullable allocations, the
// FIDL_ALLOC_ABSENT value is used.

#define FIDL_ALLOC_PRESENT ((uintptr_t)UINTPTR_MAX)
#define FIDL_ALLOC_ABSENT ((uintptr_t)0)

#define FIDL_HANDLE_ABSENT (0u)
#define FIDL_HANDLE_PRESENT (UINT32_MAX)

// Out of line allocations are all 8 byte aligned.
#define FIDL_ALIGNMENT ((size_t)8)
#define FIDL_ALIGN(a) (((a) + 7) & ~7u)
#define FIDL_ALIGNDECL alignas(FIDL_ALIGNMENT)

// An opaque struct representing the encoding of a particular fidl
// type.
typedef struct fidl_type fidl_type_t;

// Primitive types.

// Both on the wire and once deserialized, primitive fidl types
// correspond directly to C types. There is no intermediate layer of
// typedefs. For instance, fidl's float64 is generated as double.

// All primitive types are non-nullable.

// All primitive types are naturally sized and aligned on the wire.

// fidl     C         Meaning.
// ---------------------------------------------
// bool     bool      A boolean.
// int8     int8_t    An 8 bit signed integer.
// int16    int16_t   A 16 bit signed integer.
// int32    int32_t   A 32 bit signed integer.
// int64    int64_t   A 64 bit signed integer.
// uint8    uint8_t   An 8 bit unsigned integer.
// uint16   uint16_t  A 16 bit unsigned integer.
// uint32   uint32_t  A 32 bit unsigned integer.
// uint64   uint64_t  A 64 bit unsigned integer.
// float32  float     A 32 bit IEEE-754 float.
// float64  double    A 64 bit IEEE-754 float.

// Enums.

// Fidl enums have an undering integer type (one of int8, int16,
// int32, int64, uint8, uint16, uint32, or uint64). The wire format of
// an enum and the C format of an enum are the same as the
// corresponding primitive type.

// String types.

// Fidl strings are variable-length UTF-8 strings. Strings can be
// nullable (string?) or nonnullable (string); if nullable, the null
// string is distinct from the empty string. Strings can be bounded to
// a fixed byte length (e.g. string:40? is a nullable string of at
// most 40 bytes).

// Strings are not guaranteed to be nul terminated. Strings can
// contain embedded nuls throughout their length.

// The fidl wire format dictates that strings are valid UTF-8. It is
// up to clients to provide well-formed UTF-8 and servers to check for
// it. Message encoding and decoding can, but does not by default,
// perform this check.

// All deserialized string types are represented by the fidl_string_t
// structure. This structure consists of a size (in bytes) and a
// pointer to an out-of-line allocation of uint8_t, guaranteed to be
// at least as long as the length.

// The bound on a string type is not present in the serialized format,
// but is checked as part of validation.

typedef struct fidl_string {
  // Number of UTF-8 code units (bytes), must be 0 if |data| is null.
  uint64_t size;

  // Pointer to UTF-8 code units (bytes) or null
  char* data;
} fidl_string_t;

// When encoded, an absent nullable string is represented as a
// fidl_string_t with size 0 and FIDL_ALLOC_ABSENT data, with no
// out-of-line allocation associated with it. A present string
// (nullable or not) is represented as a fidl_string_t with some size
// and with data equal to FIDL_ALLOC_PRESENT, which the decoding
// process replaces with an actual pointer to the next out-of-line
// allocation.

// All string types:

// fidl       C              Meaning
// -----------------------------------------------------------------
// string     fidl_string_t  A string of arbitrary length.
// string?    fidl_string_t  An optional string of arbitrary length.
// string:N   fidl_string_t  A string up to N bytes long.
// string:N?  fidl_string_t  An optional string up to N bytes long.

// Arrays.

// On the wire, an array of N objects of type T (array<T, N>) is
// represented the same as N contiguous Ts. Equivalently, it is
// represented the same as a nonnullable struct containing N fields
// all of type T.

// In C, this is just represented as a C array of the corresponding C
// type.

// Vector types.

// Fidl vectors are variable-length arrays of a given type T. Vectors
// can be nullable (vector<T>?) or nonnullable (vector<T>); if
// nullable, the null vector is distinct from the empty
// vector. Vectors can be bounded to a fixed element length
// (e.g. vector<T>:40? is a nullable vector of at most 40 Ts).

// All deserialized vector types are represented by the fidl_vector_t
// structure. This structure consists of a count and a pointer to the
// bytes.

// The bound on a vector type is not present in the serialized format,
// but is checked as part of validation.

typedef struct fidl_vector {
  // Number of elements, must be 0 if |data| is null.
  uint64_t count;

  // Pointer to element data or null.
  void* data;
} fidl_vector_t;

// When encoded, an absent nullable vector is represented as a
// fidl_vector_t with size 0 and FIDL_ALLOC_ABSENT data, with no
// out-of-line allocation associated with it. A present vector
// (nullable or not) is represented as a fidl_vector_t with some size
// and with data equal to FIDL_ALLOC_PRESENT, which the decoding
// process replaces with an actual pointer to the next out-of-line
// allocation.

// All vector types:

// fidl          C              Meaning
// --------------------------------------------------------------------------
// vector<T>     fidl_vector_t  A vector of T, of arbitrary length.
// vector<T>?    fidl_vector_t  An optional vector of T, of arbitrary length.
// vector<T>:N   fidl_vector_t  A vector of T, up to N elements.
// vector<T>:N?  fidl_vector_t  An optional vector of T,  up to N elements.

// Envelope.

// An efficient way to encapsulate uninterpreted FIDL messages.
// - Stores a variable size uninterpreted payload out-of-line.
// - Payload may contain an arbitrary number of bytes and handles.
// - Allows for encapsulation of one FIDL message inside of another.
// - Building block for extensible structures such as tables & extensible
//   unions.

// When encoded for transfer, |data| indicates presence of content:
// - FIDL_ALLOC_ABSENT : envelope is null
// - FIDL_ALLOC_PRESENT : envelope is non-null, |data| is the next out-of-line object
// When decoded for consumption, |data| is a pointer to content.
// - nullptr : envelope is null
// - <valid pointer> : envelope is non-null, |data| is at indicated memory address

typedef struct {
  // The size of the entire envelope contents, including any additional
  // out-of-line objects that the envelope may contain. For example, a
  // vector<string>'s num_bytes for ["hello", "world"] would include the
  // string contents in the size, not just the outer vector. Always a multiple
  // of 8; must be zero if envelope is null.
  uint32_t num_bytes;

  // The number of handles in the envelope, including any additional
  // out-of-line objects that the envelope contains. Must be zero if envelope is null.
  uint32_t num_handles;

  // A pointer to the out-of-line envelope data in decoded form, or
  // FIDL_ALLOC_(ABSENT|PRESENT) in encoded form.
  union {
    void* data;
    uintptr_t presence;
  };
} fidl_envelope_t;

// Handle types.

// Handle types are encoded directly. Just like primitive types, there
// is no fidl-specific handle type. Generated fidl structures simply
// mention zx_handle_t.

// Handle types are either nullable (handle?), or not (handle); and
// either explicitly typed (e.g. handle<Channel> or handle<Job>), or
// not.

// All fidl handle types, regardless of subtype, are represented as
// zx_handle_t. The encoding tables do know the handle subtypes,
// however, for clients which wish to perform explicit checking.

// The following are the possible handle subtypes.

// process
// thread
// vmo
// channel
// event
// port
// interrupt
// iomap
// pci
// log
// socket
// resource
// eventpair
// job
// vmar
// fifo
// hypervisor
// guest
// timer

// All handle types are 4 byte sized and aligned on the wire.

// When encoded, absent nullable handles are represented as
// FIDL_HANDLE_ABSENT. Present handles, whether nullable or not, are
// represented as FIDL_HANDLE_PRESENT, which the decoding process will
// overwrite with the next handle value in the channel message.

// #define FIDL_HANDLE_ABSENT ((zx_handle_t)ZX_HANDLE_INVALID)
// #define FIDL_HANDLE_PRESENT ((zx_handle_t)UINT32_MAX)

// fidl        C            Meaning
// ------------------------------------------------------------------
// handle      zx_handle_t  Any valid handle.
// handle?     zx_handle_t  Any valid handle, or ZX_HANDLE_INVALID.
// handle<T>   zx_handle_t  Any valid T handle.
// handle<T>?  zx_handle_t  Any valid T handle, or ZX_HANDLE_INVALID.

// Unions.

// Fidl unions are a tagged sum type. The tag is a 4 bytes. For every
// union type, the fidl compiler generates an enum representing the
// different variants of the enum. This is followed, in C and on the
// wire, by large enough and aligned enough storage for all members of
// the union.

// Unions may be nullable. Nullable unions are represented as a
// pointer to an out of line allocation of tag-and-member. As with
// other out-of-line allocations, ones present on the wire take the
// value FIDL_ALLOC_PRESENT and those that are not are represented by
// FIDL_ALLOC_NULL. Nonnullable unions are represented inline as a
// tag-and-member.

// For each fidl union type, a corresponding C type is generated. They
// are all structs consisting of a fidl_union_tag_t discriminant,
// followed by an anonymous union of all the union members.

typedef uint32_t fidl_union_tag_t;

// fidl                 C                            Meaning
// --------------------------------------------------------------------
// union foo {...}      struct union_foo {           An inline union.
//                          fidl_union_tag_t tag;
//                          union {...};
//                      }
//
// union foo {...}?     struct union_foo*            A pointer to a
//                                                   union_foo, or else
//                                                   FIDL_ALLOC_ABSENT.

// Tables.

// Tables are 'flexible structs', where all members are optional, and new
// members can be added, or old members removed while preserving ABI
// compatibility. Each table member is referenced by ordinal, sequentially
// assigned from 1 onward, with no gaps. Each member content is stored
// out-of-line in an envelope, and a table is simply a vector of these envelopes
// with the requirement that the last envelope must be present in order
// to guarantee a canonical representation.

typedef struct {
  fidl_vector_t envelopes;
} fidl_table_t;

// Extensible unions.

// Extensible unions, or "xunions" (colloquially pronounced "zoo-nions") are
// similar to unions, except that storage for union members are out-of-line
// rather than inline. This enables union members to be added and removed while
// preserving ABI compatibility with the existing xunion definition. Like
// unions, xunions have a 4-byte tag, and may be nullable.

typedef uint32_t fidl_xunion_tag_t;

enum {
  kFidlXUnionEmptyTag = 0,  // The tag representing an empty xunion.
};

typedef struct {
  fidl_xunion_tag_t tag;
  uint32_t padding;  // Should always be zero.
  fidl_envelope_t envelope;
} fidl_xunion_t;

// Messages.

// All fidl messages share a common 16 byte header.

enum {
  kFidlWireFormatMagicNumberInitial = 1,
};

// typedef struct fidl_message_header {
//   zx_txid_t txid;
//   uint8_t flags[3];
//   // This value indicates the message's wire format. Two sides with different
//   // wire formats are incompatible with each other
//   uint8_t magic_number;
//   uint64_t ordinal;
// } fidl_message_header_t;

// Messages which do not have a response use zero as a special
// transaction id.

#define FIDL_TXID_NO_RESPONSE 0ul

// // A FIDL message.
// typedef struct fidl_msg {
//   // The bytes of the message.
//   //
//   // The bytes of the message might be in the encoded or decoded form.
//   // Functions that take a |fidl_msg_t| as an argument should document whether
//   // the expect encoded or decoded messages.
//   //
//   // See |num_bytes| for the number of bytes in the message.
//   void* bytes;

//   // The handles of the message.
//   //
//   // See |num_bytes| for the number of bytes in the message.
//   zx_handle_t* handles;

//   // The number of bytes in |bytes|.
//   uint32_t num_bytes;

//   // The number of handles in |handles|.
//   uint32_t num_handles;
// } fidl_msg_t;

// // An outstanding FIDL transaction.
// typedef struct fidl_txn fidl_txn_t;
// struct fidl_txn {
//   // Replies to the outstanding request and complete the FIDL transaction.
//   //
//   // Pass the |fidl_txn_t| object itself as the first parameter. The |msg|
//   // should already be encoded. This function always consumes any handles
//   // present in |msg|.
//   //
//   // Call |reply| only once for each |txn| object. After |reply| returns, the
//   // |txn| object is considered invalid and might have been freed or reused
//   // for another purpose.
//   zx_status_t (*reply)(fidl_txn_t* txn, const fidl_msg_t* msg);
// };

// An epitaph is a message that a server sends just prior to closing the
// connection.  It provides an indication of why the connection is being closed.
// Epitaphs are defined in the FIDL wire format specification.  Once sent down
// the wire, the channel should be closed.
// typedef struct fidl_epitaph {
//   FIDL_ALIGNDECL

//   // The method ordinal for all epitaphs must be kFidlOrdinalEpitaph
//   fidl_message_header_t hdr;

//   // The error associated with this epitaph is stored as a struct{int32} in
//   // the message payload. System errors must be constants of type zx_status_t,
//   // which are all negative. Positive numbers should be used for application
//   // errors. A value of ZX_OK indicates no error.
//   zx_status_t error;
// } fidl_epitaph_t;

// This ordinal value is reserved for Epitaphs.
enum {
  kFidlOrdinalEpitaph = 0xFFFFFFFF,
};

// Assumptions.

// Ensure that FIDL_ALIGNMENT is sufficient.
// static_assert(alignof(bool) <= FIDL_ALIGNMENT && "");
// static_assert(alignof(int8_t) <= FIDL_ALIGNMENT && "");
// static_assert(alignof(int16_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(int32_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(int64_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(uint8_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(uint16_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(uint32_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(uint64_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(float) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(double) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(void*) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(fidl_union_tag_t) <= FIDL_ALIGNMENT, "");
// static_assert(alignof(fidl_message_header_t) <= FIDL_ALIGNMENT, "");

// __END_CDECLS

namespace fidl {

enum FidlNullability : bool {
  kNonnullable = false,
  kNullable = true,
};

enum FidlStrictness : bool {
  kFlexible = false,
  kStrict = true,
};

constexpr inline uint64_t FidlAlign(uint32_t offset) {
  constexpr uint64_t alignment_mask = FIDL_ALIGNMENT - 1;
  return (offset + alignment_mask) & ~alignment_mask;
}

// Determine if the pointer is aligned to |FIDL_ALIGNMENT|.
inline bool IsAligned(const uint8_t* ptr) {
  auto uintptr = reinterpret_cast<uintptr_t>(ptr);
  constexpr uintptr_t kAlignment = FIDL_ALIGNMENT;
  return uintptr % kAlignment == 0;
}

// Add |size| to out-of-line |offset|, maintaining alignment. For example, a pointer to a struct
// that is 4 bytes still needs to advance the next out-of-line offset by 8 to maintain
// the aligned-to-FIDL_ALIGNMENT property.
// Returns false on overflow. Otherwise, resulting offset is stored in |out_offset|.
inline bool AddOutOfLine(uint32_t offset, uint32_t size, uint32_t* out_offset) {
  constexpr uint32_t kMask = FIDL_ALIGNMENT - 1;
  uint32_t new_offset = offset;
  if (add_overflow(new_offset, size, &new_offset) || add_overflow(new_offset, kMask, &new_offset)) {
    return false;
  }
  new_offset &= ~kMask;
  *out_offset = new_offset;
  return true;
}

struct FidlStructField {
  const fidl_type* type;
  union {
    uint32_t offset;  // If |type| is non-null (i.e. this field is a non-primitive type)
    uint32_t padding_offset;  // If |type| is null (i.e. this field is a primitive type)
  };
  uint8_t padding;
  const FidlStructField* alt_field;

  constexpr FidlStructField(const fidl_type* type, uint32_t offset, uint8_t padding, const FidlStructField* alt_field = nullptr)
      : type(type), offset(offset), padding(padding), alt_field(alt_field) {}
};

struct FidlUnionField {
  const fidl_type* type;
  uint32_t padding;
  uint32_t xunion_ordinal;

  constexpr FidlUnionField(const fidl_type* type, uint32_t padding)
      : type(type), padding(padding), xunion_ordinal(0) {}

  constexpr FidlUnionField(const fidl_type* type, uint32_t padding, uint32_t xunion_ordinal)
      : type(type), padding(padding), xunion_ordinal(xunion_ordinal) {}
};

struct FidlTableField {
  const fidl_type* type;
  uint32_t ordinal;

  constexpr FidlTableField(const fidl_type* type, uint32_t ordinal)
      : type(type), ordinal(ordinal) {}
};

struct FidlXUnionField {
  const fidl_type* type;
  uint32_t ordinal;

  constexpr FidlXUnionField(const fidl_type* type, uint32_t ordinal)
      : type(type), ordinal(ordinal) {}
};

enum FidlTypeTag : uint32_t {
  kFidlTypePrimitive,
  kFidlTypeEnum,
  kFidlTypeBits,
  kFidlTypeStruct,
  kFidlTypeStructPointer,
  kFidlTypeUnion,
  kFidlTypeUnionPointer,
  kFidlTypeArray,
  kFidlTypeString,
  kFidlTypeHandle,
  kFidlTypeVector,
  kFidlTypeTable,
  kFidlTypeXUnion,
};

enum struct FidlCodedPrimitive : uint32_t {
  kBool,
  kInt8,
  kInt16,
  kInt32,
  kInt64,
  kUint8,
  kUint16,
  kUint32,
  kUint64,
  kFloat32,
  kFloat64,
};

typedef bool (*EnumValidationPredicate)(uint64_t);

struct FidlCodedEnum {
  const FidlCodedPrimitive underlying_type;
  const EnumValidationPredicate validate;
  const char* name;  // may be nullptr if omitted at compile time

  constexpr explicit FidlCodedEnum(FidlCodedPrimitive underlying_type,
                                   EnumValidationPredicate validate, const char* name)
      : underlying_type(underlying_type), validate(validate), name(name) {}
};

struct FidlCodedBits {
  const FidlCodedPrimitive underlying_type;
  const uint64_t mask;
  const char* name;  // may be nullptr if omitted at compile time

  constexpr explicit FidlCodedBits(FidlCodedPrimitive underlying_type, uint64_t mask,
                                   const char* name)
      : underlying_type(underlying_type), mask(mask), name(name) {}
};

// Though the |size| is implied by the fields, computing that information is not the purview of this
// library. It's easier for the compiler to stash it.
struct FidlCodedStruct {
  const FidlStructField* const fields;
  const uint32_t field_count;
  const uint32_t size;
  const char* name;  // may be nullptr if omitted at compile time
  const FidlCodedStruct* alt_type;

  constexpr FidlCodedStruct(const FidlStructField* fields, uint32_t field_count, uint32_t size,
                            const char* name, const FidlCodedStruct* alt_type = nullptr)
      : fields(fields), field_count(field_count), size(size), name(name), alt_type(alt_type) {}
};

struct FidlCodedStructPointer {
  const FidlCodedStruct* const struct_type;

  constexpr explicit FidlCodedStructPointer(const FidlCodedStruct* struct_type)
      : struct_type(struct_type) {}
};

struct FidlCodedTable {
  const FidlTableField* const fields;
  const uint32_t field_count;
  const char* name;  // may be nullptr if omitted at compile time

  constexpr FidlCodedTable(const FidlTableField* fields, uint32_t field_count, const char* name)
      : fields(fields), field_count(field_count), name(name) {}
};

struct FidlCodedXUnion;

// On-the-wire unions begin with a tag which is an index into |fields|.
// |data_offset| is the offset of the data in the wire format (tag + padding).
struct FidlCodedUnion {
  const FidlUnionField* const fields;
  const uint32_t field_count;
  const uint32_t data_offset;
  const uint32_t size;
  const char* name;  // may be nullptr if omitted at compile time
  const FidlCodedUnion* alt_type;

  constexpr FidlCodedUnion(const FidlUnionField* const fields, uint32_t field_count,
                           uint32_t data_offset, uint32_t size, const char* name,
                           const FidlCodedUnion* alt_type = nullptr)
      : fields(fields),
        field_count(field_count),
        data_offset(data_offset),
        size(size),
        name(name),
        alt_type(alt_type) {}
};

struct FidlCodedUnionPointer {
  const FidlCodedUnion* const union_type;

  constexpr explicit FidlCodedUnionPointer(const FidlCodedUnion* union_type)
      : union_type(union_type) {}
};

struct FidlCodedXUnion {
  const uint32_t field_count;
  const FidlXUnionField* const fields;
  const FidlNullability nullable;
  const char* name;  // may be nullptr if omitted at compile time
  const FidlStrictness strictness;

  constexpr FidlCodedXUnion(uint32_t field_count, const FidlXUnionField* fields,
                            FidlNullability nullable, const char* name, FidlStrictness strictness)
      : field_count(field_count),
        fields(fields),
        nullable(nullable),
        name(name),
        strictness(strictness) {}
};

// An array is essentially a struct with |array_size / element_size| of the same field, named at
// |element|.
struct FidlCodedArray {
  const fidl_type* const element;
  const uint32_t array_size;
  const uint32_t element_size;
  const FidlCodedArray* alt_type;

  constexpr FidlCodedArray(const fidl_type* element, uint32_t array_size, uint32_t element_size, const FidlCodedArray* alt_type)
      : element(element), array_size(array_size), element_size(element_size), alt_type(alt_type) {}
};

// TODO(fxb/???): Switch to using this more ergonomic coding table.
struct FidlCodedArrayNew {
  const fidl_type* const element;
  const uint32_t element_count;
  const uint32_t element_size;
  const uint32_t element_padding;
  const FidlCodedArrayNew* alt_type;

  constexpr FidlCodedArrayNew(const fidl_type* element, uint32_t element_count,
                              uint32_t element_size, uint32_t element_padding,
                              const FidlCodedArrayNew* alt_type)
      : element(element), element_count(element_count),
        element_size(element_size), element_padding(element_padding),
        alt_type(alt_type) {}
};

// Note: must keep in sync with fidlc types.h HandleSubtype.
// enum FidlHandleSubtype : zx_obj_type_t {
//   // special case to indicate subtype is not specified.
//   kFidlHandleSubtypeHandle = ZX_OBJ_TYPE_NONE,

//   kFidlHandleSubtypeBti = ZX_OBJ_TYPE_BTI,
//   kFidlHandleSubtypeChannel = ZX_OBJ_TYPE_CHANNEL,
//   kFidlHandleSubtypeEvent = ZX_OBJ_TYPE_EVENT,
//   kFidlHandleSubtypeEventpair = ZX_OBJ_TYPE_EVENTPAIR,
//   kFidlHandleSubtypeException = ZX_OBJ_TYPE_EXCEPTION,
//   kFidlHandleSubtypeFifo = ZX_OBJ_TYPE_FIFO,
//   kFidlHandleSubtypeGuest = ZX_OBJ_TYPE_GUEST,
//   kFidlHandleSubtypeInterrupt = ZX_OBJ_TYPE_INTERRUPT,
//   kFidlHandleSubtypeIommu = ZX_OBJ_TYPE_IOMMU,
//   kFidlHandleSubtypeJob = ZX_OBJ_TYPE_JOB,
//   kFidlHandleSubtypeLog = ZX_OBJ_TYPE_LOG,
//   kFidlHandleSubtypePager = ZX_OBJ_TYPE_PAGER,
//   kFidlHandleSubtypePciDevice = ZX_OBJ_TYPE_PCI_DEVICE,
//   kFidlHandleSubtypePmt = ZX_OBJ_TYPE_PMT,
//   kFidlHandleSubtypePort = ZX_OBJ_TYPE_PORT,
//   kFidlHandleSubtypeProcess = ZX_OBJ_TYPE_PROCESS,
//   kFidlHandleSubtypeProfile = ZX_OBJ_TYPE_PROFILE,
//   kFidlHandleSubtypeResource = ZX_OBJ_TYPE_RESOURCE,
//   kFidlHandleSubtypeSocket = ZX_OBJ_TYPE_SOCKET,
//   kFidlHandleSubtypeSuspendToken = ZX_OBJ_TYPE_SUSPEND_TOKEN,
//   kFidlHandleSubtypeThread = ZX_OBJ_TYPE_THREAD,
//   kFidlHandleSubtypeTimer = ZX_OBJ_TYPE_TIMER,
//   kFidlHandleSubtypeVcpu = ZX_OBJ_TYPE_VCPU,
//   kFidlHandleSubtypeVmar = ZX_OBJ_TYPE_VMAR,
//   kFidlHandleSubtypeVmo = ZX_OBJ_TYPE_VMO,
// };

struct FidlCodedHandle {
  const uint32_t handle_subtype;
  const FidlNullability nullable;

  constexpr FidlCodedHandle(uint32_t handle_subtype, FidlNullability nullable)
      : handle_subtype(handle_subtype), nullable(nullable) {}
};

struct FidlCodedString {
  const uint32_t max_size;
  const FidlNullability nullable;

  constexpr FidlCodedString(uint32_t max_size, FidlNullability nullable)
      : max_size(max_size), nullable(nullable) {}
};

// Note that |max_count * element_size| is guaranteed to fit into a uint32_t. Unlike other types,
// the |element| pointer may be null. This occurs when the element type contains no interesting bits
// (i.e. pointers or handles).
struct FidlCodedVector {
  const fidl_type* const element;
  const uint32_t max_count;
  const uint32_t element_size;
  const FidlNullability nullable;
  const FidlCodedVector* alt_type;

  constexpr FidlCodedVector(const fidl_type* element, uint32_t max_count, uint32_t element_size,
                            FidlNullability nullable, const FidlCodedVector* alt_type)
      : element(element), max_count(max_count), element_size(element_size), nullable(nullable),
        alt_type(alt_type) {}
};

}  // namespace fidl

struct fidl_type {
  const fidl::FidlTypeTag type_tag;
  union {
    const fidl::FidlCodedPrimitive coded_primitive;
    const fidl::FidlCodedEnum coded_enum;
    const fidl::FidlCodedBits coded_bits;
    fidl::FidlCodedStruct coded_struct;
    const fidl::FidlCodedStructPointer coded_struct_pointer;
    const fidl::FidlCodedTable coded_table;
    fidl::FidlCodedUnion coded_union;
    const fidl::FidlCodedUnionPointer coded_union_pointer;
    fidl::FidlCodedXUnion coded_xunion;
    const fidl::FidlCodedHandle coded_handle;
    const fidl::FidlCodedString coded_string;
    const fidl::FidlCodedArray coded_array;
    const fidl::FidlCodedVector coded_vector;
  };

  constexpr fidl_type(fidl::FidlCodedPrimitive coded_primitive) noexcept
      : type_tag(fidl::kFidlTypePrimitive), coded_primitive(coded_primitive) {}

  constexpr fidl_type(fidl::FidlCodedEnum coded_enum) noexcept
      : type_tag(fidl::kFidlTypeEnum), coded_enum(coded_enum) {}

  constexpr fidl_type(fidl::FidlCodedBits coded_bits) noexcept
      : type_tag(fidl::kFidlTypeBits), coded_bits(coded_bits) {}

  constexpr fidl_type(fidl::FidlCodedStruct coded_struct) noexcept
      : type_tag(fidl::kFidlTypeStruct), coded_struct(coded_struct) {}

  constexpr fidl_type(fidl::FidlCodedStructPointer coded_struct_pointer) noexcept
      : type_tag(fidl::kFidlTypeStructPointer), coded_struct_pointer(coded_struct_pointer) {}

  constexpr fidl_type(fidl::FidlCodedTable coded_table) noexcept
      : type_tag(fidl::kFidlTypeTable), coded_table(coded_table) {}

  constexpr fidl_type(fidl::FidlCodedUnion coded_union) noexcept
      : type_tag(fidl::kFidlTypeUnion), coded_union(coded_union) {}

  constexpr fidl_type(fidl::FidlCodedUnionPointer coded_union_pointer) noexcept
      : type_tag(fidl::kFidlTypeUnionPointer), coded_union_pointer(coded_union_pointer) {}

  constexpr fidl_type(fidl::FidlCodedXUnion coded_xunion) noexcept
      : type_tag(fidl::kFidlTypeXUnion), coded_xunion(coded_xunion) {}

  constexpr fidl_type(fidl::FidlCodedHandle coded_handle) noexcept
      : type_tag(fidl::kFidlTypeHandle), coded_handle(coded_handle) {}

  constexpr fidl_type(fidl::FidlCodedString coded_string) noexcept
      : type_tag(fidl::kFidlTypeString), coded_string(coded_string) {}

  constexpr fidl_type(fidl::FidlCodedArray coded_array) noexcept
      : type_tag(fidl::kFidlTypeArray), coded_array(coded_array) {}

  constexpr fidl_type(fidl::FidlCodedVector coded_vector) noexcept
      : type_tag(fidl::kFidlTypeVector), coded_vector(coded_vector) {}
};

namespace fidl {

namespace internal {

extern const fidl_type kBoolTable;
extern const fidl_type kInt8Table;
extern const fidl_type kInt16Table;
extern const fidl_type kInt32Table;
extern const fidl_type kInt64Table;
extern const fidl_type kUint8Table;
extern const fidl_type kUint16Table;
extern const fidl_type kUint32Table;
extern const fidl_type kUint64Table;
extern const fidl_type kFloat32Table;
extern const fidl_type kFloat64Table;

}  // namespace internal

}  // namespace fidl

#endif  // SYSROOT_ZIRCON_FIDL_H_
