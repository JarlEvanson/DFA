#ifndef COMPILER_STACK
#define COMPILER_STACK

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "vector.h"

#define DECLARE_STACK_PEEK(stackName, type) \
    type peek ## stackName(stackName* stack)

#define DEFINE_STACK_PEEK(stackName, type) \
    DECLARE_STACK_PEEK( stackName, type ) { \
        assert( stack->size > 0 ); \
        return stack->vec[stack->size - 1]; \
    }

#define DEFINE_STACK(typeName, type) \
    DEFINE_VECTOR_INIT( typeName ## Stack, type ) \
    DEFINE_FREE_VECTOR( typeName ## Stack, type ) \
    DEFINE_POP_VECTOR( typeName ## Stack, type ) \
    DEFINE_PUSH_VECTOR( typeName ## Stack, type ) \
    DEFINE_CLONE_VECTOR( typeName ## Stack, type ) \
    DEFINE_SHRINK_VECTOR( typeName ## Stack, type ) \
    DEFINE_REVERSE_VECTOR( typeName ## Stack, type )  \
    DEFINE_CAN_POP_VECTOR( typeName ## Stack, type ) \
    DEFINE_STACK_PEEK( typeName ## Stack, type ) \

#define DECLARE_STACK(typeName, type) \
    typedef struct typeName ## Stack { \
        type* vec; \
        uint32_t size; \
        uint32_t capacity; \
    } typeName ## Stack; \
    DECLARE_VECTOR_INIT( typeName ## Stack, type ); \
    DECLARE_FREE_VECTOR( typeName ## Stack, type ); \
    DECLARE_POP_VECTOR( typeName ## Stack, type ); \
    DECLARE_PUSH_VECTOR( typeName ## Stack, type ); \
    DECLARE_CLONE_VECTOR( typeName ## Stack, type ); \
    DECLARE_SHRINK_VECTOR( typeName ## Stack, type ); \
    DECLARE_REVERSE_VECTOR( typeName ## Stack, type ) ; \
    DECLARE_CAN_POP_VECTOR( typeName ## Stack, type ); \
    DECLARE_STACK_PEEK( typeName ## Stack, type ); \

#endif