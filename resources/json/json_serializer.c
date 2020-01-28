//////////////////////////////////////////////////////////////////////////////
//
//  json_serializer.c
//
//  JSON Serializer
//
//  Module description in header.
//
//  Copyright Fortune Brands Home & Security, 2019
//  All Rights Reserved.  This is a confidential intellectual property work
//  of Fortune Brands Home & Security and is not to be copied or transmitted to
//  other parties without the express written permission of Fortune Brands
//  Home & Security.
//
//////////////////////////////////////////////////////////////////////////////


/* ***************************    Includes     **************************** */

// Standard Includes
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// FreeRTOS Includes

// Library Includes
#include "enum_label.h"
#include "jWrite.h"

// Module Includes
#include "json_serialize_deserialize_types.h"
#include "json_serializer.h"

/* ***************************   Definitions   **************************** */

#define JSON_SUPPORTED_NESTED_NODES             (10U)

/* ****************************   Structures   **************************** */

typedef struct
{
    int depth;
    jsonContainerType_t parent_type;
    const jsonContainer_t *p_node;
} jsonContainerNode_t;


// Defines a list of nodes
// Size is a little weird of a param, because the size is statically defined,
// however it keeps all the management of this object local to itself.
typedef struct
{
    jsonContainerNode_t nodes[JSON_SUPPORTED_NESTED_NODES];
    int count;
    int size;
} jsonContainerNodeList_t;


/* ***********************   Function Prototypes   ************************ */

static bool jsonWriteOpenArray(const jsonContainer_t *const p_array,
                               const jsonContainerType_t parent_container,
                               struct jWriteControl *const p_jwc);

static bool jsonWriteOpenObject(const jsonContainer_t *const p_object,
                                const jsonContainerType_t parent_container,
                                struct jWriteControl *const p_jwc);

static void jsonWriteArrayElement(const jsonContainer_t *const p_array, const int idx_to_write,
                                  struct jWriteControl *const p_jwc);

static jsonSerializeResult_t jsonGetResult(const int jwrite_result);


static void jsonNodePush(jsonContainerNodeList_t *const p_list, const jsonContainerNode_t *const p_node);
static jsonContainerNode_t *jsonNodePop(jsonContainerNodeList_t *const p_list);
static const jsonContainerNode_t *jsonNodePeek(const jsonContainerNodeList_t *const p_list);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

jsonSerializeResult_t jsonWriteContainer(const jsonContainer_t *const p_container,
                                         const jsonContainerType_t parent_container,
                                         struct jWriteControl *const p_jwc)
{
    assert(p_container != NULL);

    // Instantiate node list
    jsonContainerNodeList_t node_list = (jsonContainerNodeList_t)
    {
        .size = JSON_SUPPORTED_NESTED_NODES, .count = 0
    };

    // Depth must start at `1`, otherwise the root object of this container will not be ended
    int depth = 1;
    int jw_result = 0;

    // Push the first container into the stack
    jsonContainerNode_t node = (jsonContainerNode_t)
    {
        .p_node = p_container, .depth = depth, .parent_type = parent_container
    };
    jsonNodePush(&node_list, &node);

    // As long as there are more nodes in the list, keep iterating
    bool more_nodes_to_write = (node_list.count != 0);

    // Loop over the list, adding children as they are encountered, until the list is empty
    while (more_nodes_to_write)
    {
        // Pop the next node and process it
        node = *(jsonNodePop(&node_list));
        const jsonContainer_t *p_curr_container = node.p_node;
        const jsonContainerType_t curr_parent_type = node.parent_type;

        // Variable tracks the state of the current node. Need to know this in-order to call
        //`jwEnd` the right number of times
        bool node_written = false;

        // Traverse the tree of objects and write each as it is encountered, pushing children onto a stack
        // as traversal occurs.
        switch (p_curr_container->type)
        {
            case E_JSON_ARRAY:
                node_written = jsonWriteOpenArray(p_curr_container, curr_parent_type, p_jwc);
                break;

            case E_JSON_OBJECT:
                node_written = jsonWriteOpenObject(p_curr_container, curr_parent_type, p_jwc);
                break;

            default:
                // Unhandled type of container
                assert(false);
        }

        // Check if there are children, if so process them.
        if ((p_curr_container->num_children) > 0)
        {
            // Increase depth by one, as we're about to add the children  to the node list then
            // subsequently visit the last one pushed.
            depth++;

            // Push all the children, in reverse order, so that the processing order
            // matches the original indexing. Even though JSON spec doesn't care about ordering
            // this ensures that the order can be easily crafted in code.
            for (int idx = ((p_curr_container->num_children) - 1); idx >= 0; idx--)
            {
                node = (jsonContainerNode_t)
                {
                    .p_node = ((p_curr_container->children)[idx]),
                    .depth = depth,
                    .parent_type = (p_curr_container->type)
                };
                jsonNodePush(&node_list, &node);
            }
        }
        else
        {
            // Since there are no children, determine if there are more nodes to write to continue the loop
            more_nodes_to_write = (node_list.count != 0);

            // To properly form the JSON document, as it is structured in the containers, the function
            // needs to "end" as many nodes as it is different from the next node, (if there is a next node that is)
            if (more_nodes_to_write)
            {
                const jsonContainerNode_t *const p_next_node = jsonNodePeek(&node_list);
                // Next node should always have depth less than or equal to current, based on traversal
                assert(depth >= p_next_node->depth);

                // Because we're about to process the next node (which will either be a sibling
                // or a parent), need to end the current node. Node should only be ended, however
                // if it has actually been created.
                if (node_written)
                {
                    jw_result = jwEnd(p_jwc);
                    assert(jw_result == JWRITE_OK);
                }

                // Continue closing nodes until the next node's depth is reached.
                while ((depth) > (p_next_node->depth))
                {
                    jw_result = jwEnd(p_jwc);
                    assert(jw_result == JWRITE_OK);
                    depth--;
                }
            }
            // This is the last bit to run before exiting the loop,
            // as the condition to exit has been satisfied
            else
            {
                // If the current node was not written, the final depth is one less than the current
                if (!node_written)
                {
                    depth--;
                }

                // End as many objects as the depth that has been traveled
                while ((depth--) > 0)
                {
                    jw_result = jwEnd(p_jwc);
                    // A JWrite Error occurred
                    assert(jw_result == JWRITE_OK);
                }
            }

        }
    }

    return (jsonGetResult(jw_result));
}

jsonSerializeResult_t jsonWriteElement(const jsonKeyValue_t *const p_key,
                                       const void *const p_data,
                                       struct jWriteControl *const p_jwc)
{
    assert((p_key != - NULL) && (p_data != NULL) && (p_jwc != NULL));

    // Set p_elem to point to the element to be serialized
    const void *const p_elem = (void *)((unsigned int)p_data + p_key->struct_member_offset);
    const char *const p_key_str = p_key->key_str;
    int enum_value = -1;

    switch (p_key->value_tok_type)
    {
        case JSMN_STRING:
            if (p_key->c_type != E_JSON_C_ENUM)
            {
                // WARNING: String must be null terminated, otherwise jWrite will fail
                assert(((uint8_t *)p_elem)[(p_key->struct_member_size) - 1] == NULL);
                jwObj_string(p_jwc, p_key_str, p_elem);
            }
            else // Value is an enum, use the enum labels to find the string
            {
                // If asserting here, p_key to see if that key value is ACTUALLY
                // an enum type. If it is, ensure a array of enum labels is set in the struct.
                assert(p_key->enum_labels != NULL);

                // Get enum value based on element size (in bytes)
                switch (p_key->struct_member_size)
                {
                    case 1:
                        enum_value = *((uint8_t *)p_elem);
                        break;

                    case 2:
                        enum_value = *((uint16_t *)p_elem);
                        break;

                    case 4:
                        enum_value = *((uint32_t *)p_elem);
                        break;

                    default:
                        // Don't set a value. Evidently the enum is larger than 4 bytes...
                        // If this assert fails, check to ensure the enum has a
                        // size defined in the keyvalue list
                        assert(false);
                        break;
                }

                // Cast to meet function definition. JWrite won't attempt to change the value.
                jwObj_string(p_jwc, p_key_str, elGetLabel(p_key->enum_labels, enum_value));
            }

            break;

        case JSMN_PRIMITIVE:
            switch (p_key->c_type)
            {
                case (E_JSON_C_INT8):
                    jwObj_int(p_jwc, p_key_str, (int32_t)(*((int8_t *)p_elem)));
                    break;

                case (E_JSON_C_INT16):
                    jwObj_int(p_jwc, p_key_str, (int32_t)(*((int16_t *)p_elem)));
                    break;

                case (E_JSON_C_INT32):
                    jwObj_int(p_jwc, p_key_str, (*((int32_t *)p_elem)));
                    break;

                case (E_JSON_C_INT64):
                    jwObj_int64(p_jwc, p_key_str, (*((int64_t *)p_elem)));
                    break;

                case (E_JSON_C_UINT8):
                    jwObj_int(p_jwc, p_key_str, (int32_t)(*((uint8_t *)p_elem)));
                    break;

                case (E_JSON_C_UINT16):
                    jwObj_int(p_jwc, p_key_str, (int32_t)(*((uint16_t *)p_elem)));
                    break;

                case (E_JSON_C_UINT32):
                    // Casting to a 64 signed value, this ensures the whole 2^31-1 range can be used
                    jwObj_int64(p_jwc, p_key_str, (int64_t)(*((uint32_t *)p_elem)));
                    break;

                case (E_JSON_C_FLOAT):
                    jwObj_double(p_jwc, p_key_str, (double)(*((float *)p_elem)));
                    break;

                case (E_JSON_C_BOOL):
                    jwObj_bool(p_jwc, p_key_str, (*((bool *)p_elem)));
                    break;

                default:
                    assert(false);
                    break;
            }

            break;

        default:
            // Currently only supports STRINGS and PRIMATIVES as single elements
            assert(false);
            break;
    }

    // Return jWrite current status
    return (jsonGetResult(p_jwc->error));
}



jsonSerializeResult_t jsonWriteArray(const jsonContainer_t *const p_array,
                                     const jsonContainerType_t parent_container,
                                     struct jWriteControl *const p_jwc)
{
    jsonWriteOpenArray(p_array, parent_container, p_jwc);
    int jw_result = jwEnd(p_jwc);
    // A JWrite Error occurred
    assert(jw_result == JWRITE_OK);

    return (jsonGetResult(jw_result));
}



/* *************************   Private Functions   ************************ */

// Writes an array (does not call `jwEnd` at the conclusion) and iterates through
// indicies of the array writing each object.
// Return value indicates if array was written (true) or not (false)
static bool jsonWriteOpenArray(const jsonContainer_t *const p_array,
                               const jsonContainerType_t parent_container,
                               struct jWriteControl *const p_jwc)
{
    assert((p_array != NULL) && (p_jwc != NULL));

    // Currently if there is no items in the array,
    // and there are no specified children, don't write the data
    // This will avoid having empty objects in the document (saving bytes)
    // NOTE: If the need arises to "force empty nodes", then the easiest solution
    // NOTE: could be to just add a force_empty (or similar) flag to the data structure
    bool write_array = (p_array->num_elements > 0) || (p_array->children != NULL);

    if (write_array)
    {
        switch (parent_container)
        {
            case E_JSON_ARRAY:
                // Parent is an array, so name not necessary
                jwArr_array(p_jwc);
                break;

            case E_JSON_OBJECT:
            case E_JSON_UNDEFINED:
                // Array with object parent MUST have a name
                assert(p_array->key_str != NULL);
                jwObj_array(p_jwc, p_array->key_str);
                break;

            default:
                // Unhandled type of parent
                assert(false);
        }

        // Iterate through the HOMOGENEOUS data
        for (int idx  = 0; idx < p_array->num_elements; idx++)
        {
            // Write all of the array elements
            jsonWriteArrayElement(p_array, idx, p_jwc);
        }
    }

    // NOTE: Array MUST be ended in the calling function

    return write_array;
}

// Write an entire object
// This object does not HAVE to be the root object. If it is the root object,
// pass in UNDEFINED as the parent container.
// Return value indicates if object was written (true) or not (false)
static bool jsonWriteOpenObject(const jsonContainer_t *const p_object,
                                const jsonContainerType_t parent_container,
                                struct jWriteControl *const p_jwc)
{
    // Object must NOT be NULL
    assert((p_object != NULL) && (p_jwc != NULL));
    const char *p_key_str = p_object->key_str;

    switch (parent_container)
    {
        case E_JSON_ARRAY:
            // Objects in arrays are not allowed to have names
            assert(p_key_str == NULL);
            // Create an unnamed object in an array
            jwArr_object(p_jwc);
            break;

        case E_JSON_OBJECT:
        case E_JSON_UNDEFINED: // indicates root object
            // Create an object with a key name referenced in p_object
            jwObj_object(p_jwc, (char *)(p_key_str != NULL ? p_key_str : ""));
            break;

        default:
            // Unhandled type of parent
            assert(false);
    }

    // If there is key-value data to write, do so, otherwise there's nothing left to do
    if ((p_object->p_key_list != NULL) && (p_object->p_data != NULL))
    {
        // Iterate through the object's key list pulling data out of p_data struct
        const jsonKeyValueList_t *p_key_list = p_object->p_key_list;

        const bool *const p_elements_to_serialize = p_object->p_elements_to_serialize;

        for (int idx = 0; idx < p_key_list->size; idx++)
        {
            // Only serialize the data if either the data is intended to be serialized (check the array)
            // or the p_elements_to_serialize array is NULL (all should be serialized)
            // && operator guaranteed to evaluate left-to-right (N1570 - 6.5.13 Logical AND operator)
            if (((p_elements_to_serialize != NULL) && (p_elements_to_serialize[idx])) ||
                (p_elements_to_serialize == NULL))
            {
                const jsonKeyValue_t *const p_key = &(p_key_list->p_keys[idx]);
                jsonWriteElement(p_key, (p_object->p_data), p_jwc);
            }
        }
    }

    // NOTE: Object MUST be ended in the calling function

    // For the time being, object will always be written (empty or not)
    return true;
}

// Serializes an array element to the JSON Document, based on the type.
static void jsonWriteArrayElement(const jsonContainer_t *const p_array, const int idx_to_write,
                                  struct jWriteControl *const p_jwc)
{
    // Pointer to container and pointer data array should NOT be NULL
    assert((p_array != NULL) && (p_array->p_data != NULL));

    int enum_value = -1;
    assert(idx_to_write < p_array->num_elements);
    int data_offset_bytes;
    void *p_elem  = NULL;

    // Determine offset for index
    switch (p_array->value_tok_type)
    {
        case JSMN_STRING:
            // Use element size for string data, as this means that the values
            // are either strings or enumerations and the size MUST be specified
            assert(p_array->element_size != 0);
            data_offset_bytes = (p_array->element_size) * idx_to_write;
            break;

        case JSMN_PRIMITIVE:

            // Just calcuate the offset based on the data size
            switch (p_array->c_type)
            {
                case (E_JSON_C_INT8):
                case (E_JSON_C_UINT8):
                    data_offset_bytes = (1) * idx_to_write;
                    break;

                case (E_JSON_C_INT16):
                case (E_JSON_C_UINT16):
                    data_offset_bytes = (2) * idx_to_write;
                    break;

                case (E_JSON_C_INT32):
                case (E_JSON_C_UINT32):
                    data_offset_bytes = (4) * idx_to_write;
                    break;

                case (E_JSON_C_INT64):
                    data_offset_bytes = (sizeof(int64_t)) * idx_to_write;
                    break;

                case (E_JSON_C_FLOAT):
                    data_offset_bytes = (sizeof(float)) * idx_to_write;
                    break;

                case (E_JSON_C_BOOL):
                    data_offset_bytes = (sizeof(bool)) * idx_to_write;
                    break;

                default:
                    assert(false);
                    break;
            }

            break;

        default:
            // Only supports STRINGS and PRIMATIVES
            assert(false);
            break;
    }

    // Calculate data position
    p_elem = ((uint8_t *)p_array->p_data) + data_offset_bytes;

    // Serialize the data
    switch (p_array->value_tok_type)
    {
        case JSMN_STRING:
            if (p_array->c_type != E_JSON_C_ENUM)
            {
                // WARNING: String must be null terminated
                assert(((uint8_t *)p_elem)[(p_array->element_size) - 1] == NULL);
                jwArr_string(p_jwc, p_elem);
            }
            else // Value is an enum, use the enum labels to find the string
            {
                assert(p_array->enum_labels != NULL);

                // Get enum value based on element size
                switch (p_array->element_size)
                {
                    case 1:
                        enum_value = *((uint8_t *)p_elem);
                        break;

                    case 2:
                        enum_value = *((uint16_t *)p_elem);
                        break;

                    case 4:
                        enum_value = *((uint32_t *)p_elem);
                        break;

                    default:
                        // Don't set a value. Evidently the enum is larger than 4 bytes...
                        assert(false);
                        break;
                }

                // Cast to meet function definition. JWrite won't attempt to change the value.
                jwArr_string(p_jwc, elGetLabel(p_array->enum_labels, enum_value));
            }

            break;

        case JSMN_PRIMITIVE:
            switch (p_array->c_type)
            {
                case (E_JSON_C_INT8):
                    jwArr_int(p_jwc, (int32_t)(*((int8_t *)p_elem)));
                    break;

                case (E_JSON_C_INT16):
                    jwArr_int(p_jwc, (int32_t)(*((int16_t *)p_elem)));
                    break;

                case (E_JSON_C_INT32):
                    jwArr_int(p_jwc, (*((int32_t *)p_elem)));
                    break;

                case (E_JSON_C_INT64):
                    jwArr_int64(p_jwc, (*((int64_t *)p_elem)));
                    break;

                case (E_JSON_C_UINT8):
                    jwArr_int(p_jwc, (int32_t)(*((uint8_t *)p_elem)));
                    break;

                case (E_JSON_C_UINT16):
                    jwArr_int(p_jwc, (int32_t)(*((uint16_t *)p_elem)));
                    break;

                case (E_JSON_C_UINT32):
                    // Casting to a 64 signed value, this ensures the whole 2^31-1 range can be used
                    jwArr_int64(p_jwc, (int64_t)(*((uint32_t *)p_elem)));
                    break;

                case (E_JSON_C_FLOAT):
                    jwArr_double(p_jwc,  (double)(*((float *)p_elem)));
                    break;

                case (E_JSON_C_BOOL):
                    jwArr_bool(p_jwc,  (*((bool *)p_elem)));
                    break;

                default:
                    assert(false);
                    break;
            }

            break;

        default:
            // Only supports STRINGS and PRIMATIVES
            assert(false);
            break;
    }
}


// Converts a jWrite result into a more generic return value for use with this module
static jsonSerializeResult_t jsonGetResult(const int jwrite_result)
{
    jsonSerializeResult_t rslt = E_JSON_SERIALIZE_SUCCESS;

    if (jwrite_result != JWRITE_OK)
    {
        switch (jwrite_result)
        {
            case JWRITE_BUF_FULL:
                rslt = E_JSON_SERIALIZE_BUFFER_FULL;
                break;

            // Fallthrough intentional
            case JWRITE_NOT_ARRAY:
            case JWRITE_NOT_OBJECT:
            case JWRITE_STACK_FULL:
            case JWRITE_STACK_EMPTY:
            case JWRITE_NEST_ERROR:
                rslt = E_JSON_SERIALIZE_JWRITE_ERROR;
                break;

            default:
                // Unhandled status
                assert(false);
        }
    }

    return rslt;
}

///////////////////////////////////////////////////////////////////////////////////////
// Node list helper functions
///////////////////////////////////////////////////////////////////////////////////////

// Adds a node to the list
static void jsonNodePush(jsonContainerNodeList_t *const p_list, const jsonContainerNode_t *const p_node)
{
    assert((p_list != NULL) && (p_node != NULL) && (p_list->size != 0));

    // Check to see if there is at least 1 spot in the list
    if (p_list->count < (p_list->size))
    {
        p_list->nodes[(p_list->count)++] = *p_node;
    }
    else
    {
        // No space left in the list
        assert(false);
    }
}

// Returns a pointer to a node from the list and "removes" it from the list
// Returns null if there are no items in the list
static jsonContainerNode_t *jsonNodePop(jsonContainerNodeList_t *const p_list)
{
    assert((p_list != NULL) && (p_list->size != 0));

    // Check to see if there is at least 1 item in the list
    jsonContainerNode_t *p_node = NULL;

    if (p_list->count != 0)
    {
        p_node = &(p_list->nodes[--(p_list->count)]);
    }
    else
    {
        // No items in the list
        assert(false);
    }

    return p_node;
}

// Returns a pointer to a node from the top of the list
// Returns null if there are no items in the list
static const jsonContainerNode_t *jsonNodePeek(const jsonContainerNodeList_t *const p_list)
{
    assert((p_list != NULL) && (p_list->size != 0));

    // Check to see if there is at least 1 item in the list
    const jsonContainerNode_t *p_node = NULL;

    if (p_list->count != 0)
    {
        p_node = &(p_list->nodes[(p_list->count)]);
    }

    return p_node;
}
