// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <cassert>

#include "transformer.h"

namespace {

class Transformer final {
public:
  Transformer(const uint8_t* in_bytes, const uint32_t in_num_bytes,
              uint8_t* out_bytes, uint32_t* out_num_bytes,
              const char** out_error_msg) :
    in_bytes_(in_bytes), in_num_bytes_(in_num_bytes),
    out_bytes_(out_bytes), out_num_bytes_(out_num_bytes),
    out_error_msg_(out_error_msg) {}

  zx_status_t RunOnTopLevelStruct(const fidl_type_t* type) {
    assert(type->type_tag == fidl::kFidlTypeStruct && "only top-level structs supported");
    const fidl::FidlCodedStruct& coded_struct = type->coded_struct;
    const fidl::FidlCodedStruct& coded_struct_in_v1_no_ee = *coded_struct.in_v1_no_ee;

    return TransformStruct(*type, Position{
      .src_inline_offset = 0,
      .src_out_of_line_offset = coded_struct.size,
      .dst_inline_offset = 0,
      .dst_out_of_line_offset = coded_struct_in_v1_no_ee.size,
    });
  }

 private:
  struct Position {
    uint32_t src_inline_offset = 0;
    uint32_t src_out_of_line_offset = 0;
    uint32_t dst_inline_offset = 0;
    uint32_t dst_out_of_line_offset = 0;
  };

  zx_status_t Transform(const fidl_type_t& type,
                        const Position& position, const uint32_t element_size) {
    switch (type.type_tag) {
    case fidl::kFidlTypePrimitive:
    case fidl::kFidlTypeEnum:
    case fidl::kFidlTypeBits:
    case fidl::kFidlTypeStructPointer:
    case fidl::kFidlTypeUnionPointer:
    case fidl::kFidlTypeString:
      // nothing to do, copy
      // would be nice to be passed "size_including_padding" to have generic handling
      // memcpy(out_bytes_ + current_out_offset_,
      //       in_bytes_ + current_in_offset_,
      //       field_size_plus_padding);
      return ZX_OK;
    case fidl::kFidlTypeStruct:
      return TransformStruct(type, position);
    case fidl::kFidlTypeUnion:
      return TransformUnion(type, position);
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
  }

  zx_status_t TransformStruct(const fidl_type_t& type, const Position& position) {
    assert(type.type_tag == fidl::kFidlTypeStruct);
    const fidl::FidlCodedStruct& coded_struct = type.coded_struct;
  
    const uint32_t last_field_index = coded_struct.field_count - 1;
    const uint32_t offset_end_of_src_struct = position.src_inline_offset + coded_struct.size;
    for (uint32_t field_index = 0; field_index <= last_field_index; field_index++) {
      const auto& src_field = coded_struct.fields[field_index];
      const auto& dst_field = coded_struct.fields[field_index];
      uint32_t src_field_offset = position.src_inline_offset + src_field.offset;
      uint32_t dst_field_offset = position.dst_inline_offset + dst_field.offset;
      uint32_t next_src_field_offset = (field_index == last_field_index) ?
          offset_end_of_src_struct :
          position.src_inline_offset + coded_struct.fields[field_index + 1].offset;
      uint32_t field_size_plus_padding = next_src_field_offset - src_field_offset;
      auto field_position = Position{
        .src_inline_offset = src_field_offset,
        .src_out_of_line_offset = position.src_out_of_line_offset,
        .dst_inline_offset = dst_field_offset,
        .dst_out_of_line_offset = position.dst_out_of_line_offset,
      };
      if (zx_status_t status = Transform(*src_field.type, field_position, field_size_plus_padding); status != ZX_OK) {
        return status;
      }
    }
    return ZX_OK;
  }

  zx_status_t TransformUnion(const fidl_type_t& type, const Position& position) {
    assert(type.type_tag == fidl::kFidlTypeUnion);
    const fidl::FidlCodedUnion& coded_union = type.coded_union;
    const fidl::FidlCodedXUnion& coded_xunion = *type.coded_union.in_v1_no_ee;

    const fidl_xunion_t* src_union_as_xunion =
      reinterpret_cast<const fidl_xunion_t*>(in_bytes_ + position.src_inline_offset);

    // look up variant type by ordinal
    const auto ordinal = reinterpret_cast<uint32_t>(src_union_as_xunion->tag);
    const fidl_type_t* variant_type = nullptr;
    for (uint32_t variant_index = 0; variant_index <= coded_xunion.field_count; variant_index++) {
      const fidl::FidlXUnionField& candidate_variant = coded_xunion.fields[variant_index];
      if (candidate_variant.ordinal == ordinal) {
        variant_type = candidate_variant.type;
        break;
      }
    }
    assert(variant_type != nullptr && "ordinal has no corresponding variant");

    // TODO: write ordinal in dst
    // TODO: align dst

    // copy out-of-line data into inline portion
    return Transform(*variant_type, Position{
      .src_inline_offset = position.src_out_of_line_offset,
      .src_out_of_line_offset = 0, // NEXT out of line object!
      .dst_inline_offset = position.dst_inline_offset + 4, // or 8 depending on alignment
      .dst_out_of_line_offset = 0, // NEXT out of line object!
    }, 0 /* WRONG */);
  }

  const uint8_t* in_bytes_;
  const uint32_t in_num_bytes_;
  uint8_t* out_bytes_;
  uint32_t* out_num_bytes_;
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
