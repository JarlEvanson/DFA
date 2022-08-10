#ifndef COMPILER_VECTOR
#define COMPILER_VECTOR

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#pragma region

#define DECLARE_VECTOR_INIT(vecName, type) \
    vecName init ## vecName(uint32_t capacity)

#define DEFINE_VECTOR_INIT(vecName, type) \
    DECLARE_VECTOR_INIT( vecName, type ) { \
        vecName vector; \
        vector.vec = (type *) calloc( capacity, sizeof(type) ); \
        vector.capacity = capacity; \
        vector.size = 0; \
        return vector; \
    }

#define DECLARE_FREE_VECTOR(vecName, type) \
    void free ## vecName(vecName* vector)

#define DEFINE_FREE_VECTOR(vecName, type) \
    DECLARE_FREE_VECTOR( vecName, type ) { \
        if( vector->vec != NULL ) { \
            free(vector->vec); \
        } \
        vector->vec = NULL; \
        vector->capacity = 0; \
        vector->size = 0; \
    }

#define DECLARE_POP_VECTOR(vecName, type) \
    type pop ## vecName(vecName* vector)

#define DEFINE_POP_VECTOR(vecName, type) \
    DECLARE_POP_VECTOR( vecName, type ) { \
        assert( vector->size > 0 ); \
        vector->size--; \
        return vector->vec[vector->size]; \
    }

#define DECLARE_PUSH_VECTOR(vecName, type) \
    void push ## vecName(vecName* vector, type value)

#define DEFINE_PUSH_VECTOR(vecName, type) \
    DECLARE_PUSH_VECTOR( vecName, type ) { \
        if( vector->size < vector->capacity ) { \
            vector->vec[vector->size] = value; \
            vector->size++; \
        } else { \
            vector->vec = (type*) realloc( vector->vec, 2 * vector->capacity * sizeof( type ) ); \
            vector->capacity *= 2; \
            vector->vec[vector->size] = value; \
            vector->size++; \
        } \
    }

#define DECLARE_GET_VECTOR(vecName, type) \
    type get ## vecName(vecName* vector, uint32_t index)

#define DEFINE_GET_VECTOR(vecName, type) \
    DECLARE_GET_VECTOR( vecName, type ) { \
        assert( index < vector->size ); \
        return vector->vec[index]; \
    }

#define DECLARE_CLONE_VECTOR(vecName, type) \
    vecName clone ## vecName(vecName* vector)

#define DEFINE_CLONE_VECTOR(vecName, type) \
    DECLARE_CLONE_VECTOR( vecName, type ) { \
        vecName cloned; \
        cloned = init ## vecName( vector->capacity ); \
        memcpy( cloned.vec, vector->vec, sizeof( type ) * vector->size ); \
        cloned.size = vector->size; \
        return cloned; \
    }

#define DECLARE_APPEND_VECTOR(vecName, type) \
    void append ## vecName( vecName* vector, vecName appendee )

#define DEFINE_APPEND_VECTOR(vecName, type) \
    DECLARE_APPEND_VECTOR( vecName, type ) { \
        vector->vec = realloc( vector->vec, sizeof( type ) * (vector->size + appendee.size) ); \
        memcpy( vector->vec + vector->size, appendee.vec, sizeof( type ) * appendee.size ); \
        vector->size += appendee.size; \
        vector->capacity = vector->size; \
    }

#define DECLARE_SWAP_REMOVE_VECTOR(vecName, type) \
    void swapRemove ## vecName( vecName* vector, uint32_t index )

#define DEFINE_SWAP_REMOVE_VECTOR(vecName, type) \
    DECLARE_SWAP_REMOVE_VECTOR( vecName, type ) { \
        if( index < vector->size ) { \
            vector->vec[index] = vector->vec[vector->size - 1]; \
            vector->size--; \
        } \
    }

#define DECLARE_SHRINK_VECTOR(vecName, type) \
    void shrink ## vecName( vecName* vector)

#define DEFINE_SHRINK_VECTOR(vecName, type) \
    DECLARE_SHRINK_VECTOR( vecName, type ) { \
        vector->vec = (type*) realloc( vector->vec, ( vector->size ) * sizeof( type ) ); \
        vector->capacity = vector->size; \
    }

#define DECLARE_CONTAINS_VECTOR(vecName, type) \
    bool contains ## vecName(vecName* vector, type value)

#define DEFINE_CONTAINS_VECTOR(vecName, type) \
    DECLARE_CONTAINS_VECTOR( vecName, type ) { \
        for( uint32_t index = 0; index < vector->size; index++ ) { \
            if( vector->vec[index] == value ) \
                return true; \
        } \
        return false; \
    }

#define DECLARE_REVERSE_VECTOR(vecName, type) \
    void reverse ## vecName(vecName* vector)

#define DEFINE_REVERSE_VECTOR(vecName, type) \
    DECLARE_REVERSE_VECTOR( vecName, type ) { \
        vecName temp = init ## vecName( vector->size ); \
        while( vector->size > 0 ) { \
            push ## vecName( &temp, pop ## vecName( vector ) ); \
        } \
        free ## vecName( vector ); \
        *vector = temp; \
    }

#define DECLARE_CAN_POP_VECTOR(vecName, type) \
    bool canPop ## vecName(vecName* vector)

#define DEFINE_CAN_POP_VECTOR(vecName, type) \
    DECLARE_CAN_POP_VECTOR( vecName, type ) { \
        return vector->size > 0 ? true : false; \
    }

#define DECLARE_DEDUPLICATE_VECTOR(vecName, type) \
    void deduplicate ## vecName(vecName* vector)

#define DEFINE_DEDUPLICATE_VECTOR(vecName, type) \
    DECLARE_DEDUPLICATE_VECTOR(vecName, type) { \
        vecName vec = init ## vecName( vector->size ); \
        for( uint32_t index = 0; index < vector->size; index++ ) { \
            if( !contains ## vecName( &vec, get ## vecName( vector, index ) ) ) \
                push ## vecName( &vec, get ## vecName( vector, index ) ); \
        } \
        free ## vecName( vector ); \
        *vector = vec; \
    }

#define DECLARE_EQUALS_VECTOR(vecName, type) \
    bool areEqual ## vecName(vecName* vec1, vecName* vec2)

#define DEFINE_EQUALS_VECTOR(vecName, type) \
    DECLARE_EQUALS_VECTOR(vecName, type) { \
        if( vec1->size != vec2->size ) \
            return false; \
        for( uint32_t index = 0; index < vec1->size; index++ ) { \
            if( vec1->vec[index] != vec2->vec[index] ) \
                return false; \
        } \
        return true; \
    }

#pragma endregion


/*
 *CONTAINS_VECTOR, DEDUPLICATE_VECTOR, and EQUALS_VECTOR require the equality operator, so they can only be used on primitive types
 *DEDUPLICATE_VECTOR requires CONTAINS_VECTOR
*/
#define DECLARE_VECTOR(typeName, type) \
    typedef struct typeName ## Vector { \
        type * vec; \
        uint32_t size; \
        uint32_t capacity; \
    } typeName ## Vector; \
    DECLARE_VECTOR_INIT( typeName ## Vector, type ); \
    DECLARE_FREE_VECTOR( typeName ## Vector, type ); \
    DECLARE_POP_VECTOR( typeName ## Vector, type ); \
    DECLARE_PUSH_VECTOR( typeName ## Vector, type ); \
    DECLARE_GET_VECTOR( typeName ## Vector, type ); \
    DECLARE_CLONE_VECTOR( typeName ## Vector, type ); \
    DECLARE_APPEND_VECTOR( typeName ## Vector, type ); \
    DECLARE_SWAP_REMOVE_VECTOR( typeName ## Vector, type ); \
    DECLARE_SHRINK_VECTOR( typeName ## Vector, type ); \
    DECLARE_REVERSE_VECTOR( typeName ## Vector, type ); \
    DECLARE_CAN_POP_VECTOR( typeName ## Vector, type );

#define DEFINE_VECTOR(typeName, type) \
    DEFINE_VECTOR_INIT( typeName ## Vector, type ) \
    DEFINE_FREE_VECTOR( typeName ## Vector, type ) \
    DEFINE_POP_VECTOR( typeName ## Vector, type ) \
    DEFINE_PUSH_VECTOR( typeName ## Vector, type ) \
    DEFINE_GET_VECTOR( typeName ## Vector, type ) \
    DEFINE_CLONE_VECTOR( typeName ## Vector, type ) \
    DEFINE_APPEND_VECTOR( typeName ## Vector, type ) \
    DEFINE_SWAP_REMOVE_VECTOR( typeName ## Vector, type ) \
    DEFINE_SHRINK_VECTOR( typeName ## Vector, type ) \
    DEFINE_REVERSE_VECTOR( typeName ## Vector, type ) \
    DEFINE_CAN_POP_VECTOR( typeName ## Vector, type )

#endif