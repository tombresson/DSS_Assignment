//////////////////////////////////////////////////////////////////////////////
//
//  json_serialize_deserialize_types.h
//
//  JSON Serialization and Deserialization Data Types
//
//  This module defines datatypes that are shared among both the serializer
//  and deserializer functionality.
//
//  Copyright Fortune Brands Home & Security, 2019
//  All Rights Reserved.  This is a confidential intellectual property work
//  of Fortune Brands Home & Security and is not to be copied or transmitted to
//  other parties without the express written permission of Fortune Brands
//  Home & Security.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef JSON_SERIALIZE_DESERIALIZE_TYPES_H
#define JSON_SERIALIZE_DESERIALIZE_TYPES_H

/* ***************************    Includes     **************************** */

// Include Jsmn header for `jsmntype_t` definition below
#include "jsmn.h"

// Included for the `enumLabel_t` definition used below
#include "enum_label.h"

/* ***************************   Definitions   **************************** */

// Macro to more easily define list of children inline of a jsonContainer_t
// Takes some of the pain out of making a array of pointers.
//
// i.e.:
// ```c
//  jsonContainer_t child_container_a;
//  jsonContainer_t child_container_b;
//
//  static jsonContainer_t parent_container =
//  {
//      .type = E_JSON_OBJECT,
//      .children = JSON_CHILD_LIST(&child_container_a, &child_container_b),
//      .num_children = 2,
//  };
// ```
//
// If the jsonContainer_t is a `const`, it will be required to cast them to
// in the macro to non-const pointers.
#define JSON_CHILD_LIST(...)       ((jsonContainer_t **)(&((jsonContainer_t *[]){__VA_ARGS__})))


// Defines the various primatives that are supported
// WARN: This type MUST match the type in the defined datastructure, otherwise adjacent
// WARN: elements are at risk of being read/written.
typedef enum
{
    E_JSON_C_NONE, // Value is not a primitive
    E_JSON_C_INT8,
    E_JSON_C_INT16,
    E_JSON_C_INT32, // NOTE: Preferred value for general use
    E_JSON_C_INT64,

    // NOTE: Caution should be taken when using deserializing into unsigned values
    // NOTE: because everything in JSON is treated as 64 bit double, negative
    // NOTE: values will ALWAYS be possible in received data.
    // NOTE: These are primarily supported for serialization convenience purposes
    E_JSON_C_UINT8,  // Byte values should use this type
    E_JSON_C_UINT16,
    E_JSON_C_UINT32,

    E_JSON_C_FLOAT,
    E_JSON_C_BOOL,

    // NOTE: When using type ENUM, the enum representing "0" should be an "unknown" value
    // NOTE: This is because if the value string does not match a known enum value, 0 will
    // NOTE: be written to the data field.
    E_JSON_C_ENUM,

    // NOTE: This will deserialize ONLY the pointer to the string into the buffer.
    // NOTE: keep this in  mind if the buffer is being thrown away after deserialization!
    // WARN: This must be used with the jsonStr_t defined below.
    E_JSON_C_STR_PTR
} jsonCPrimitiveType_t;

// Container type, if not specified, default is object
typedef enum
{
    E_JSON_OBJECT = 0,
    E_JSON_ARRAY,

    // Undefined should only be used for root objects/array's parents
    E_JSON_UNDEFINED
} jsonContainerType_t;

/* ****************************   Structures   **************************** */

// Datatype to be used in conjunction with `E_JSON_C_STR_PTR` type
// String pointer will be assigned to the `str` of the JSON Buffer that
// it is deserialized from. Keep this in mind when deciding to use this data type.
// The `len` value with be the exact length in bytes of the *non-NULL terminated* string
typedef struct
{
    const char *str;
    int len;
} jsonStr_t;

// Definition of a key string and expected value type.
// Used when searching the list of tokens for key-value pairs
// When populating elements to form a table, every element of the table must have a:
// * Key String
// * Value Struct Member offset
// * Value Struct Member size
// * Value Token Type
// Other fields are populated depending on the value token type
typedef struct
{
    const char *const key_str; // String that represents the key
    const int struct_member_offset; // Offset of the struct member to be deserialized to
    const int struct_member_size; // Size of the element of the struct (primarily used for string buffers)
    const jsmntype_t value_tok_type; // The JSMN token type that the value will be

    // Below is meta data that may be populated based on the type of information represented
    // The primitive data type that the value string will be converted to
    const jsonCPrimitiveType_t c_type;
    // Table to translate string to an enum
    const enumLabel_t *const enum_labels;
} jsonKeyValue_t;

// Struct that contains the definition data around a array of jsonKeyValue_t data elements
typedef struct
{
    const jsonKeyValue_t *const p_keys;
    const int size; // Number of keys in the list
} jsonKeyValueList_t;

// Type that defines JSON Objects
// These contain lists of key/value pairs and can reference child objects
typedef struct JsonContainer
{
    // Denotes what type of container this is
    const jsonContainerType_t type;

    // Child objects, can be NULL/0 if no children
    // NOTE: Use the `JSON_CHILD_LIST` macro to easily build the list of children
    struct JsonContainer **children;
    const int num_children;

    // Optional. If NULL or "", object will be assumed to have no name
    // This should ONLY be the case for the root object and arrays that contain objects
    const char *key_str;

    // Pointer to the data array/data structure
    // This should be set to the location of either the data structure (E_JSON_OBJECT type) that
    // is represented by the member sizes and offsets in the keylist
    // OR
    // to a data array (E_JSON_ARRAY type)
    // WARNING! FOR E_JSON_ARRAY, this MUST BE AT LEAST THE SIZE OF (element_size * array_size)
    void *p_data;

    union // Contains data based on the value of `type`
    {
        // Object Specific Data - (E_JSON_OBJECT)
        struct
        {
            // List of key/value data
            const jsonKeyValueList_t *p_key_list;

            /////////////////////////////////////////////////////////////////////////////////////
            // These two variables serve similar purposes, however, because their function is
            // different, depending on the intended direction (serialize or deserialize),
            // their names indicate the intended functionality.
            /////////////////////////////////////////////////////////////////////////////////////

            // Pointer to boolean array that will be written to when the keys are found in the JSON data.
            // NOTE: OPTIONAL - Can be set to NULL if the feedback is not needed.
            // WARNING! MUST BE AT LEAST AS LARGE AS THE NUMBER OF KEYS IN THE LIST. (p_key_list->size)
            bool *p_deserialized_elements;

            // Indicates to the serializer the specific data to serialize
            // Allows for a subset of the pointed-to key-value list to be serialized
            // NOTE: OPTIONAL - Can be set to NULL if the entire list is intended to be serialized.
            // WARNING! MUST BE AT LEAST AS LARGE AS THE NUMBER OF KEYS IN THE LIST. (p_key_list->size)
            bool *p_elements_to_serialize;
        };

        // Array specific data - (E_JSON_ARRAY)
        // Arrays must consist of HOMOGENEOUS data. No support for mixed data, currently.
        struct
        {
            // Type of tokens that will be in the JSON array
            const jsmntype_t value_tok_type;

            // Specified for when the value type is marked denoted as primitive
            // If the intention is to have the array contain objects or arrays, use the
            // `children` field in the root structure
            const jsonCPrimitiveType_t c_type;

            // Table to translate string to an enum
            const enumLabel_t *const enum_labels;

            // Element Size, in bytes. For most types this is defined by the type,
            // except for enums and strings.
            // For ENUMS: this should be set to the actual size of the enum type.
            // For STRINGS: This should be set to the buffer size.
            // Null termination (when serializing) will be observed as the end of the string.
            // String will be null terminated on deserialization. If it does not fit in
            // the specified size, it will be truncated.
            const int element_size;

            // Number of positions in the array
            int num_elements;
        };
    };
} jsonContainer_t;


/* ***********************   Function Prototypes   ************************ */

#endif /* JSON_SERIALIZE_DESERIALIZE_TYPES_H */
