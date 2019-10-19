#include <stdio.h>
#include <iostream>

#include "transformer.h"
#include "tables.h"

bool cmp_payload(const uint8_t* actual, size_t actual_size, const uint8_t* expected,
                 size_t expected_size) {
  bool pass = true;
  for (size_t i = 0; i < actual_size && i < expected_size; i++) {
    if (actual[i] != expected[i]) {
      pass = false;
      std::cout << std::dec << "element[" << i << "]: " << std::hex << "actual=0x" << +actual[i]
                << " "
                << "expected=0x" << +expected[i] << "\n";
    }
  }
  if (actual_size != expected_size) {
    pass = false;
    std::cout << std::dec << "element[...]: "
              << "actual.size=" << +actual_size << " "
              << "expected.size=" << +expected_size << "\n";
  }
  return pass;
}

uint8_t sandwich1_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich1.before
    0x00, 0x00, 0x00, 0x00, // Sandwich1.before (padding)

    0xdb, 0xf0, 0xc2, 0x7f, // Sandwich1.union.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich1.union.padding
    0x08, 0x00, 0x00, 0x00, // Sandwich1.union.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // Sandwich1.union.env.num_handle
    0xff, 0xff, 0xff, 0xff, // Sandwich1.union.env.presence
    0xff, 0xff, 0xff, 0xff, // Sandwich1.union.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich1.after
    0x00, 0x00, 0x00, 0x00, // Sandwich1.after (padding)

    0x09, 0x0a, 0x0b, 0x0c, // Sandwich1.union.data
    0x00, 0x00, 0x00, 0x00, // Sandwich1.union.data (padding)
};

uint8_t sandwich1_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich1.before

    0x01, 0x00, 0x00, 0x00, // Sandwich1.union.tag
    0x09, 0x0a, 0x0b, 0x0c, // Sandwich1.union.data

    0x05, 0x06, 0x07, 0x08, // Sandwich1.after
};

bool test_sandwich1() {
    uint8_t in_buffer[ZX_CHANNEL_MAX_MSG_BYTES];
    uint8_t out_buffer[ZX_CHANNEL_MAX_MSG_BYTES];
    uint32_t out_buffer_size;
    const char* error_msg;
    zx_status_t status = fidl_transform_xunion_to_union(
        &v1_example_Sandwich1Table,
        sandwich1_case1_input, sizeof(sandwich1_case1_input),
        out_buffer, &out_buffer_size,
        &error_msg);
    if (status != ZX_OK) {
        return false;
    }
    return cmp_payload(
        out_buffer, out_buffer_size,
        sandwich1_case1_expected_output, sizeof(sandwich1_case1_expected_output)
        );
}

int main() {
    zx_status_t status = test_sandwich1();
    printf("test_sandwich1: %s\n", status);
    return 0;
}