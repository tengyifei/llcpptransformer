// WARNING: This file is machine generated by fidlc.

#include "fidl.h"

extern "C" {

// old

extern const fidl_type_t example_UnionSize8Aligned4Table;
extern const fidl_type_t example_Sandwich1Table;
extern const fidl_type_t example_UnionSize36Alignment4Table;
extern const fidl_type_t example_Sandwich4Table;
extern const fidl_type_t example_UnionSize16Aligned4Table;
extern const fidl_type_t example_Sandwich2Table;
extern const fidl_type_t example_StructSize16Alignement8Table;
extern const fidl_type_t example_UnionSize24Alignement8Table;
extern const fidl_type_t example_UnionOfUnionTable;
extern const fidl_type_t example_Sandwich5Table;
extern const fidl_type_t example_Sandwich3Table;



static const ::fidl::FidlUnionField Fields26example_UnionSize8Aligned4[] = {
    ::fidl::FidlUnionField(nullptr, 3u, 964920088u),
    ::fidl::FidlUnionField(nullptr, 3u, 1734933826u),
    ::fidl::FidlUnionField(nullptr, 0u, 2143482075u)
};
extern inline const ::fidl::FidlCodedUnion* const example_UnionSize8Aligned4Table_alt;
const fidl_type_t example_UnionSize8Aligned4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields26example_UnionSize8Aligned4, 3u, 4u, 8u, "example/UnionSize8Aligned4", example_UnionSize8Aligned4Table_alt));

extern inline const ::fidl::FidlStructField* const Fields17example_Sandwich1_field1_alt_field;
static const ::fidl::FidlStructField Fields17example_Sandwich1[] = {
    ::fidl::FidlStructField(&example_UnionSize8Aligned4Table, 4u, 0u, Fields17example_Sandwich1_field1_alt_field)
};
extern inline const ::fidl::FidlCodedStruct* const example_Sandwich1Table_alt;
const fidl_type_t example_Sandwich1Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich1, 1u, 16u, "example/Sandwich1", example_Sandwich1Table_alt));

static const ::fidl::FidlUnionField Fields29example_UnionSize36Alignment4[] = {
    ::fidl::FidlUnionField(nullptr, 31u, 1946634093u),
    ::fidl::FidlUnionField(nullptr, 31u, 627860762u),
    ::fidl::FidlUnionField(nullptr, 31u, 79574741u),
    ::fidl::FidlUnionField(nullptr, 0u, 1581322265u)
};
extern inline const ::fidl::FidlCodedUnion* const example_UnionSize36Alignment4Table_alt;
const fidl_type_t example_UnionSize36Alignment4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields29example_UnionSize36Alignment4, 4u, 4u, 36u, "example/UnionSize36Alignment4", example_UnionSize36Alignment4Table_alt));

extern inline const ::fidl::FidlStructField* const Fields17example_Sandwich4_field1_alt_field;
static const ::fidl::FidlStructField Fields17example_Sandwich4[] = {
    ::fidl::FidlStructField(&example_UnionSize36Alignment4Table, 4u, 0u, Fields17example_Sandwich4_field1_alt_field)
};
extern inline const ::fidl::FidlCodedStruct* const example_Sandwich4Table_alt;
const fidl_type_t example_Sandwich4Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich4, 1u, 44u, "example/Sandwich4", example_Sandwich4Table_alt));

static const ::fidl::FidlUnionField Fields27example_UnionSize16Aligned4[] = {
    ::fidl::FidlUnionField(nullptr, 7u, 1136806121u),
    ::fidl::FidlUnionField(nullptr, 7u, 1657784343u),
    ::fidl::FidlUnionField(nullptr, 7u, 1244121714u),
    ::fidl::FidlUnionField(nullptr, 2u, 550622143u)
};
extern inline const ::fidl::FidlCodedUnion* const example_UnionSize16Aligned4Table_alt;
const fidl_type_t example_UnionSize16Aligned4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields27example_UnionSize16Aligned4, 4u, 4u, 12u, "example/UnionSize16Aligned4", example_UnionSize16Aligned4Table_alt));

extern inline const ::fidl::FidlStructField* const Fields17example_Sandwich2_field1_alt_field;
static const ::fidl::FidlStructField Fields17example_Sandwich2[] = {
    ::fidl::FidlStructField(&example_UnionSize16Aligned4Table, 4u, 0u, Fields17example_Sandwich2_field1_alt_field)
};
extern inline const ::fidl::FidlCodedStruct* const example_Sandwich2Table_alt;
const fidl_type_t example_Sandwich2Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich2, 1u, 20u, "example/Sandwich2", example_Sandwich2Table_alt));

static const ::fidl::FidlStructField Fields31example_StructSize16Alignement8[] = {};
extern inline const ::fidl::FidlCodedStruct* const example_StructSize16Alignement8Table_alt;
const fidl_type_t example_StructSize16Alignement8Table = fidl_type_t(::fidl::FidlCodedStruct(Fields31example_StructSize16Alignement8, 0u, 16u, "example/StructSize16Alignement8", example_StructSize16Alignement8Table_alt));

static const ::fidl::FidlUnionField Fields30example_UnionSize24Alignement8[] = {
    ::fidl::FidlUnionField(nullptr, 15u, 621982873u),
    ::fidl::FidlUnionField(nullptr, 15u, 1544195805u),
    ::fidl::FidlUnionField(nullptr, 15u, 1885474715u),
    ::fidl::FidlUnionField(&example_StructSize16Alignement8Table, 0u, 872699291u)
};
extern inline const ::fidl::FidlCodedUnion* const example_UnionSize24Alignement8Table_alt;
const fidl_type_t example_UnionSize24Alignement8Table = fidl_type_t(::fidl::FidlCodedUnion(Fields30example_UnionSize24Alignement8, 4u, 8u, 24u, "example/UnionSize24Alignement8", example_UnionSize24Alignement8Table_alt));

static const ::fidl::FidlUnionField Fields20example_UnionOfUnion[] = {
    ::fidl::FidlUnionField(nullptr, 23u, 1201318480u),
    ::fidl::FidlUnionField(&example_UnionSize8Aligned4Table, 16u, 548068704u),
    ::fidl::FidlUnionField(&example_UnionSize16Aligned4Table, 12u, 762734029u),
    ::fidl::FidlUnionField(&example_UnionSize24Alignement8Table, 0u, 108145951u)
};
extern inline const ::fidl::FidlCodedUnion* const example_UnionOfUnionTable_alt;
const fidl_type_t example_UnionOfUnionTable = fidl_type_t(::fidl::FidlCodedUnion(Fields20example_UnionOfUnion, 4u, 8u, 32u, "example/UnionOfUnion", example_UnionOfUnionTable_alt));

extern inline const ::fidl::FidlStructField* const Fields17example_Sandwich5_field1_alt_field;
static const ::fidl::FidlStructField Fields17example_Sandwich5[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&example_UnionOfUnionTable, 8u, 0u, Fields17example_Sandwich5_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 44u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const example_Sandwich5Table_alt;
const fidl_type_t example_Sandwich5Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich5, 3u, 48u, "example/Sandwich5", example_Sandwich5Table_alt));

extern inline const ::fidl::FidlStructField* const Fields17example_Sandwich3_field1_alt_field;
static const ::fidl::FidlStructField Fields17example_Sandwich3[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&example_UnionSize24Alignement8Table, 8u, 0u, Fields17example_Sandwich3_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 36u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const example_Sandwich3Table_alt;
const fidl_type_t example_Sandwich3Table = fidl_type_t(::fidl::FidlCodedStruct(Fields17example_Sandwich3, 3u, 40u, "example/Sandwich3", example_Sandwich3Table_alt));

// v1

extern const fidl_type_t v1_example_UnionSize8Aligned4Table;
extern const fidl_type_t v1_example_Sandwich1Table;
extern const fidl_type_t v1_example_UnionSize36Alignment4Table;
extern const fidl_type_t v1_example_Sandwich4Table;
extern const fidl_type_t v1_example_UnionSize16Aligned4Table;
extern const fidl_type_t v1_example_Sandwich2Table;
extern const fidl_type_t v1_example_StructSize16Alignement8Table;
extern const fidl_type_t v1_example_UnionSize24Alignement8Table;
extern const fidl_type_t v1_example_UnionOfUnionTable;
extern const fidl_type_t v1_example_Sandwich5Table;
extern const fidl_type_t v1_example_Sandwich3Table;



static const ::fidl::FidlUnionField Fields29v1_example_UnionSize8Aligned4[] = {
    ::fidl::FidlUnionField(nullptr, 7u, 964920088u),
    ::fidl::FidlUnionField(nullptr, 7u, 1734933826u),
    ::fidl::FidlUnionField(nullptr, 4u, 2143482075u)
};
extern inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize8Aligned4Table_alt;
const fidl_type_t v1_example_UnionSize8Aligned4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields29v1_example_UnionSize8Aligned4, 3u, 8u, 24u, "example/UnionSize8Aligned4", v1_example_UnionSize8Aligned4Table_alt));

extern inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich1_field1_alt_field;
static const ::fidl::FidlStructField Fields20v1_example_Sandwich1[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&v1_example_UnionSize8Aligned4Table, 8u, 0u, Fields20v1_example_Sandwich1_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 36u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich1Table_alt;
const fidl_type_t v1_example_Sandwich1Table = fidl_type_t(::fidl::FidlCodedStruct(Fields20v1_example_Sandwich1, 3u, 40u, "example/Sandwich1", v1_example_Sandwich1Table_alt));

static const ::fidl::FidlUnionField Fields32v1_example_UnionSize36Alignment4[] = {
    ::fidl::FidlUnionField(nullptr, 7u, 1946634093u),
    ::fidl::FidlUnionField(nullptr, 7u, 627860762u),
    ::fidl::FidlUnionField(nullptr, 7u, 79574741u),
    ::fidl::FidlUnionField(nullptr, 0u, 1581322265u)
};
extern inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize36Alignment4Table_alt;
const fidl_type_t v1_example_UnionSize36Alignment4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields32v1_example_UnionSize36Alignment4, 4u, 8u, 24u, "example/UnionSize36Alignment4", v1_example_UnionSize36Alignment4Table_alt));

extern inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich4_field1_alt_field;
static const ::fidl::FidlStructField Fields20v1_example_Sandwich4[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&v1_example_UnionSize36Alignment4Table, 8u, 0u, Fields20v1_example_Sandwich4_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 36u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich4Table_alt;
const fidl_type_t v1_example_Sandwich4Table = fidl_type_t(::fidl::FidlCodedStruct(Fields20v1_example_Sandwich4, 3u, 40u, "example/Sandwich4", v1_example_Sandwich4Table_alt));

static const ::fidl::FidlUnionField Fields30v1_example_UnionSize16Aligned4[] = {
    ::fidl::FidlUnionField(nullptr, 7u, 1136806121u),
    ::fidl::FidlUnionField(nullptr, 7u, 1657784343u),
    ::fidl::FidlUnionField(nullptr, 7u, 1244121714u),
    ::fidl::FidlUnionField(nullptr, 2u, 550622143u)
};
extern inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize16Aligned4Table_alt;
const fidl_type_t v1_example_UnionSize16Aligned4Table = fidl_type_t(::fidl::FidlCodedUnion(Fields30v1_example_UnionSize16Aligned4, 4u, 8u, 24u, "example/UnionSize16Aligned4", v1_example_UnionSize16Aligned4Table_alt));

extern inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich2_field1_alt_field;
static const ::fidl::FidlStructField Fields20v1_example_Sandwich2[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&v1_example_UnionSize16Aligned4Table, 8u, 0u, Fields20v1_example_Sandwich2_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 36u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich2Table_alt;
const fidl_type_t v1_example_Sandwich2Table = fidl_type_t(::fidl::FidlCodedStruct(Fields20v1_example_Sandwich2, 3u, 40u, "example/Sandwich2", v1_example_Sandwich2Table_alt));

static const ::fidl::FidlStructField Fields34v1_example_StructSize16Alignement8[] = {};
extern inline const ::fidl::FidlCodedStruct* const v1_example_StructSize16Alignement8Table_alt;
const fidl_type_t v1_example_StructSize16Alignement8Table = fidl_type_t(::fidl::FidlCodedStruct(Fields34v1_example_StructSize16Alignement8, 0u, 16u, "example/StructSize16Alignement8", v1_example_StructSize16Alignement8Table_alt));

static const ::fidl::FidlUnionField Fields33v1_example_UnionSize24Alignement8[] = {
    ::fidl::FidlUnionField(nullptr, 7u, 621982873u),
    ::fidl::FidlUnionField(nullptr, 7u, 1544195805u),
    ::fidl::FidlUnionField(nullptr, 7u, 1885474715u),
    ::fidl::FidlUnionField(&v1_example_StructSize16Alignement8Table, 0u, 872699291u)
};
extern inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize24Alignement8Table_alt;
const fidl_type_t v1_example_UnionSize24Alignement8Table = fidl_type_t(::fidl::FidlCodedUnion(Fields33v1_example_UnionSize24Alignement8, 4u, 8u, 24u, "example/UnionSize24Alignement8", v1_example_UnionSize24Alignement8Table_alt));

static const ::fidl::FidlUnionField Fields23v1_example_UnionOfUnion[] = {
    ::fidl::FidlUnionField(nullptr, 7u, 1201318480u),
    ::fidl::FidlUnionField(&v1_example_UnionSize8Aligned4Table, 0u, 548068704u),
    ::fidl::FidlUnionField(&v1_example_UnionSize16Aligned4Table, 0u, 762734029u),
    ::fidl::FidlUnionField(&v1_example_UnionSize24Alignement8Table, 0u, 108145951u)
};
extern inline const ::fidl::FidlCodedUnion* const v1_example_UnionOfUnionTable_alt;
const fidl_type_t v1_example_UnionOfUnionTable = fidl_type_t(::fidl::FidlCodedUnion(Fields23v1_example_UnionOfUnion, 4u, 8u, 24u, "example/UnionOfUnion", v1_example_UnionOfUnionTable_alt));

extern inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich5_field1_alt_field;
static const ::fidl::FidlStructField Fields20v1_example_Sandwich5[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&v1_example_UnionOfUnionTable, 8u, 0u, Fields20v1_example_Sandwich5_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 36u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich5Table_alt;
const fidl_type_t v1_example_Sandwich5Table = fidl_type_t(::fidl::FidlCodedStruct(Fields20v1_example_Sandwich5, 3u, 40u, "example/Sandwich5", v1_example_Sandwich5Table_alt));

extern inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich3_field1_alt_field;
static const ::fidl::FidlStructField Fields20v1_example_Sandwich3[] = {
    ::fidl::FidlStructField(nullptr, 4u, 4u),
    ::fidl::FidlStructField(&v1_example_UnionSize24Alignement8Table, 8u, 0u, Fields20v1_example_Sandwich3_field1_alt_field),
    ::fidl::FidlStructField(nullptr, 36u, 4u)
};
extern inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich3Table_alt;
const fidl_type_t v1_example_Sandwich3Table = fidl_type_t(::fidl::FidlCodedStruct(Fields20v1_example_Sandwich3, 3u, 40u, "example/Sandwich3", v1_example_Sandwich3Table_alt));

// old <-> v1 mappings

inline const ::fidl::FidlCodedStruct* const example_Sandwich1Table_alt = &v1_example_Sandwich1Table.coded_struct;
inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich1Table_alt = &example_Sandwich1Table.coded_struct;

inline const ::fidl::FidlStructField* const Fields17example_Sandwich1_field1_alt_field = &Fields20v1_example_Sandwich1[1];
inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich1_field1_alt_field = &Fields17example_Sandwich1[0];

inline const ::fidl::FidlCodedStruct* const example_Sandwich2Table_alt = &v1_example_Sandwich2Table.coded_struct;
inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich2Table_alt = &example_Sandwich2Table.coded_struct;

inline const ::fidl::FidlStructField* const Fields17example_Sandwich2_field1_alt_field = &Fields20v1_example_Sandwich2[1];
inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich2_field1_alt_field = &Fields17example_Sandwich2[0];

inline const ::fidl::FidlCodedStruct* const example_Sandwich3Table_alt = &v1_example_Sandwich3Table.coded_struct;
inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich3Table_alt = &example_Sandwich3Table.coded_struct;

inline const ::fidl::FidlStructField* const Fields17example_Sandwich3_field1_alt_field = &Fields20v1_example_Sandwich3[1];
inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich3_field1_alt_field = &Fields17example_Sandwich3[1];

inline const ::fidl::FidlCodedStruct* const example_Sandwich4Table_alt = &v1_example_Sandwich4Table.coded_struct;
inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich4Table_alt = &example_Sandwich4Table.coded_struct;

inline const ::fidl::FidlStructField* const Fields17example_Sandwich4_field1_alt_field = &Fields20v1_example_Sandwich4[1];
inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich4_field1_alt_field = &Fields17example_Sandwich4[0];

inline const ::fidl::FidlCodedStruct* const example_Sandwich5Table_alt = &v1_example_Sandwich5Table.coded_struct;
inline const ::fidl::FidlCodedStruct* const v1_example_Sandwich5Table_alt = &example_Sandwich5Table.coded_struct;

inline const ::fidl::FidlStructField* const Fields17example_Sandwich5_field1_alt_field = &Fields20v1_example_Sandwich5[1];
inline const ::fidl::FidlStructField* const Fields20v1_example_Sandwich5_field1_alt_field = &Fields17example_Sandwich5[1];

inline const ::fidl::FidlCodedStruct* const example_StructSize16Alignement8Table_alt = &v1_example_StructSize16Alignement8Table.coded_struct;
inline const ::fidl::FidlCodedStruct* const v1_example_StructSize16Alignement8Table_alt = &example_StructSize16Alignement8Table.coded_struct;


inline const ::fidl::FidlCodedUnion* const example_UnionOfUnionTable_alt = &v1_example_UnionOfUnionTable.coded_union;
inline const ::fidl::FidlCodedUnion* const v1_example_UnionOfUnionTable_alt = &example_UnionOfUnionTable.coded_union;

inline const ::fidl::FidlCodedUnion* const example_UnionSize16Aligned4Table_alt = &v1_example_UnionSize16Aligned4Table.coded_union;
inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize16Aligned4Table_alt = &example_UnionSize16Aligned4Table.coded_union;

inline const ::fidl::FidlCodedUnion* const example_UnionSize24Alignement8Table_alt = &v1_example_UnionSize24Alignement8Table.coded_union;
inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize24Alignement8Table_alt = &example_UnionSize24Alignement8Table.coded_union;

inline const ::fidl::FidlCodedUnion* const example_UnionSize36Alignment4Table_alt = &v1_example_UnionSize36Alignment4Table.coded_union;
inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize36Alignment4Table_alt = &example_UnionSize36Alignment4Table.coded_union;

inline const ::fidl::FidlCodedUnion* const example_UnionSize8Aligned4Table_alt = &v1_example_UnionSize8Aligned4Table.coded_union;
inline const ::fidl::FidlCodedUnion* const v1_example_UnionSize8Aligned4Table_alt = &example_UnionSize8Aligned4Table.coded_union;

} // extern "C"
