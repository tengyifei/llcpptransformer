// WARNING: This file is machine generated by fidlc.

#include <lib/fidl/internal.h>

extern "C" {

extern const fidl_type_t example_UnionSize8Aligned4Table;
extern const fidl_type_t example_Sandwich1Table;
extern const fidl_type_t example_UnionSize16Aligned4Table;
extern const fidl_type_t example_Sandwich3Table;
extern const fidl_type_t example_Sandwich2Table;
extern const fidl_type_t example_StructSize16Alignement8Table;
extern const fidl_type_t example_UnionSize24Alignement8Table;



static const ::fidl::FidlUnionField Fields26example_UnionSize8Aligned4[] = {
    ::fidl::FidlUnionField(nullptr, 0u)
};
const fidl_type_t example_UnionSize8Aligned4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields26example_UnionSize8Aligned4, 1u, 4u, 8u, "example/UnionSize8Aligned4"));

static const ::fidl::FidlStructField Fields17example_Sandwich1[] = {
    ::fidl::FidlStructField(&example_UnionSize8Aligned4Table, 4u, 0u)
};
const fidl_type_t example_Sandwich1Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich1, 1u, 16u, "example/Sandwich1"));

static const ::fidl::FidlUnionField Fields27example_UnionSize16Aligned4[] = {
    ::fidl::FidlUnionField(nullptr, 2u)
};
const fidl_type_t example_UnionSize16Aligned4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields27example_UnionSize16Aligned4, 1u, 4u, 12u, "example/UnionSize16Aligned4"));

static const ::fidl::FidlStructField Fields17example_Sandwich3[] = {
    ::fidl::FidlStructField(&example_UnionSize16Aligned4Table, 4u, 0u)
};
const fidl_type_t example_Sandwich3Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich3, 1u, 20u, "example/Sandwich3"));

static const ::fidl::FidlStructField Fields17example_Sandwich2[] = {
    ::fidl::FidlStructField(&example_UnionSize16Aligned4Table, 4u, 0u)
};
const fidl_type_t example_Sandwich2Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich2, 1u, 20u, "example/Sandwich2"));

static const ::fidl::FidlStructField Fields31example_StructSize16Alignement8[] = {};
const fidl_type_t example_StructSize16Alignement8Table = fidl_type_t(::fidl::FidlCodedStruct(Fields31example_StructSize16Alignement8, 0u, 16u, "example/StructSize16Alignement8"));

static const ::fidl::FidlUnionField Fields30example_UnionSize24Alignement8[] = {
    ::fidl::FidlUnionField(&example_StructSize16Alignement8Table, 0u)
};
const fidl_type_t example_UnionSize24Alignement8Table = fidl_type_t(::fidl::FidlCodedUnion(Fields30example_UnionSize24Alignement8, 1u, 8u, 24u, "example/UnionSize24Alignement8"));

} // extern "C"
