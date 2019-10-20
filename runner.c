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

    0xdb, 0xf0, 0xc2, 0x7f, // UnionSize8Aligned4.tag, i.e. Sandwich1.union
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.padding
    0x08, 0x00, 0x00, 0x00, // UnionSize8Aligned4.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionSize8Aligned4.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionSize8Aligned4.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich1.after
    0x00, 0x00, 0x00, 0x00, // Sandwich1.after (padding)

    0x09, 0x0a, 0x0b, 0x0c, // UnionSize8Aligned4.data, i.e. Sandwich1.union.data
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.data (padding)
};

uint8_t sandwich1_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich1.before

    0x02, 0x00, 0x00, 0x00, // UnionSize8Aligned4.tag, i.e. Sandwich1.union
    0x09, 0x0a, 0x0b, 0x0c, // UnionSize8Aligned4.data

    0x05, 0x06, 0x07, 0x08, // Sandwich1.after
};

uint8_t sandwich2_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before
    0x00, 0x00, 0x00, 0x00, // Sandwich2.before (padding)

    0xbf, 0xd3, 0xd1, 0x20, // UnionSize16Aligned4.tag, i.e. Sandwich2.union
    0x00, 0x00, 0x00, 0x00, // UnionSize16Aligned4.padding
    0x08, 0x00, 0x00, 0x00, // UnionSize16Aligned4.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionSize16Aligned4.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionSize16Aligned4.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionSize16Aligned4.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
    0x00, 0x00, 0x00, 0x00, // Sandwich2.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize16Aligned4.data, i.e. Sandwich2.union.data
    0xa4, 0xa5, 0x00, 0x00, // UnionSize16Aligned4.data [cont.] and padding
};

uint8_t sandwich2_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich2.before

    0x03, 0x00, 0x00, 0x00, // UnionSize16Aligned4.tag, i.e. Sandwich2.union
    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize16Aligned4.data
    0xa4, 0xa5, 0x00, 0x00, // UnionSize16Aligned4.data [cont.] and padding

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
};

uint8_t sandwich3_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich3.before
    0x00, 0x00, 0x00, 0x00, // Sandwich3.before (padding)

    0x9b, 0x55, 0x04, 0x34, // UnionSize24Alignement8.tag, i.e. Sandwich2.union
    0x00, 0x00, 0x00, 0x00, // UnionSize24Alignement8.padding
    0x16, 0x00, 0x00, 0x00, // UnionSize24Alignement8.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionSize24Alignement8.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionSize24Alignement8.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionSize24Alignement8.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich2.after
    0x00, 0x00, 0x00, 0x00, // Sandwich2.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize24Alignement8.data, i.e Sandwich2.union.data
    0xa4, 0xa5, 0xa6, 0xa7, // UnionSize24Alignement8.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // UnionSize24Alignement8.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // UnionSize24Alignement8.data [cont.]
};

uint8_t sandwich3_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich3.before
    0x00, 0x00, 0x00, 0x00, // Sandwich3.before (padding)

    0x03, 0x00, 0x00, 0x00, // UnionSize24Alignement8.tag, i.e. Sandwich3.union
    0x00, 0x00, 0x00, 0x00, // UnionSize24Alignement8.tag (padding)
    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize24Alignement8.data
    0xa4, 0xa5, 0xa6, 0xa7, // UnionSize24Alignement8.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // UnionSize24Alignement8.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // UnionSize24Alignement8.data [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich3.after
    0x00, 0x00, 0x00, 0x00, // Sandwich3.after (padding)
};

uint8_t sandwich4_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich4.before
    0x00, 0x00, 0x00, 0x00, // Sandwich4.before (padding)

    0x19, 0x10, 0x41, 0x5e, // UnionSize36Alignment4.tag, i.e. Sandwich4.union
    0x00, 0x00, 0x00, 0x00, // UnionSize36Alignment4.tag (padding)
    0x32, 0x00, 0x00, 0x00, // UnionSize36Alignment4.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionSize36Alignment4.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionSize36Alignment4.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionSize36Alignment4.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich4.after
    0x00, 0x00, 0x00, 0x00, // Sandwich4.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize36Alignment4.data, i.e. Sandwich4.union.data
    0xa4, 0xa5, 0xa6, 0xa7, // UnionSize36Alignment4.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // UnionSize36Alignment4.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // UnionSize36Alignment4.data [cont.]
    0xb0, 0xb1, 0xb2, 0xb3, // UnionSize36Alignment4.data [cont.]
    0xb4, 0xb5, 0xb6, 0xb7, // UnionSize36Alignment4.data [cont.]
    0xb8, 0xb9, 0xba, 0xbb, // UnionSize36Alignment4.data [cont.]
    0xbc, 0xbd, 0xbe, 0xbf, // UnionSize36Alignment4.data [cont.]
};

uint8_t sandwich4_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich4.before

    0x03, 0x00, 0x00, 0x00, // UnionSize36Alignment4.tag, i.e. Sandwich2.union
    0xa0, 0xa1, 0xa2, 0xa3, // UnionSize36Alignment4.data
    0xa4, 0xa5, 0xa6, 0xa7, // UnionSize36Alignment4.data [cont.]
    0xa8, 0xa9, 0xaa, 0xab, // UnionSize36Alignment4.data [cont.]
    0xac, 0xad, 0xae, 0xaf, // UnionSize36Alignment4.data [cont.]
    0xb0, 0xb1, 0xb2, 0xb3, // UnionSize36Alignment4.data [cont.]
    0xb4, 0xb5, 0xb6, 0xb7, // UnionSize36Alignment4.data [cont.]
    0xb8, 0xb9, 0xba, 0xbb, // UnionSize36Alignment4.data [cont.]
    0xbc, 0xbd, 0xbe, 0xbf, // UnionSize36Alignment4.data [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich4.after
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

    0x02, 0x00, 0x00, 0x00, // UnionSize8Aligned4.tag, i.e Sandwich5.UnionOfUnion.data
    0x09, 0x0a, 0x0b, 0x0c, // UnionSize8Aligned4.data
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.data (padding)
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.data (padding)
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.data (padding)
    0x00, 0x00, 0x00, 0x00, // UnionSize8Aligned4.UnionSize8Aligned4.data (padding)

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

uint8_t sandwich6_case1_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0xad, 0xcc, 0xc3, 0x79, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x06, 0x00, 0x00, 0x00, // vector<uint8>.size, i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence [cont.]

    0xa0, 0xa1, 0xa2, 0xa3, // vector<uint8>.data
    0xa4, 0xa5, 0x00, 0x00, // vector<uint8>.data [cont.] + padding
};

uint8_t sandwich6_case1_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x01, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0x06, 0x00, 0x00, 0x00, // vector<uint8>.size (start of UnionWithVector.data)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0xa0, 0xa1, 0xa2, 0xa3, // vector<uint8>.data
    0xa4, 0xa5, 0x00, 0x00, // vector<uint8>.data [cont.] + padding
};

uint8_t sandwich6_case1_absent_vector_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0xad, 0xcc, 0xc3, 0x79, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x06, 0x00, 0x00, 0x00, // vector<uint8>.size, i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.absent
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.absent [cont.]
};

uint8_t sandwich6_case1_absent_vector_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x01, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0x06, 0x00, 0x00, 0x00, // vector<uint8>.size (start of UnionWithVector.data)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.absent
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.absent [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)
};

uint8_t sandwich6_case2_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x38, 0x43, 0x31, 0x3b, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x15, 0x00, 0x00, 0x00, // vector<uint8>.size (21), i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence [cont.]

    0x73, 0x6f, 0x66, 0x74, // vector<uint8>.data
    0x20, 0x6d, 0x69, 0x67, // vector<uint8>.data [cont.]
    0x72, 0x61, 0x74, 0x69, // vector<uint8>.data [cont.]
    0x6f, 0x6e, 0x73, 0x20, // vector<uint8>.data [cont.]
    0x72, 0x6f, 0x63, 0x6b, // vector<uint8>.data [cont.]
    0x21, 0x00, 0x00, 0x00, // vector<uint8>.data [cont.] + padding
};

uint8_t sandwich6_case2_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x02, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0x15, 0x00, 0x00, 0x00, // vector<uint8>.size (start of UnionWithVector.data)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x73, 0x6f, 0x66, 0x74, // vector<uint8>.data
    0x20, 0x6d, 0x69, 0x67, // vector<uint8>.data [cont.]
    0x72, 0x61, 0x74, 0x69, // vector<uint8>.data [cont.]
    0x6f, 0x6e, 0x73, 0x20, // vector<uint8>.data [cont.]
    0x72, 0x6f, 0x63, 0x6b, // vector<uint8>.data [cont.]
    0x21, 0x00, 0x00, 0x00, // vector<uint8>.data [cont.] + padding
};

// TODO: Verify this example with GIDL. This one likely needs to look like
// case 6, i.e. due to the alignement of 1 of the struct, there is no
// paddding in between vector elements.
uint8_t sandwich6_case3_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0xdc, 0x3c, 0xc1, 0x4b, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x03, 0x00, 0x00, 0x00, // vector<struct>.size (21), i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<struct>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<struct>.presence
    0xff, 0xff, 0xff, 0xff, // vector<struct>.presence [cont.]

    0x73, 0x6f, 0x66, 0xcc, // StructSize3Alignment1 (start of vector<struct>.data)
    0x20, 0x6d, 0x69, 0xcc, // StructSize3Alignment1 (element #2)
    0x72, 0x61, 0x74, 0xcc, // StructSize3Alignment1 (element #3)
    0xcc, 0xcc, 0xcc, 0xcc, // (padding)
};

uint8_t sandwich6_case3_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x03, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0x03, 0x00, 0x00, 0x00, // vector<uint8>.size (start of UnionWithVector.data)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x73, 0x6f, 0x66, 0x00, // StructSize3Alignment1 (start of vector<struct>.data)
    0x20, 0x6d, 0x69, 0x00, // StructSize3Alignment1 (element #2)
    0x72, 0x61, 0x74, 0x00, // StructSize3Alignment1 (element #3)
    0x00, 0x00, 0x00, 0x00, // (padding)
};

uint8_t sandwich6_case4_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x3c, 0xaa, 0x08, 0x1d, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x03, 0x00, 0x00, 0x00, // vector<struct>.size (21), i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<struct>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<struct>.presence
    0xff, 0xff, 0xff, 0xff, // vector<struct>.presence [cont.]

    0x73, 0x6f, 0x66, 0xcc, // StructSize3Alignment2 (start of vector<struct>.data)
    0x20, 0x6d, 0x69, 0xcc, // StructSize3Alignment2 (element #2)
    0x72, 0x61, 0x74, 0xcc, // StructSize3Alignment2 (element #3)
    0xcc, 0xcc, 0xcc, 0xcc, // (padding)
};

uint8_t sandwich6_case4_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x04, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0x03, 0x00, 0x00, 0x00, // vector<uint8>.size (start of UnionWithVector.data)
    0x00, 0x00, 0x00, 0x00, // vector<uint8>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence
    0xff, 0xff, 0xff, 0xff, // vector<uint8>.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x73, 0x6f, 0x66, 0x00, // StructSize3Alignment2 (start of vector<struct>.data)
    0x20, 0x6d, 0x69, 0x00, // StructSize3Alignment2 (element #2)
    0x72, 0x61, 0x74, 0x00, // StructSize3Alignment2 (element #3)
    0x00, 0x00, 0x00, 0x00, // (padding)
};

uint8_t sandwich6_case5_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x76, 0xaa, 0x1e, 0x47, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0x03, 0x00, 0x00, 0x00, // vector<handle>.size, i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<handle>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<handle>.presence
    0xff, 0xff, 0xff, 0xff, // vector<handle>.presence [cont.]

    0xff, 0xff, 0xff, 0xff, // vector<handle>.data
    0xff, 0xff, 0xff, 0xff, // vector<handle>.data
    0xff, 0xff, 0xff, 0xff, // vector<handle>.data
    0xff, 0xff, 0xff, 0xff, // vector<handle>.data (padding)
};

uint8_t sandwich6_case5_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x05, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0x03, 0x00, 0x00, 0x00, // vector<handle>.size, i.e. Sandwich6.union.data
    0x00, 0x00, 0x00, 0x00, // vector<handle>.size (padding)
    0xff, 0xff, 0xff, 0xff, // vector<handle>.presence
    0xff, 0xff, 0xff, 0xff, // vector<handle>.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0xff, 0xff, 0xff, 0xff, // vector<handle>.data
    0xff, 0xff, 0xff, 0xff, // vector<handle>.data
    0xff, 0xff, 0xff, 0xff, // vector<handle>.data
    0x00, 0x00, 0x00, 0x00, // vector<handle>.data (padding)
};

uint8_t sandwich6_case6_input[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x10, 0xa8, 0xa0, 0x5e, // UnionWithVector.ordinal (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.ordinal (padding)
    0x0f, 0x00, 0x00, 0x00, // UnionWithVector.env.num_bytes
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.env.num_handle
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence
    0xff, 0xff, 0xff, 0xff, // UnionWithVector.env.presence [cont.]

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)

    0xa1, 0xa2, 0xa3, 0xa4, // array<StructSize3Alignment1>:2, i.e. Sandwich6.union.data
    0xa5, 0xa6, 0xcc, 0xcc, // array<StructSize3Alignment1>:2
};

uint8_t sandwich6_case6_expected_output[] = {
    0x01, 0x02, 0x03, 0x04, // Sandwich6.before
    0x00, 0x00, 0x00, 0x00, // Sandwich6.before (padding)

    0x06, 0x00, 0x00, 0x00, // UnionWithVector.tag (start of Sandwich6.union)
    0x00, 0x00, 0x00, 0x00, // UnionWithVector.tag (padding)
    0xa1, 0xa2, 0xa3, 0xa4, // array<StructSize3Alignment1>:2, i.e. Sandwich6.union.data
    0xa5, 0xa6, 0x00, 0x00, // array<StructSize3Alignment1>:2
    0x00, 0x00, 0x00, 0x00, // Sandwich6.union.data (padding)
    0x00, 0x00, 0x00, 0x00, // Sandwich6.union.data (padding)

    0x05, 0x06, 0x07, 0x08, // Sandwich6.after
    0x00, 0x00, 0x00, 0x00, // Sandwich6.after (padding)
};

bool run_single_test(const fidl_type_t* src_type,
                     const uint8_t* src_bytes, uint32_t src_num_bytes,
                     const uint8_t* expected_dst_bytes, uint32_t expected_dst_num_bytes,
                     const char** out_error_msg) {
    uint8_t actual_dst_bytes[ZX_CHANNEL_MAX_MSG_BYTES];
    uint32_t actual_dst_num_bytes;
    memset(actual_dst_bytes, 0xcc /* poison */, ZX_CHANNEL_MAX_MSG_BYTES);

    zx_status_t status = fidl_transform_xunion_to_union(
        src_type,
        src_bytes, src_num_bytes,
        actual_dst_bytes, &actual_dst_num_bytes,
        out_error_msg);
    if (status != ZX_OK) {
        return false;
    }
    return cmp_payload(
        actual_dst_bytes, actual_dst_num_bytes,
        expected_dst_bytes, expected_dst_num_bytes
        );
}

bool test_sandwich1(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich1Table,
        sandwich1_case1_input, sizeof(sandwich1_case1_input),
        sandwich1_case1_expected_output, sizeof(sandwich1_case1_expected_output),
        out_error_msg
    );
}

bool test_sandwich2(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich2Table,
        sandwich2_case1_input, sizeof(sandwich2_case1_input),
        sandwich2_case1_expected_output, sizeof(sandwich2_case1_expected_output),
        out_error_msg
    );
}

bool test_sandwich3(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich3Table,
        sandwich3_case1_input, sizeof(sandwich3_case1_input),
        sandwich3_case1_expected_output, sizeof(sandwich3_case1_expected_output),
        out_error_msg
    );
}

bool test_sandwich4(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich4Table,
        sandwich4_case1_input, sizeof(sandwich4_case1_input),
        sandwich4_case1_expected_output, sizeof(sandwich4_case1_expected_output),
        out_error_msg
    );
}

bool test_sandwich5_case1(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich5Table,
        sandwich5_case1_input, sizeof(sandwich5_case1_input),
        sandwich5_case1_expected_output, sizeof(sandwich5_case1_expected_output),
        out_error_msg
    );
}

bool test_sandwich5_case2(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich5Table,
        sandwich5_case2_input, sizeof(sandwich5_case2_input),
        sandwich5_case2_expected_output, sizeof(sandwich5_case2_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case1(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case1_input, sizeof(sandwich6_case1_input),
        sandwich6_case1_expected_output, sizeof(sandwich6_case1_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case1_absent_vector(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case1_absent_vector_input, sizeof(sandwich6_case1_absent_vector_input),
        sandwich6_case1_absent_vector_expected_output, sizeof(sandwich6_case1_absent_vector_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case2(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case2_input, sizeof(sandwich6_case2_input),
        sandwich6_case2_expected_output, sizeof(sandwich6_case2_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case3(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case3_input, sizeof(sandwich6_case3_input),
        sandwich6_case3_expected_output, sizeof(sandwich6_case3_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case4(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case4_input, sizeof(sandwich6_case4_input),
        sandwich6_case4_expected_output, sizeof(sandwich6_case4_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case5(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case5_input, sizeof(sandwich6_case5_input),
        sandwich6_case5_expected_output, sizeof(sandwich6_case5_expected_output),
        out_error_msg
    );
}

bool test_sandwich6_case6(const char** out_error_msg) {
    return run_single_test(
        &v1_example_Sandwich6Table,
        sandwich6_case6_input, sizeof(sandwich6_case6_input),
        sandwich6_case6_expected_output, sizeof(sandwich6_case6_expected_output),
        out_error_msg
    );
}

#define RUN(TEST_FUNC)                              \
    {                                               \
        const char* error_msg = nullptr;            \
        zx_status_t status = TEST_FUNC(&error_msg); \
        if (status == ZX_OK) {                      \
            printf("[ \033[0;31mERROR\033[0m  ]");  \
        } else {                                    \
            printf("[ \033[0;32mPASSES\033[0m ]");  \
        }                                           \
        printf(" " #TEST_FUNC);                     \
        if (error_msg) {                            \
            printf(": %s ", error_msg);             \
        }                                           \
        printf("\n");                               \
    }

int main() {
    RUN(test_sandwich1)
    RUN(test_sandwich2)
    RUN(test_sandwich3)
    RUN(test_sandwich4)
    RUN(test_sandwich5_case1)
    RUN(test_sandwich5_case2)
    RUN(test_sandwich6_case1)
    RUN(test_sandwich6_case1_absent_vector)
    RUN(test_sandwich6_case2)
    RUN(test_sandwich6_case3)
    RUN(test_sandwich6_case4)
    RUN(test_sandwich6_case5)
    RUN(test_sandwich6_case6)
    return 0;
}