// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <cstdio>
#include <cassert>

#include "transformer.h"

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
      assert(false && "bug should not get called");
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
  const T Read(const Position& position) {
    return reinterpret_cast<const T>(src_bytes_ + position.src_inline_offset);
  }

  void Copy(const Position& position, uint32_t size) {
    printf("Copy: src_offset=%d dst_offset=%d size=%d\n",
           position.src_inline_offset,
           position.dst_inline_offset,
           size);

    assert(size > 0);
    if (!(position.src_inline_offset + size <= src_num_bytes_)) {
      printf("don't stop me now!");
    }
    assert(position.src_inline_offset + size <= src_num_bytes_);

    memcpy(dst_bytes_ + position.dst_inline_offset,
           src_bytes_ + position.src_inline_offset,
           size);
    UpdateHighestOffset(position.dst_inline_offset + size);
  }

  void Pad(const Position& position, uint32_t size) {
    printf("Pad: dst_offset=%d size=%d\n", position.dst_inline_offset, size);

    assert(size > 0);

    memset(dst_bytes_ + position.dst_inline_offset, 0, size);
    UpdateHighestOffset(position.dst_inline_offset + size);
  }

  template <typename T> // TODO: restrict to only uint32_t, uint64_t, etc.?
  void Write(const Position& position, T value) {
    printf("Write: dst_offset=%d, sizeof(value)=%d\n", position.dst_inline_offset, sizeof(value));

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
    const fidl::FidlCodedStruct& coded_struct = type->coded_struct;
    const fidl::FidlCodedStruct& coded_struct_in_v1_no_ee = *coded_struct.alt_type;

    zx_status_t status = TransformStruct(*type, Position{
      .src_inline_offset = 0,
      .src_out_of_line_offset = coded_struct.size,
      .dst_inline_offset = 0,
      .dst_out_of_line_offset = coded_struct_in_v1_no_ee.size,
    });
    if (status != ZX_OK) {
      return status;
    }
    return ZX_OK;
  }

 private:
  zx_status_t Transform(const fidl_type_t* type,
                        const Position& position, const uint32_t dst_size) {
    if (!type) {
      goto no_transform_just_copy;
    }

    switch (type->type_tag) {
    case fidl::kFidlTypePrimitive:
    case fidl::kFidlTypeEnum:
    case fidl::kFidlTypeBits:
      goto no_transform_just_copy;

    case fidl::kFidlTypeStructPointer:
    case fidl::kFidlTypeUnionPointer:
    case fidl::kFidlTypeString:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;

    case fidl::kFidlTypeStruct:
      return TransformStruct(*type, position);
    case fidl::kFidlTypeUnion:
      return TransformUnion(*type, position, dst_size);
    case fidl::kFidlTypeArray:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
    case fidl::kFidlTypeHandle:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
    case fidl::kFidlTypeVector:
      assert(false && "TODO!");
      return ZX_ERR_BAD_STATE;
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

  zx_status_t TransformStruct(const fidl_type_t& v1_no_ee_type, Position current_position) {
    assert(v1_no_ee_type.type_tag == fidl::kFidlTypeStruct);
    const fidl::FidlCodedStruct& src_coded_struct = v1_no_ee_type.coded_struct;

    // Copy structs without any coded fields, and done.
    if (src_coded_struct.field_count == 0) {
      src_dst.Copy(current_position, src_coded_struct.size);
      return ZX_OK;
    }

    const uint32_t src_start_of_struct = current_position.src_inline_offset;

    const fidl::FidlCodedStruct& dst_coded_struct = *v1_no_ee_type.coded_struct.alt_type;
    const uint32_t dst_start_of_struct = current_position.dst_inline_offset;
    const uint32_t dst_end_of_src_struct =
        current_position.dst_inline_offset + dst_coded_struct.size;

    for (uint32_t src_field_index = 0;
         src_field_index < src_coded_struct.field_count;
         src_field_index++) {

      const auto& src_field = src_coded_struct.fields[src_field_index];

      // Copy fields without coding tables.
      if (!src_field.type) {
        uint32_t dst_size =
          src_field.offset /*.padding_offset*/ + (src_start_of_struct - current_position.src_inline_offset);
        src_dst.Copy(current_position, dst_size);
        current_position.dst_inline_offset += dst_size;
        continue;
      }


      assert(src_field.alt_field);
      const auto& dst_field = *src_field.alt_field;

      // Pad between fields (if needed).
      if (current_position.dst_inline_offset < dst_field.offset) {
        uint32_t size = dst_field.offset - current_position.dst_inline_offset;
        src_dst.Pad(current_position, size);
      }

      // Set current position before transforming field.
      current_position.src_inline_offset = src_field.offset;
      current_position.dst_inline_offset = dst_field.offset;

      // Transform field.
      uint32_t src_next_field_offset =
          current_position.src_inline_offset + InlineSize(src_field.type, WireFormat::kV1NoEe);
      uint32_t dst_next_field_offset =
          current_position.dst_inline_offset + InlineSize(dst_field.type, WireFormat::kOld);
      uint32_t dst_size = dst_next_field_offset - dst_field.offset;
      if (zx_status_t status = Transform(src_field.type, current_position, dst_size);
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

  zx_status_t TransformUnion(const fidl_type_t& type,
                             const Position& position, const uint32_t dst_size) {
    assert(type.type_tag == fidl::kFidlTypeUnion);
    const fidl::FidlCodedUnion& src_coded_union = type.coded_union;
    const fidl::FidlCodedUnion& dst_coded_union = *type.coded_union.alt_type;
    assert(src_coded_union.field_count == dst_coded_union.field_count);

    // Read: flexible-union ordinal.
    const fidl_xunion_t* src_union_as_xunion =
        src_dst.Read<const fidl_xunion_t*>(position);
    const auto xunion_ordinal =
        reinterpret_cast<uint32_t>(src_union_as_xunion->tag);

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
    assert(src_field_found && "ordinal has no corresponding variant");
    const fidl::FidlUnionField* dst_field = &dst_coded_union.fields[src_field_index];

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
    uint32_t dst_field_size = dst_size - dst_coded_union.data_offset;
    if (zx_status_t status = Transform(src_field->type, field_position, dst_field_size);
        status != ZX_OK) {
      return status;
    }

    // Done.
    return ZX_OK;
  }

  SrcDst src_dst;
  const char** const out_error_msg_;
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
