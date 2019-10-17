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

  zx_status_t Transform(const fidl_type_t& type, const uint32_t element_size) {
    switch (type.type_tag) {
    case fidl::kFidlTypePrimitive:
    case fidl::kFidlTypeEnum:
    case fidl::kFidlTypeBits:
    case fidl::kFidlTypeStructPointer:
    case fidl::kFidlTypeUnionPointer:
    case fidl::kFidlTypeString: {
      // nothing to do, copy
      // would be nice to be passed "size_including_padding" to have generic handling
      // memcpy(out_bytes_ + current_out_offset_,
      //       in_bytes_ + current_in_offset_,
      //       field_size_plus_padding);
      // current_out_offset_ += field_size_plus_padding;
      current_in_offset_ += element_size;
      return ZX_OK;
    }
    case fidl::kFidlTypeStruct:
    case fidl::kFidlTypeUnion:
    case fidl::kFidlTypeArray:
    case fidl::kFidlTypeHandle:
    case fidl::kFidlTypeVector:
    case fidl::kFidlTypeTable:
    case fidl::kFidlTypeXUnion:
      assert(false && "bug: should not be called with element_size");
    }
    return ZX_ERR_BAD_STATE;
  }

  zx_status_t Transform(const fidl_type_t& type, const Position& position) {
    switch (type.type_tag) {
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

    case fidl::kFidlTypePrimitive:
    case fidl::kFidlTypeEnum:
    case fidl::kFidlTypeBits:
    case fidl::kFidlTypeStructPointer:
    case fidl::kFidlTypeUnionPointer:
    case fidl::kFidlTypeString:
      assert(false && "bug: should not be called with position");
    }
    return ZX_ERR_BAD_STATE;
  }

  zx_status_t TransformStruct(const fidl_type_t& type, const Position& position) {
    assert(type.type_tag == fidl::kFidlTypeStruct);
    const fidl::FidlCodedStruct& coded_struct = type.coded_struct;
  
    const uint32_t last_field_index = coded_struct.field_count - 1;
    const uint32_t offset_start_of_struct = current_in_offset_;
    const uint32_t offset_end_of_struct = current_in_offset_ + coded_struct.size;
    for (uint32_t field_index = 0; field_index <= last_field_index; field_index++) {
      const auto& field = coded_struct.fields[field_index];
      uint32_t next_field_offset = (field_index == last_field_index) ?
          offset_end_of_struct :
          offset_start_of_struct + coded_struct.fields[field_index + 1].offset;
      uint32_t field_size_plus_padding = next_field_offset - current_in_offset_;
      if (zx_status_t status = Transform(*field.type, field_size_plus_padding); status != ZX_OK) {
        return status;
      }
    }
    return ZX_OK;
  }

  zx_status_t TransformUnion(const fidl_type_t& type, const Position& position) {
    assert(type.type_tag == fidl::kFidlTypeUnion);
    const fidl::FidlCodedUnion& coded_union = type.coded_union;
    const fidl::FidlCodedXUnion& coded_xunion = *type.coded_union.in_v1_no_ee;

    const fidl_xunion_t* union_as_xunion =
      reinterpret_cast<const fidl_xunion_t*>(in_bytes_ + current_in_offset_);

    // look up variant type by ordinal
    const auto ordinal = reinterpret_cast<uint32_t>(union_as_xunion->tag);
    const fidl_type_t* variant_type = nullptr;
    for (uint32_t variant_index = 0; variant_index <= coded_xunion.field_count; variant_index++) {
      const fidl::FidlXUnionField& candidate_variant = coded_xunion.fields[variant_index];
      if (candidate_variant.ordinal == ordinal) {
        variant_type = candidate_variant.type;
        break;
      }
    }
    assert(variant_type != nullptr && "ordinal has no corresponding variant");

    // copy out-of-line data into inline portion
    Transform(*variant_type, Position{
      .src_inline_offset = position.src_out_of_line_offset,
      .src_out_of_line_offset = 0, // NEXT out of line object!
      .dst_inline_offset = position.dst_inline_offset + 4, // or 8 depending on alignment
      .dst_out_of_line_offset = 0, // NEXT out of line object!
    });

    return ZX_OK;
  }

  uint32_t current_in_offset_ = 0;
  uint32_t current_out_offset_ = 0;
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
