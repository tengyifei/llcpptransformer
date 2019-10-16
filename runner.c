#include <stdio.h>

#include "transformer.h"
#include "tables.h"

zx_status_t test_sandwich1() {
    uint8_t in_buffer[ZX_CHANNEL_MAX_MSG_BYTES];
    uint8_t out_buffer[ZX_CHANNEL_MAX_MSG_BYTES];
    uint32_t out_buffer_size;
    const char* error_msg;
    zx_status_t status = fidl_transform_xunion_to_union(&example_Sandwich1Table,
                                                        in_buffer, 10,
                                                        out_buffer, &out_buffer_size,
                                                        &error_msg);
    return status;
}

int main() {
    zx_status_t status = test_sandwich1();
    printf("test_sandwich1: %d\n", status);
    return 0;
}