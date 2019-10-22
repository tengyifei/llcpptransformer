// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <cstdio>
#include <cassert>

#include "transformer.h"

// #define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(FORMAT, ...)                                  \
  {                                                                \
    printf(FORMAT, __VA_ARGS__);                                   \
  }
#define DEBUG_PRINT_POSITION(POSITION)                             \
  {                                                                \
    printf("Position: src_inline=%d -> dst_inline=%d\n",           \
            POSITION.src_inline_offset,                            \
            POSITION.dst_inline_offset);                           \
    printf("          src_out_of_line=%d -> dst_out_of_line=%d\n", \
            POSITION.src_out_of_line_offset,                       \
            POSITION.dst_out_of_line_offset);                      \
  }
#else
#define DEBUG_PRINTF(FORMAT, ...) { /* no debug */ }
#define DEBUG_PRINT_POSITION(POSITION) { /* no debug */ }
#endif

// Aligns elements within an array or vector. Simiar to FIDL_ALIGN except that
// alignment is 1 for elements of size 1,
//              2 for those of size 2
//              4 for elements of size 3,
//              8 otherwise.
#define FIDL_ELEM_ALIGN(a)    \
  (((a) < 3) ? (a) :              \
  ((a) <= 4) ? (((a) + 3) & ~3)   \
             : (((a) + 7) & ~7))

namespace {

enum struct WireFormat {
  kOld,
  kV1,
};

uint32_t InlineSize(const fidl_type_t* type, WireFormat wire_format) {
    if (!type) {
      return 8;
    }
    switch (type->type_tag) {
    case fidl::kFidlTypePrimitive:
    case fidl::kFidlTypeEnum:
    case fidl::kFidlTypeBits:
      assert(false && "bug: should not get called");
    case fidl::kFidlTypeStructPointer:
    case fidl::kFidlTypeUnionPointer:
      return 8;
    case fidl::kFidlTypeVector:
    case fidl::kFidlTypeString:
      return 16;
    case fidl::kFidlTypeStruct:
      return type->coded_struct.size;
    case fidl::kFidlTypeUnion:
      switch (wire_format) {
      case WireFormat::kOld:
        return type->coded_union.size;
      case WireFormat::kV1:
        return 24; // xunion
      }
    case fidl::kFidlTypeArray:
      return type->coded_array.array_size;
    case fidl::kFidlTypeHandle:
    case fidl::kFidlTypeTable:
    case fidl::kFidlTypeXUnion:
    default:
      assert(false && "TODO!");
      return 0;
    }
}

const uint32_t UNKNOWN_OFFSET = 0x87654321;

struct Position {
  uint32_t src_inline_offset = 0;
  uint32_t src_out_of_line_offset = 0;
  uint32_t dst_inline_offset = 0;
  uint32_t dst_out_of_line_offset = 0;

  inline Position IncreaseInlineOffset(uint32_t increase) const {
    return IncreaseSrcInlineOffset(increase).IncreaseDstInlineOffset(increase);
  }

  inline Position IncreaseSrcInlineOffset(uint32_t increase) const {
    return Position{
      .src_inline_offset = src_inline_offset + increase,
      .src_out_of_line_offset = src_out_of_line_offset,
      .dst_inline_offset = dst_inline_offset,
      .dst_out_of_line_offset = dst_out_of_line_offset,
    };
  }

  inline Position IncreaseDstInlineOffset(uint32_t increase) const {
    return Position{
      .src_inline_offset = src_inline_offset,
      .src_out_of_line_offset = src_out_of_line_offset,
      .dst_inline_offset = dst_inline_offset + increase,
      .dst_out_of_line_offset = dst_out_of_line_offset,
    };
  }
};

class SrcDst final {
public:
  SrcDst(const uint8_t* src_bytes, const uint32_t src_num_bytes,
         uint8_t* dst_bytes, uint32_t* dst_num_bytes)
    : src_bytes_(src_bytes), src_num_bytes_(src_num_bytes),
      dst_bytes_(dst_bytes), dst_num_bytes_(dst_num_bytes) {}

  ~SrcDst() {
    *dst_num_bytes_ = dst_highest_offset_;
  }

  template <typename T> // TODO: restrict T should be pointer type
  const T* Read(const Position& position) const {
    return reinterpret_cast<const T*>(src_bytes_ + position.src_inline_offset);
  }

  void Copy(const Position& position, uint32_t size) {
    DEBUG_PRINTF("Copy: src_offset=%d dst_offset=%d size=%d\n",
           position.src_inline_offset,
           position.dst_inline_offset,
           size);

    assert(size > 0);
    assert(position.src_inline_offset + size <= src_num_bytes_);

    memcpy(dst_bytes_ + position.dst_inline_offset,
           src_bytes_ + position.src_inline_offset,
           size);
    UpdateHighestOffset(position.dst_inline_offset + size);
  }

  void Pad(const Position& position, uint32_t size) {
    DEBUG_PRINTF("Pad: dst_offset=%d size=%d\n",
                 position.dst_inline_offset,
                 size);

    memset(dst_bytes_ + position.dst_inline_offset, 0, size);
    UpdateHighestOffset(position.dst_inline_offset + size);
  }

  template <typename T> // TODO: restrict to only uint32_t, uint64_t, etc.
  void Write(const Position& position, T value) {
    DEBUG_PRINTF("Write: dst_offset=%d, sizeof(value)=%d\n",
                 position.dst_inline_offset,
                 sizeof(value));

    auto ptr = reinterpret_cast<T*>(dst_bytes_ + position.dst_inline_offset);
    *ptr = value;
    UpdateHighestOffset(position.dst_inline_offset + sizeof(value));
  }

private:
  void UpdateHighestOffset(uint32_t dst_offset) {
    if (dst_offset > dst_highest_offset_) {
      dst_highest_offset_ = dst_offset;
    }
  }

  const uint8_t* src_bytes_;
  const uint32_t src_num_bytes_;
  uint8_t* dst_bytes_;
  uint32_t* dst_num_bytes_;

  uint32_t dst_highest_offset_ = 0;
};

class TransformerBase {
public:
  TransformerBase(SrcDst* src_dst, const char** out_error_msg)
    : src_dst(src_dst), out_error_msg_(out_error_msg) {}
  virtual ~TransformerBase() = default;

  zx_status_t TransformTopLevelStruct(const fidl_type_t* type) {
    if (type->type_tag != fidl::kFidlTypeStruct) {
      return Fail(ZX_ERR_INVALID_ARGS, "only top-level structs supported");
    }

    const auto& src_coded_struct = type->coded_struct;
    const auto& dst_coded_struct = *src_coded_struct.alt_type;
    zx_status_t status = TransformStruct(src_coded_struct, dst_coded_struct, Position{
      .src_inline_offset = 0,
      .src_out_of_line_offset = src_coded_struct.size,
      .dst_inline_offset = 0,
      .dst_out_of_line_offset = dst_coded_struct.size,
    }, dst_coded_struct.size);
    if (status != ZX_OK) {
      return status;
    }
    return ZX_OK;
  }

protected:
  zx_status_t Transform(const fidl_type_t* type,
                        const Position& position,
                        const uint32_t dst_size) {
    if (!type) {
      goto no_transform_just_copy;
    }

    switch (type->type_tag) {
    case fidl::kFidlTypePrimitive:
    case fidl::kFidlTypeEnum:
    case fidl::kFidlTypeBits:
    case fidl::kFidlTypeHandle:
      goto no_transform_just_copy;
    case fidl::kFidlTypeStructPointer: {
      const auto& src_coded_struct = *type->coded_struct_pointer.struct_type;
      const auto& dst_coded_struct = *src_coded_struct.alt_type;
      return TransformStructPointer(src_coded_struct, dst_coded_struct, position);
    }
    case fidl::kFidlTypeUnionPointer:
      assert(false && "nullable unions are no longer supported");
      return ZX_ERR_BAD_STATE;
    case fidl::kFidlTypeStruct: {
      const auto& src_coded_struct = type->coded_struct;
      const auto& dst_coded_struct = *src_coded_struct.alt_type;
      return TransformStruct(src_coded_struct, dst_coded_struct, position, dst_size);
    }
    case fidl::kFidlTypeUnion: {
      const auto& src_coded_union = type->coded_union;
      const auto& dst_coded_union = *src_coded_union.alt_type;
      return TransformUnion(src_coded_union, dst_coded_union, position);
    }
    case fidl::kFidlTypeArray: {
      const auto convert = [](const fidl::FidlCodedArray& coded_array) {
        return fidl::FidlCodedArrayNew(
          coded_array.element,
          coded_array.array_size / coded_array.element_size,
          coded_array.element_size,
          0,
          nullptr /* alt_type unused, we provide both src and dst */);
      };
      auto src_coded_array = convert(type->coded_array);
      auto dst_coded_array = convert(*type->coded_array.alt_type);
      uint32_t dst_array_size = type->coded_array.alt_type->array_size;
      return TransformArray(src_coded_array, dst_coded_array, position, dst_array_size);
    }
    case fidl::kFidlTypeString:
      return TransformString(position);
    case fidl::kFidlTypeVector: {
      const auto& src_coded_vector = type->coded_vector;
      const auto& dst_coded_vector = *src_coded_vector.alt_type;
      return TransformVector(src_coded_vector, dst_coded_vector, position);
    }
    case fidl::kFidlTypeTable:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
    case fidl::kFidlTypeXUnion:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
    }

    return ZX_ERR_BAD_STATE;

no_transform_just_copy:
    src_dst->Copy(position, dst_size);
    return ZX_OK;
  }

  virtual zx_status_t TransformStructPointer(
    const fidl::FidlCodedStruct& src_coded_struct,
    const fidl::FidlCodedStruct& dst_coded_struct,
    const Position& position) = 0;

  virtual zx_status_t TransformStruct(
    const fidl::FidlCodedStruct& src_coded_struct,
    const fidl::FidlCodedStruct& dst_coded_struct,
    const Position& position,
    uint32_t dst_size) = 0;

  virtual zx_status_t TransformUnion(
    const fidl::FidlCodedUnion& src_coded_union,
    const fidl::FidlCodedUnion& dst_coded_union,
    const Position& position) = 0;

  virtual zx_status_t TransformString(
    const Position& position) = 0;

  virtual zx_status_t TransformVector(
    const fidl::FidlCodedVector& src_coded_vector,
    const fidl::FidlCodedVector& dst_coded_vector,
    const Position& position) = 0;

  virtual zx_status_t TransformArray(
    const fidl::FidlCodedArrayNew& src_coded_array,
    const fidl::FidlCodedArrayNew& dst_coded_array,
    const Position& position,
    uint32_t dst_array_size) = 0;

  inline zx_status_t Fail(zx_status_t status, const char* error_msg) {
    *out_error_msg_ = error_msg;
    return status;
  }

  SrcDst* src_dst;

private:
  const char** out_error_msg_;
};

class Unsupported final : public TransformerBase {
public:
  Unsupported(const char** out_error_msg) : TransformerBase(nullptr, out_error_msg) {}

protected:
  zx_status_t TransformStructPointer(
    const fidl::FidlCodedStruct& src_coded_struct,
    const fidl::FidlCodedStruct& dst_coded_struct,
    const Position& position) {
    assert(false && "unreachable");
    return ZX_ERR_BAD_STATE;
  }

  zx_status_t TransformStruct(
    const fidl::FidlCodedStruct& src_coded_struct,
    const fidl::FidlCodedStruct& dst_coded_struct,
    const Position& position,
    uint32_t dst_size) {
    return Fail(ZX_ERR_INVALID_ARGS, "unsupported transformation");
  }

  zx_status_t TransformUnion(
    const fidl::FidlCodedUnion& src_coded_union,
    const fidl::FidlCodedUnion& dst_coded_union,
    const Position& position) {
    assert(false && "unreachable");
    return ZX_ERR_BAD_STATE;
  }

  zx_status_t TransformString(
    const Position& position) {
    assert(false && "unreachable");
    return ZX_ERR_BAD_STATE;
  }

  zx_status_t TransformVector(
    const fidl::FidlCodedVector& src_coded_vector,
    const fidl::FidlCodedVector& dst_coded_vector,
    const Position& position) {
    assert(false && "unreachable");
    return ZX_ERR_BAD_STATE;
  }

  zx_status_t TransformArray(
    const fidl::FidlCodedArrayNew& src_coded_array,
    const fidl::FidlCodedArrayNew& dst_coded_array,
    const Position& position,
    uint32_t dst_array_size) {
    assert(false && "unreachable");
    return ZX_ERR_BAD_STATE;
  }
};

class V1ToOld final : public TransformerBase {
public:
  V1ToOld(SrcDst* src_dst, const char** out_error_msg)
    : TransformerBase(src_dst, out_error_msg) {}

 private:
  zx_status_t TransformStructPointer(const fidl::FidlCodedStruct& src_coded_struct,
                                     const fidl::FidlCodedStruct& dst_coded_struct,
                                     const Position& position) {
    // Read presence.
    auto presence = *src_dst->Read<uint64_t>(position);

    // Copy presence.
    src_dst->Copy(position, 8);

    // Early exit when absent.
    if (presence != FIDL_ALLOC_PRESENT) {
      return ZX_OK;
    }

    // Transform struct.
    uint32_t aligned_src_size = FIDL_ALIGN(src_coded_struct.size);
    uint32_t aligned_dst_size = FIDL_ALIGN(dst_coded_struct.size);
    const auto struct_position = Position{
      .src_inline_offset = position.src_out_of_line_offset,
      .src_out_of_line_offset = position.src_out_of_line_offset + aligned_src_size,
      .dst_inline_offset = position.dst_out_of_line_offset,
      .dst_out_of_line_offset = position.dst_out_of_line_offset + aligned_dst_size,
    };
    return TransformStruct(src_coded_struct, dst_coded_struct,
                           struct_position, aligned_dst_size);
  }

  zx_status_t TransformStruct(const fidl::FidlCodedStruct& src_coded_struct,
                              const fidl::FidlCodedStruct& dst_coded_struct,
                              const Position& position,
                              uint32_t dst_size) {
    // Note: we cannot use dst_coded_struct.size, and must instead rely on
    // the provided dst_size since this struct could be placed in an alignment
    // context that is larger than its inherent size.

    // Copy structs without any coded fields, and done.
    if (src_coded_struct.field_count == 0) {
      src_dst->Copy(position, dst_size);
      return ZX_OK;
    }

    const uint32_t src_start_of_struct = position.src_inline_offset;
    const uint32_t dst_start_of_struct = position.dst_inline_offset;
    const uint32_t dst_end_of_src_struct = position.dst_inline_offset + dst_size;

    auto current_position = position;
    for (uint32_t src_field_index = 0;
         src_field_index < src_coded_struct.field_count;
         src_field_index++) {

      const auto& src_field = src_coded_struct.fields[src_field_index];

      // Copy fields without coding tables.
      if (!src_field.type) {
        uint32_t dst_field_size =
          src_field.offset /*.padding_offset*/ + (src_start_of_struct - current_position.src_inline_offset);
        src_dst->Copy(current_position, dst_field_size);
        current_position = current_position.IncreaseInlineOffset(dst_field_size);
        continue;
      }

      assert(src_field.alt_field);
      const auto& dst_field = *src_field.alt_field;

      // Pad between fields (if needed).
      if (current_position.dst_inline_offset < dst_field.offset) {
        uint32_t padding_size = dst_field.offset - current_position.dst_inline_offset;
        src_dst->Pad(current_position, padding_size);
        current_position = current_position.IncreaseInlineOffset(padding_size);
      }

      // Set current position before transforming field.
      // TODO: We shouldn't need those if we kept track of everything perfectly.
      current_position.src_inline_offset = src_start_of_struct + src_field.offset;
      current_position.dst_inline_offset = dst_start_of_struct + dst_field.offset;

      // Transform field.
      uint32_t src_next_field_offset =
          current_position.src_inline_offset + InlineSize(src_field.type, WireFormat::kV1);
      uint32_t dst_next_field_offset =
          current_position.dst_inline_offset + InlineSize(dst_field.type, WireFormat::kOld);
      uint32_t dst_field_size = dst_next_field_offset - dst_field.offset;
      if (zx_status_t status = Transform(src_field.type, current_position, dst_field_size);
          status != ZX_OK) {
        return status;
      }

      // Update current position for next iteration.
      current_position.src_inline_offset = src_next_field_offset;
      current_position.dst_inline_offset = dst_next_field_offset;
    }

    // Pad end (if needed).
    if (current_position.dst_inline_offset < dst_end_of_src_struct) {
      uint32_t size = dst_end_of_src_struct - current_position.dst_inline_offset;
      src_dst->Pad(current_position, size);
    }

    // Done.
    return ZX_OK;
  }

  zx_status_t TransformUnion(const fidl::FidlCodedUnion& src_coded_union,
                             const fidl::FidlCodedUnion& dst_coded_union,
                             const Position& position) {
    assert(src_coded_union.field_count == dst_coded_union.field_count);

    // Read: flexible-union ordinal.
    auto src_xunion = src_dst->Read<const fidl_xunion_t>(position);
    uint32_t xunion_ordinal = src_xunion->tag;

    // Retrieve: flexible-union field (or variant).
    bool src_field_found = false;
    uint32_t src_field_index = 0;
    const fidl::FidlUnionField* src_field = nullptr;
    for (/* src_field_index needed after the loop */;
         src_field_index <= src_coded_union.field_count;
         src_field_index++) {
      const fidl::FidlUnionField* candidate_src_field = &src_coded_union.fields[src_field_index];
      if (candidate_src_field->xunion_ordinal == xunion_ordinal) {
        src_field_found = true;
        src_field = candidate_src_field;
        break;
      }
    }
    if (!src_field_found) {
      return Fail(ZX_ERR_BAD_STATE, "ordinal has no corresponding variant");
    }

    const fidl::FidlUnionField& dst_field = dst_coded_union.fields[src_field_index];

    // Write: static-union tag, and pad (if needed).
    switch (dst_coded_union.data_offset) {
    case 4:
      src_dst->Write(position, src_field_index);
      break;
    case 8:
      src_dst->Write(position, static_cast<uint64_t>(src_field_index));
      break;
    default:
      assert(false && "static-union data offset can only be 4 or 8");
    }

    // Write: static-union field (or variant).
    auto field_position = Position{
      .src_inline_offset = position.src_out_of_line_offset,
      .src_out_of_line_offset = position.src_out_of_line_offset + InlineSize(src_field->type, WireFormat::kOld),
      .dst_inline_offset = position.dst_inline_offset + dst_coded_union.data_offset,
      .dst_out_of_line_offset = position.dst_out_of_line_offset,
    };
    uint32_t dst_field_size = dst_coded_union.size - dst_coded_union.data_offset;
    if (zx_status_t status = Transform(src_field->type, field_position, dst_field_size);
        status != ZX_OK) {
      return status;
    }

    // Pad after static-union data.
    auto field_padding_position = field_position.IncreaseDstInlineOffset(
      dst_field_size - dst_field.padding);
    src_dst->Pad(field_padding_position, dst_field.padding);

    // Done.
    return ZX_OK;
  }

  zx_status_t TransformString(const Position& position) {
    static const auto string_as_coded_vector = fidl::FidlCodedVector(
      nullptr /* element */,
      0 /*max count, unused */,
      1 /* element_size */,
      fidl::FidlNullability::kNullable /* being lax, we do not check constraints */,
      nullptr /* alt_type unused, we provide both src and dst */);
    return TransformVector(string_as_coded_vector, string_as_coded_vector, position);
  }

  zx_status_t TransformVector(const fidl::FidlCodedVector& src_coded_vector,
                              const fidl::FidlCodedVector& dst_coded_vector,
                              const Position& position) {
    // Read number of elements in vectors.
    auto num_elements = *src_dst->Read<uint32_t>(position);

    // Read presence.
    auto presence = *src_dst->Read<uint64_t>(position.IncreaseSrcInlineOffset(8));

    // Copy vector header.
    src_dst->Copy(position, 16);

    // Early exit on nullable vectors.
    if (presence != FIDL_ALLOC_PRESENT) {
      return ZX_OK;
    }

    // Viewing vectors data as arrays.
    uint32_t src_element_padding =
      FIDL_ELEM_ALIGN(src_coded_vector.element_size) - src_coded_vector.element_size;
    uint32_t dst_element_padding =
      FIDL_ELEM_ALIGN(dst_coded_vector.element_size) - dst_coded_vector.element_size;
    const auto convert = [&](const fidl::FidlCodedVector& coded_vector, uint32_t element_padding) {
      return fidl::FidlCodedArrayNew(
        coded_vector.element,
        num_elements,
        coded_vector.element_size,
        element_padding,
        nullptr /* alt_type unused, we provide both src and dst */);
    };
    const auto src_vector_data_as_coded_array = convert(src_coded_vector, src_element_padding);
    const auto dst_vector_data_as_coded_array = convert(dst_coded_vector, dst_element_padding);

    // Calculate vector size.
    uint32_t src_vector_size = FIDL_ALIGN(
      num_elements * (src_coded_vector.element_size + src_element_padding));
    uint32_t dst_vector_size = FIDL_ALIGN(
      num_elements * (dst_coded_vector.element_size + dst_element_padding));

    // Transform elements.
    auto vector_data_position = Position{
      .src_inline_offset = position.src_out_of_line_offset,
      .src_out_of_line_offset = position.src_out_of_line_offset + src_vector_size,
      .dst_inline_offset = position.dst_out_of_line_offset,
      .dst_out_of_line_offset = position.dst_out_of_line_offset + dst_vector_size
    };
    if (zx_status_t status = TransformArray(src_vector_data_as_coded_array,
                                            dst_vector_data_as_coded_array,
                                            vector_data_position,
                                            dst_vector_size);
        status != ZX_OK) {
      return status;
    }

    // Done.
    return ZX_OK;
  }

  zx_status_t TransformArray(const fidl::FidlCodedArrayNew& src_coded_array,
                             const fidl::FidlCodedArrayNew& dst_coded_array,
                             const Position& position,
                             uint32_t dst_array_size) {
    assert(src_coded_array.element_count == dst_coded_array.element_count);

    // Fast path for elements without coding tables (e.g. strings).
    if (!src_coded_array.element) {
      src_dst->Copy(position, dst_array_size);
      return ZX_OK;
    }

    // Slow path otherwise.
    auto element_position = position;
    for (uint32_t i = 0; i < src_coded_array.element_count; i++) {
      if (zx_status_t status = Transform(src_coded_array.element, element_position,
                                         dst_coded_array.element_size);
          status != ZX_OK) {
        return status;
      }

      // Pad end of an element.
      auto padding_position = element_position.
        IncreaseSrcInlineOffset(src_coded_array.element_size).
        IncreaseDstInlineOffset(dst_coded_array.element_size);
      src_dst->Pad(padding_position, dst_coded_array.element_padding);
      
      element_position = padding_position.
        IncreaseSrcInlineOffset(src_coded_array.element_padding).
        IncreaseDstInlineOffset(dst_coded_array.element_padding);
    }

    // Pad end of elements.
    uint32_t padding = dst_array_size + position.dst_inline_offset
                                      - element_position.dst_inline_offset;
    src_dst->Pad(element_position, padding);

    return ZX_OK;
  }
};

}  // namespace

zx_status_t fidl_transform(fidl_transformation_t transformation,
                           const fidl_type_t* type,
                           const void* src_bytes, void* dst_bytes,
                           uint32_t src_num_bytes, uint32_t* dst_num_bytes,
                           const char** out_error_msg) {
  auto src_dst = SrcDst(
    static_cast<const uint8_t*>(src_bytes), src_num_bytes,
    static_cast<uint8_t*>(dst_bytes), dst_num_bytes);
  switch (transformation) {
  case FIDL_TRANSFORMATION_NONE:
    return ZX_OK;
  case FIDL_TRANSFORMATION_V1_TO_OLD:
    return V1ToOld(&src_dst, out_error_msg).TransformTopLevelStruct(type);
  default:
    return Unsupported(out_error_msg).TransformTopLevelStruct(type);
  }
}
