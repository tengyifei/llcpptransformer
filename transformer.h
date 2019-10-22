// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FIDL_TRANSFORMER_H_
#define LIB_FIDL_TRANSFORMER_H_

// #include <zircon/compiler.h>
#include "fidl.h"
// #include <zircon/types.h>

// __BEGIN_CDECLS

// Available transformations.
enum FidlTransformation {
    // No-op transformation.
    NONE = 0,

    // In the v1 wire format, static-unions are encoded as flexible-unions.
    //
    // Performing this transformation will inline all static-unions into their
    // container (including their data which will move from out-of-line to
    // inline).
    V1_TO_OLD = 1,
};

// Transforms an encoded FIDL buffer from one wire format to another. See the
// `Trasnformation` enum for supported transformations.
zx_status_t fidl_transform(FidlTransformation transformation,
                           const fidl_type_t* type,
                           const void* src_bytes, uint32_t src_num_bytes,
                           void* dst_bytes, uint32_t* dst_num_bytes,
                           const char** out_error_msg);

// __END_CDECLS

#endif  // LIB_FIDL_TRANSFORMER_H_
