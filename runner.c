#include <stdio.h>

#include "transformer.h"

int main() {
    uint8_t in_buffer[64000];
    uint8_t out_buffer[64000];
    uint32_t out_buffer_size;
    const char* error_msg;
    fidl_type_t* type;
    zx_status_t status = fidl_transform_xunion_to_union(type,
                                                        in_buffer, 10,
                                                        out_buffer, &out_buffer_size,
                                                        &error_msg);
    printf("status: %d\n", status);
    return 0;
}