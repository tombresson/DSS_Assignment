//////////////////////////////////////////////////////////////////////////////
//
//  json_serializer.h
//
//  JSON Serializer
//
//  Module that utilizes the JSON keylists and JWrite to serialize data into
//  a JSON document. Module is intended to provide basic functionality for
//  serializing data into JSON that is needed by the whole system. Modules
//  that have application specific serialization needs should build that
//  functionality out in their own module in an effort to keep this module
//  general case.
//
//  Copyright Fortune Brands Home & Security, 2019
//  All Rights Reserved.  This is a confidential intellectual property work
//  of Fortune Brands Home & Security and is not to be copied or transmitted to
//  other parties without the express written permission of Fortune Brands
//  Home & Security.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

/* ***************************    Includes     **************************** */

/* ***************************   Definitions   **************************** */

// Feedback datatype for JSON parser. Informs the caller the outcome of the
// requested serialization operation.
typedef enum
{
    E_JSON_SERIALIZE_SUCCESS = 0,
    E_JSON_SERIALIZE_BUFFER_FULL,
    E_JSON_SERIALIZE_JWRITE_ERROR
} jsonSerializeResult_t;

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

// This function will serialize an entire container and all of its nested children
// The parent type must be passed in so the correct syntax is used for containter that
// is requested to be written. If there is no parent container, then simply use `undefined`.
jsonSerializeResult_t jsonWriteContainer(const jsonContainer_t *const p_container,
                                         const jsonContainerType_t parent_container,
                                         struct jWriteControl *const p_jwc);

// Writes a single element to the JSON document
// p_data points to the data structure containing the data to be written. The data that will
// be serialized will be determined by the struct_memeber_offset value in the jsonKeyValue_t data
jsonSerializeResult_t jsonWriteElement(const jsonKeyValue_t *const p_key,
                                       const void *const p_data,
                                       struct jWriteControl *const p_jwc);

// Just simply writes an array and closes it
jsonSerializeResult_t jsonWriteArray(const jsonContainer_t *const p_array,
                                     const jsonContainerType_t parent_container,
                                     struct jWriteControl *const p_jwc);

// Not currently implemented, but could be if the need presented
#if 0
// Serializes an entire key-value list of data from the pointed to data (p_data)
jsonSerializeResult_t jsonWriteElements(const jsonKeyValueList_t *const p_key_list,
                                        const void *const p_data,
                                        struct jWriteControl *const p_jwc);
#endif


#endif /* JSON_SERIALIZER_H */
