// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <cassert>

#include "transformer.h"

namespace {

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
  const T Read(uint32_t src_offset) {
    return reinterpret_cast<const T>(src_bytes_ + src_offset);
  }

  void Copy(uint32_t src_offset, uint32_t dst_offset, uint32_t size) {
    assert(src_offset + size < src_num_bytes_);
    memcpy(dst_bytes_ + dst_offset, src_bytes_ + src_offset, size);
    UpdateHighestOffset(dst_offset + size);
  }

  template <typename T> // TODO: restrict to only uint32_t, uint64_t, etc.?
  void Write(uint32_t dst_offset, T value) {
    auto ptr = reinterpret_cast<T*>(dst_bytes_ + dst_offset);
    *ptr = value;
    UpdateHighestOffset(dst_offset + sizeof(value));
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
  const uint32_t UNKNOWN_OFFSET = 0x87654321;

  struct Position {
    uint32_t src_inline_offset = 0;
    uint32_t src_out_of_line_offset = 0;
    uint32_t dst_inline_offset = 0;
    uint32_t dst_out_of_line_offset = 0;
  };

  zx_status_t Transform(const fidl_type_t* type,
                        const Position& position, const uint32_t element_size) {
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
      return TransformUnion(*type, position);
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
    src_dst.Copy(position.src_inline_offset, position.dst_inline_offset, element_size);
    return ZX_OK;
  }

  zx_status_t TransformStruct(const fidl_type_t& v1_no_ee_type, Position current_position) {
    assert(v1_no_ee_type.type_tag == fidl::kFidlTypeStruct);
    const fidl::FidlCodedStruct& src_coded_struct = v1_no_ee_type.coded_struct;
    const fidl::FidlCodedStruct& dst_coded_struct = *v1_no_ee_type.coded_struct.alt_type;
  
    // src last field index
    // potentially more fields in the v1_no_ee
    // need to skip the ones with no type when incrementing the dst_field_index
    const uint32_t src_last_field_index = src_coded_struct.field_count - 1;

    // starts and end
    const uint32_t src_start_of_struct = current_position.src_inline_offset;
    const uint32_t dst_start_of_struct = current_position.dst_inline_offset;
    const uint32_t src_end_of_src_struct =
        current_position.src_inline_offset + src_coded_struct.size;

    for (uint32_t src_field_index = 0, dst_field_index = 0;
         src_field_index <= src_last_field_index;
         src_field_index++) {
      // catch up if we skipped inline bytes in the struct
      // update current src inline position
      const auto& src_field = src_coded_struct.fields[src_field_index];
      // here special because src_field.type == null => offset really means padding_offset!
      uint32_t src_field_offset = src_start_of_struct + src_field.offset + src_field.padding;
      if (current_position.src_inline_offset < src_field_offset) {
        uint32_t size = src_field_offset - current_position.src_inline_offset;
        src_dst.Copy(current_position.src_inline_offset,
                     current_position.dst_inline_offset,
                     size);
      }
      uint32_t offset_increase = src_field_offset - current_position.src_inline_offset;
      current_position.src_inline_offset = src_field_offset;

      // transformed element size if we need it
      uint32_t next_src_field_offset = (src_field_index == src_last_field_index) ?
          src_end_of_src_struct :
          current_position.src_inline_offset + src_coded_struct.fields[src_field_index + 1].offset;
      uint32_t field_size_plus_padding = next_src_field_offset - src_field_offset;

      // depends on whether we need to skip a field in the dst fields
      // because src is a xunion we have FidlStructField for each field
      // but dst is a union, so we only have a FidlStructField for fields with type
      // if (src_field.type) {
        const auto& dst_field = dst_coded_struct.fields[dst_field_index];
        dst_field_index++;
        uint32_t dst_field_offset = dst_start_of_struct + dst_field.offset + dst_field.padding;
        current_position.dst_inline_offset = dst_field_offset;
      // }

      if (zx_status_t status = Transform(src_field.type, current_position, field_size_plus_padding);
          status != ZX_OK) {
        return status;
      }
    }
    return ZX_OK;
  }

  zx_status_t TransformUnion(const fidl_type_t& type, const Position& position) {
    assert(type.type_tag == fidl::kFidlTypeUnion);
    const fidl::FidlCodedUnion& src_coded_union = type.coded_union;
    const fidl::FidlCodedUnion& dst_coded_union = *type.coded_union.alt_type;

    // retrieve union-as-xunion ordinal
    const fidl_xunion_t* src_union_as_xunion =
        src_dst.Read<const fidl_xunion_t*>(position.src_inline_offset);
    const auto xunion_ordinal =
        reinterpret_cast<uint32_t>(src_union_as_xunion->tag);

    // look up variant by ordinal
    bool variant_found = false;
    uint32_t variant_index = 0;
    const fidl_type_t* variant_type = nullptr;
    for (/* needed after the loop */; variant_index <= dst_coded_union.field_count; variant_index++) {
      const fidl::FidlUnionField& candidate_variant = dst_coded_union.fields[variant_index];
      if (candidate_variant.xunion_ordinal == xunion_ordinal) {
        variant_found = true;
        variant_type = candidate_variant.type;
        break;
      }
    }
    assert(variant_found && "ordinal has no corresponding variant");

    // Write: static-union tag.
    src_dst.Write(position.dst_inline_offset, variant_index);

    auto variant_position = Position{
      .src_inline_offset = position.src_out_of_line_offset,
      .src_out_of_line_offset = UNKNOWN_OFFSET,
      .dst_inline_offset = position.dst_inline_offset + 4, // TODO: or 8 depending on alignment
      .dst_out_of_line_offset = UNKNOWN_OFFSET,
    };
    return Transform(variant_type, variant_position, 0 /* WRONG */);
  }

  SrcDst src_dst;
  // const uint8_t* in_bytes_;
  // const uint32_t in_num_bytes_;
  // uint8_t* out_bytes_;
  // uint32_t* out_num_bytes_;
  const char** const out_error_msg_;
};

}  // namespace

zx_status_t fidl_transform_xunion_to_union(const fidl_type_t* type,
                                           void* in_bytes, uint32_t in_num_bytes,
                                           void* out_bytes, uint32_t* out_num_bytes,
                                           const char** out_error_msg) {
  auto transformer = Transformer(
    static_cast<const uint8_t*>(in_bytes), in_num_bytes,
    static_cast<uint8_t*>(out_bytes), out_num_bytes,
    out_error_msg
  );
  return transformer.RunOnTopLevelStruct(type);
}
