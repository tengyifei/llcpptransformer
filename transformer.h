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
//
// Note: in order to avoid any padding in `fidl_transform` invocations, we
// define this enum as a `uint64_t`.
typedef uint64_t fidl_transformation_t;

// No-op transformation.
#define FIDL_TRANSFORMATION_NONE ((fidl_transformation_t)0u)

// In the v1 wire format, static-unions are encoded as flexible-unions.
//
// Performing this transformation will inline all static-unions into their
// container (including their data which will move from out-of-line to
// inline).
//
// See also `fidl_transform`.
#define FIDL_TRANSFORMATION_V1_TO_OLD ((fidl_transformation_t)1u)

// Transforms an encoded FIDL buffer from one wire format to another.
//
// See also `fidl_transformation_t` and `FIDL_TRANSFORMATION_...` constants.
zx_status_t fidl_transform(fidl_transformation_t transformation,
                           const fidl_type_t* type,
                           const void* src_bytes, void* dst_bytes,
                           uint32_t src_num_bytes, uint32_t* dst_num_bytes,
                           const char** out_error_msg);

// __END_CDECLS

#endif  // LIB_FIDL_TRANSFORMER_H_
