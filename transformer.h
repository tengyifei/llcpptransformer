// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FIDL_TRANSFORMER_H_
#define LIB_FIDL_TRANSFORMER_H_

// #include <zircon/compiler.h>
#include "fidl.h"
// #include <zircon/types.h>

// __BEGIN_CDECLS

// works on encoded messages
// on the wire xunions get converted to static unions
// out buffer should be of at least the size of type->format2->(size + max_out_of_line)
zx_status_t fidl_transform_xunion_to_union(const fidl_type_t* type,
                                           const void* in_bytes, uint32_t in_num_bytes,
                                           void* out_bytes, uint32_t* out_num_bytes,
                                           const char** out_error_msg);

// __END_CDECLS

#endif  // LIB_FIDL_TRANSFORMER_H_
