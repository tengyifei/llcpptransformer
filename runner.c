#include <stdio.h>
#include <iostream>
#include <cstring>

#include "transformer.h"
#include "tables.h"
#include "fidl.h"

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

    0x02, 0x00, 0x00, 0x00, // Sandwich1.union.tag
    0x09, 0x0a, 0x0b, 0x0c, // Sandwich1.union.data

    0x05, 0x06, 0x07, 0x08, // Sandwich1.after
};

uint8_t sandwich2_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before
    0x00, 0x00, 0x00, 0x00, // Sandwich2.before (padding)

    0xbf, 0xd3, 0xd1, 0x20, // Sandwich2.union.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.padding
    0x08, 0x00, 0x00, 0x00, // Sandwich2.union.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.env.num_handle
    0xff, 0xff, 0xff, 0xff, // Sandwich2.union.env.presence
    0xff, 0xff, 0xff, 0xff, // Sandwich2.union.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
    0x00, 0x00, 0x00, 0x00, // Sandwich2.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // Sandwich2.union.data
    0xa4, 0xa5, 0x00, 0x00, // Sandwich2.union.data [cont.] and padding
};

uint8_t sandwich2_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before

    0x03, 0x00, 0x00, 0x00, // Sandwich2.union.tag
    0xa0, 0xa1, 0xa2, 0xa3, // Sandwich2.union.data
    0xa4, 0xa5, 0x00, 0x00, // Sandwich2.union.data [cont.] and padding

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
};

uint8_t sandwich3_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before
    0x00, 0x00, 0x00, 0x00, // Sandwich2.before (padding)

    0x9b, 0x55, 0x04, 0x34, // Sandwich2.union.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.padding
    0x16, 0x00, 0x00, 0x00, // Sandwich2.union.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.env.num_handle
    0xff, 0xff, 0xff, 0xff, // Sandwich2.union.env.presence
    0xff, 0xff, 0xff, 0xff, // Sandwich2.union.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
    0x00, 0x00, 0x00, 0x00, // Sandwich2.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // Sandwich2.union.data
    0xa4, 0xa5, 0xa6, 0xa7, // Sandwich2.union.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // Sandwich2.union.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // Sandwich2.union.data [cont.]
};

uint8_t sandwich3_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before
    0x00, 0x00, 0x00, 0x00, // Sandwich2.before (padding)

    0x03, 0x00, 0x00, 0x00, // Sandwich2.union.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.tag (padding)
    0xa0, 0xa1, 0xa2, 0xa3, // Sandwich2.union.data
    0xa4, 0xa5, 0xa6, 0xa7, // Sandwich2.union.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // Sandwich2.union.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // Sandwich2.union.data [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
    0x00, 0x00, 0x00, 0x00, // Sandwich2.after (padding)
};

uint8_t sandwich4_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before
    0x00, 0x00, 0x00, 0x00, // Sandwich2.before (padding)

    0x19, 0x10, 0x41, 0x5e, // Sandwich2.union.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.padding
    0x32, 0x00, 0x00, 0x00, // Sandwich2.union.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // Sandwich2.union.env.num_handle
    0xff, 0xff, 0xff, 0xff, // Sandwich2.union.env.presence
    0xff, 0xff, 0xff, 0xff, // Sandwich2.union.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
    0x00, 0x00, 0x00, 0x00, // Sandwich2.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // Sandwich2.union.data
    0xa4, 0xa5, 0xa6, 0xa7, // Sandwich2.union.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // Sandwich2.union.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // Sandwich2.union.data [cont.]
    0xb0, 0xb1, 0xb2, 0xb3, // Sandwich2.union.data [cont.]
    0xb4, 0xb5, 0xb6, 0xb7, // Sandwich2.union.data [cont.]
    0xb8, 0xb9, 0xba, 0xbb, // Sandwich2.union.data [cont.]
    0xbc, 0xbd, 0xbe, 0xbf, // Sandwich2.union.data [cont.]
};

uint8_t sandwich4_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before

    0x03, 0x00, 0x00, 0x00, // Sandwich2.union.tag
    0xa0, 0xa1, 0xa2, 0xa3, // Sandwich2.union.data
    0xa4, 0xa5, 0xa6, 0xa7, // Sandwich2.union.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // Sandwich2.union.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // Sandwich2.union.data [cont.]
    0xb0, 0xb1, 0xb2, 0xb3, // Sandwich2.union.data [cont.]
    0xb4, 0xb5, 0xb6, 0xb7, // Sandwich2.union.data [cont.]
    0xb8, 0xb9, 0xba, 0xbb, // Sandwich2.union.data [cont.]
    0xbc, 0xbd, 0xbe, 0xbf, // Sandwich2.union.data [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
};

uint8_t sandwich5_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich5.before
    0x00, 0x00, 0x00, 0x00, // Sandwich5.before (padding)

    0x60, 0xdd, 0xaa, 0x20, // Sandwich5.UnionOfUnion.ordinal
    0x00, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.padding
    0x32, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.env.num_handle
    0xff, 0xff, 0xff, 0xff, // Sandwich5.UnionOfUnion.env.presence
    0xff, 0xff, 0xff, 0xff, // Sandwich5.UnionOfUnion.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich5.after
    0x00, 0x00, 0x00, 0x00, // Sandwich5.after (padding)

    0xdb, 0xf0, 0xc2, 0x7f, // UnionOfUnion.UnionSize8Aligned4.ordinal
    0x00, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize8Aligned4.padding
    0x08, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize8Aligned4.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize8Aligned4.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionOfUnion.UnionSize8Aligned4.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionOfUnion.UnionSize8Aligned4.env.presence [cont.]

    0x09, 0x0a, 0x0b, 0x0c, // UnionOfUnion.UnionSize8Aligned4.data
    0x00, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize8Aligned4.data (padding)
};

uint8_t sandwich5_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich5.before
    0x00, 0x00, 0x00, 0x00, // Sandwich5.before (padding)

    0x01, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.tag (padding)

    0x02, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize8Aligned4.tag, i.e Sandwich5.UnionOfUnion.data
    0x09, 0x0a, 0x0b, 0x0c, // UnionOfUnion.UnionSize8Aligned4.data
    0x00, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize8Aligned4.data (padding)

    0x05, 0x06, 0x07, 0x08, // Sandwich5.after
    0x00, 0x00, 0x00, 0x00, // Sandwich5.after (padding)
};

uint8_t sandwich5_case2_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich5.before
    0x00, 0x00, 0x00, 0x00, // Sandwich5.before (padding)

    0x1f, 0x2d, 0x72, 0x06, // Sandwich5.UnionOfUnion.ordinal
    0x00, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.padding
    0x32, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.env.num_handle
    0xff, 0xff, 0xff, 0xff, // Sandwich5.UnionOfUnion.env.presence
    0xff, 0xff, 0xff, 0xff, // Sandwich5.UnionOfUnion.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich5.after
    0x00, 0x00, 0x00, 0x00, // Sandwich5.after (padding)

    0x9b, 0x55, 0x04, 0x34, // UnionOfUnion.UnionSize24Alignement8.ordinal
    0x00, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize24Alignement8.padding
    0x16, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize24Alignement8.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionOfUnion.UnionSize24Alignement8.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionOfUnion.UnionSize24Alignement8.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionOfUnion.UnionSize24Alignement8.env.presence [cont.]

    0xa0, 0xa1, 0xa2, 0xa3, // UnionOfUnion.UnionSize24Alignement8.data
    0xa4, 0xa5, 0xa6, 0xa7, // UnionOfUnion.UnionSize24Alignement8.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // UnionOfUnion.UnionSize24Alignement8.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // UnionOfUnion.UnionSize24Alignement8.data [cont.]
};

uint8_t sandwich5_case2_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich5.before
    0x00, 0x00, 0x00, 0x00, // Sandwich5.before (padding)

    0x03, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.tag
    0x00, 0x00, 0x00, 0x00, // Sandwich5.UnionOfUnion.tag (padding)

    0x03, 0x00, 0x00, 0x00, // UnionSize24Alignement8.tag, i.e Sandwich5.UnionOfUnion.data
    0x00, 0x00, 0x00, 0x00, // UnionSize24Alignement8.tag (padding)
    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize24Alignement8.data
    0xa4, 0xa5, 0xa6, 0xa7, // UnionSize24Alignement8.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // UnionSize24Alignement8.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // UnionSize24Alignement8.data [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich5.after
    0x00, 0x00, 0x00, 0x00, // Sandwich5.after (padding)
};

bool run_single_test(const fidl_type_t* src_type,
                     const uint8_t* src_bytes, uint32_t src_num_bytes,
                     const uint8_t* expected_dst_bytes, uint32_t expected_dst_num_bytes) {
    printf("----------------\n");

    uint8_t actual_dst_bytes[ZX_CHANNEL_MAX_MSG_BYTES];
    uint32_t actual_dst_num_bytes;
    memset(actual_dst_bytes, 0xcc /* poison */, ZX_CHANNEL_MAX_MSG_BYTES);

    const char* error_msg;

    zx_status_t status = fidl_transform_xunion_to_union(
        src_type,
        src_bytes, src_num_bytes,
        actual_dst_bytes, &actual_dst_num_bytes,
        &error_msg);
    if (status != ZX_OK) {
        return false;
    }
    return cmp_payload(
        actual_dst_bytes, actual_dst_num_bytes,
        expected_dst_bytes, expected_dst_num_bytes
        );
}

bool test_sandwich1() {
    return run_single_test(
        &v1_example_Sandwich1Table,
        sandwich1_case1_input, sizeof(sandwich1_case1_input),
        sandwich1_case1_expected_output, sizeof(sandwich1_case1_expected_output)
    );
}

bool test_sandwich2() {
    return run_single_test(
        &v1_example_Sandwich2Table,
        sandwich2_case1_input, sizeof(sandwich2_case1_input),
        sandwich2_case1_expected_output, sizeof(sandwich2_case1_expected_output)
    );
}

bool test_sandwich3() {
    return run_single_test(
        &v1_example_Sandwich3Table,
        sandwich3_case1_input, sizeof(sandwich3_case1_input),
        sandwich3_case1_expected_output, sizeof(sandwich3_case1_expected_output)
    );
}

bool test_sandwich4() {
    return run_single_test(
        &v1_example_Sandwich4Table,
        sandwich4_case1_input, sizeof(sandwich4_case1_input),
        sandwich4_case1_expected_output, sizeof(sandwich4_case1_expected_output)
    );
}

bool test_sandwich5_case1() {
    return run_single_test(
        &v1_example_Sandwich5Table,
        sandwich5_case1_input, sizeof(sandwich5_case1_input),
        sandwich5_case1_expected_output, sizeof(sandwich5_case1_expected_output)
    );
}

int main() {
    {
        zx_status_t status = test_sandwich1();
        printf("test_sandwich1: %d\n", status);
    }
    {
        zx_status_t status = test_sandwich2();
        printf("test_sandwich2: %d\n", status);
    }
    {
        zx_status_t status = test_sandwich3();
        printf("test_sandwich3: %d\n", status);
    }
    {
        zx_status_t status = test_sandwich4();
        printf("test_sandwich4: %d\n", status);
    }
    {
        zx_status_t status = test_sandwich5_case1();
        printf("test_sandwich5_case1: %d\n", status);
    }
    {
        zx_status_t status = test_sandwich5_case2();
        printf("test_sandwich5_case2: %d\n", status);
    }
    return 0;
}