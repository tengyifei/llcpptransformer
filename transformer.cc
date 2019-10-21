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
  kV1NoEe,
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
      case WireFormat::kV1NoEe:
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

class Transformer final {
public:
  Transformer(const uint8_t* src_bytes, const uint32_t src_num_bytes,
              uint8_t* dst_bytes, uint32_t* dst_num_bytes,
              const char** out_error_msg)
    : src_dst(src_bytes, src_num_bytes, dst_bytes, dst_num_bytes),
      out_error_msg_(out_error_msg) {}

  zx_status_t RunOnTopLevelStruct(const fidl_type_t* type) {
    assert(type->type_tag == fidl::kFidlTypeStruct && "only top-level structs supported");
    const fidl::FidlCodedStruct& src_coded_struct = type->coded_struct;
    const fidl::FidlCodedStruct& dst_coded_struct = *src_coded_struct.alt_type;

    zx_status_t status = TransformStruct(type->coded_struct, Position{
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

 private:
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

    case fidl::kFidlTypeStructPointer:
    case fidl::kFidlTypeUnionPointer:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;

    case fidl::kFidlTypeStruct:
      return TransformStruct(type->coded_struct, position, dst_size);
    case fidl::kFidlTypeUnion:
      return TransformUnion(*type, position);
    case fidl::kFidlTypeArray: {
      auto src_coded_array = fidl::FidlCodedArrayNew(type->coded_array);
      auto dst_coded_array = fidl::FidlCodedArrayNew(*type->coded_array.alt_type);
      src_coded_array.alt_type = &dst_coded_array;
      dst_coded_array.alt_type = &src_coded_array;
      uint32_t dst_array_size = type->coded_array.alt_type->array_size;
      return TransformArray(src_coded_array, position, dst_array_size);
    }
    case fidl::kFidlTypeString:
      return TransformString(position);
    case fidl::kFidlTypeVector:
      return TransformVector(type->coded_vector, position);
    case fidl::kFidlTypeTable:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
    case fidl::kFidlTypeXUnion:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
    }

    return ZX_ERR_BAD_STATE;

no_transform_just_copy:
    src_dst.Copy(position, dst_size);
    return ZX_OK;
  }

  zx_status_t TransformStruct(const fidl::FidlCodedStruct& src_coded_struct,
                              Position current_position,
                              uint32_t dst_size) {
    // Note: we cannot use dst_coded_struct.size, and must instead rely on
    // the provided dst_size since this struct could be placed in an alignment
    // context that is larger than its inherent size.

    // Copy structs without any coded fields, and done.
    if (src_coded_struct.field_count == 0) {
      src_dst.Copy(current_position, dst_size);
      return ZX_OK;
    }

    const uint32_t src_start_of_struct = current_position.src_inline_offset;

    const fidl::FidlCodedStruct& dst_coded_struct = *src_coded_struct.alt_type;
    const uint32_t dst_start_of_struct = current_position.dst_inline_offset;
    const uint32_t dst_end_of_src_struct = current_position.dst_inline_offset + dst_size;

    for (uint32_t src_field_index = 0;
         src_field_index < src_coded_struct.field_count;
         src_field_index++) {

      const auto& src_field = src_coded_struct.fields[src_field_index];

      // Copy fields without coding tables.
      if (!src_field.type) {
        uint32_t dst_field_size =
          src_field.offset /*.padding_offset*/ + (src_start_of_struct - current_position.src_inline_offset);
        src_dst.Copy(current_position, dst_field_size);
        current_position = current_position.IncreaseInlineOffset(dst_field_size);
        continue;
      }

      assert(src_field.alt_field);
      const auto& dst_field = *src_field.alt_field;

      // Pad between fields (if needed).
      if (current_position.dst_inline_offset < dst_field.offset) {
        uint32_t padding_size = dst_field.offset - current_position.dst_inline_offset;
        src_dst.Pad(current_position, padding_size);
        current_position = current_position.IncreaseInlineOffset(padding_size);
      }

      // Set current position before transforming field.
      // TODO: We shouldn't need those if we kept track of everything perfectly.
      current_position.src_inline_offset = src_field.offset;
      current_position.dst_inline_offset = dst_field.offset;

      // Transform field.
      uint32_t src_next_field_offset =
          current_position.src_inline_offset + InlineSize(src_field.type, WireFormat::kV1NoEe);
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
      src_dst.Pad(current_position, size);
    }

    // Done.
    return ZX_OK;
  }

  zx_status_t TransformUnion(const fidl_type_t& type, const Position& position) {
    assert(type.type_tag == fidl::kFidlTypeUnion);
    const fidl::FidlCodedUnion& src_coded_union = type.coded_union;
    const fidl::FidlCodedUnion& dst_coded_union = *type.coded_union.alt_type;
    assert(src_coded_union.field_count == dst_coded_union.field_count);

    // Read: flexible-union ordinal.
    auto src_xunion = src_dst.Read<const fidl_xunion_t>(position);
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
      SetError("ordinal has no corresponding variant");
      return ZX_ERR_BAD_STATE;
    }

    const fidl::FidlUnionField& dst_field = dst_coded_union.fields[src_field_index];

    // Write: static-union tag, and pad (if needed).
    switch (dst_coded_union.data_offset) {
    case 4:
      src_dst.Write(position, src_field_index);
      break;
    case 8:
      src_dst.Write(position, static_cast<uint64_t>(src_field_index));
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
    src_dst.Pad(field_padding_position, dst_field.padding);

    // Done.
    return ZX_OK;
  }

  zx_status_t TransformString(const Position& position) {
    static const auto string_as_coded_vector_alt_type = fidl::FidlCodedVector(
      nullptr /* element */,
      0 /*max count, unused */,
      1 /* element_size */,
      fidl::FidlNullability::kNullable /* being lax, we do not check constraints */,
      nullptr
    );
    static const auto string_as_coded_vector = fidl::FidlCodedVector(
      nullptr /* element */,
      0 /*max count, unused */,
      1 /* element_size */,
      fidl::FidlNullability::kNullable /* being lax, we do not check constraints */,
      &string_as_coded_vector_alt_type
    );
    return TransformVector(string_as_coded_vector, position);
  }

  zx_status_t TransformVector(const fidl::FidlCodedVector& src_coded_vector,
                              const Position& position) {
    const fidl::FidlCodedVector& dst_coded_vector = *src_coded_vector.alt_type;

    // Read number of elements in vectors.
    auto num_elements = *src_dst.Read<uint32_t>(position);

    // Read presence.
    auto presence = *src_dst.Read<uint64_t>(position.IncreaseSrcInlineOffset(8));

    // Copy vector header.
    src_dst.Copy(position, 16);

    // Early exit on nullable vectors.
    if (presence != FIDL_ALLOC_PRESENT) {
      return ZX_OK;
    }

    // Viewing vectors data as arrays.
    uint32_t src_element_padding =
      FIDL_ELEM_ALIGN(src_coded_vector.element_size) - src_coded_vector.element_size;
    uint32_t dst_element_padding =
      FIDL_ELEM_ALIGN(dst_coded_vector.element_size) - dst_coded_vector.element_size;
    auto src_vector_data_as_coded_array = fidl::FidlCodedArrayNew(
      src_coded_vector.element,
      num_elements,
      src_coded_vector.element_size,
      src_element_padding
    );
    auto dst_vector_data_as_coded_array = fidl::FidlCodedArrayNew(
      dst_coded_vector.element,
      num_elements,
      dst_coded_vector.element_size,
      dst_element_padding
    );
    src_vector_data_as_coded_array.alt_type = &dst_vector_data_as_coded_array;
    dst_vector_data_as_coded_array.alt_type = &src_vector_data_as_coded_array;

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
    if (zx_status_t status = TransformArray(src_vector_data_as_coded_array, vector_data_position,
                                            dst_vector_size);
        status != ZX_OK) {
      return status;
    }

    // Done.
    return ZX_OK;
  }

  zx_status_t TransformArray(const fidl::FidlCodedArrayNew& src_coded_array,
                             const Position& position,
                             uint32_t dst_array_size) {

    // Fast path for elements without coding tables (e.g. strings).
    if (!src_coded_array.element) {
      src_dst.Copy(position, dst_array_size);
      return ZX_OK;
    }

    const auto& dst_coded_array = *src_coded_array.alt_type;
    assert(src_coded_array.element_count == dst_coded_array.element_count);

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
      src_dst.Pad(padding_position, dst_coded_array.element_padding);
      
      element_position = padding_position.
        IncreaseSrcInlineOffset(src_coded_array.element_padding).
        IncreaseDstInlineOffset(dst_coded_array.element_padding);
    }

    // Pad end of elements.
    uint32_t padding = dst_array_size + position.dst_inline_offset
                                      - element_position.dst_inline_offset;
    src_dst.Pad(element_position, padding);

    return ZX_OK;
  }

  void SetError(const char* error_msg) {
    *out_error_msg_ = error_msg;
  }

  SrcDst src_dst;
  const char** out_error_msg_;
};

}  // namespace

zx_status_t fidl_transform_xunion_to_union(const fidl_type_t* type,
                                           const void* in_bytes, uint32_t in_num_bytes,
                                           void* out_bytes, uint32_t* out_num_bytes,
                                           const char** out_error_msg) {
  auto transformer = Transformer(
    static_cast<const uint8_t*>(in_bytes), in_num_bytes,
    static_cast<uint8_t*>(out_bytes), out_num_bytes,
    out_error_msg
  );
  return transformer.RunOnTopLevelStruct(type);
}
