// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "transformer.h"

namespace {

class Transformer final {
};

}  // namespace

zx_status_t fidl_transform_xunion_to_union(const fidl_type_t* type,
                                           void* in_bytes, uint32_t in_num_bytes,
                                           void* out_bytes, uint32_t* out_num_bytes,
                                           const char** out_error_msg) {
  Transformer transformer;
  return ZX_OK;
}
