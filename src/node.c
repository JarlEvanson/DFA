#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "node.h"
#include "vector.h"
#include "stack.h"

typedef struct Edge {
    Node* ptr;
    uint8_t value;
    //uint32_t value; //Unicode value
} Edge;

struct Node {
    Edge* connections;
    const char** str;
    uint32_t numStrs;
    uint32_t numConnections;
    uint32_t state;
    bool accepting;
};

DEFINE_VECTOR(NodePtr, Node*);
DEFINE_CONTAINS_VECTOR(NodePtrVector, Node*);
DEFINE_DEDUPLICATE_VECTOR(NodePtrVector, Node*);
DEFINE_EQUALS_VECTOR(NodePtrVector, Node*);

DEFINE_STACK(NodePtr, Node*);

DEFINE_VECTOR(EdgePtr, Edge*);

Node* createNode(uint32_t state, bool accepting) {
    Node* node = (Node*) malloc( sizeof( Node ) );
    node->connections = NULL;
    node->str = NULL;
    node->numStrs = 0;
    node->numConnections = 0;
    node->state = state;
    node->accepting = accepting;
    return node;
}

void freeNode(Node* node) {
    if( node->connections == NULL )
        free( node->connections );
}

void addConnection(Node* node, Node* connectee, uint8_t value) {
    node->connections = (Edge*) realloc( node->connections, (node->numConnections + 1) * sizeof( Edge ) );
    node->connections[node->numConnections].ptr = connectee;
    node->connections[node->numConnections].value = value;
    node->numConnections++;
}

void addConnectionEdge(Node* node, Edge* edge) {
    node->connections = (Edge*) realloc( node->connections, (node->numConnections + 1) * sizeof( Edge ) );
    node->connections[node->numConnections] = *edge;
    node->numConnections++;
}

uint32_t getNodeState(Node* node) {
    return node->state;
}

//Only returns one, so may not get all associated with NFA, but will get the all connections associated with the value when used on a DFA
Node* getNodeConnection(Node* node, uint8_t value) {
    for( uint32_t connIndex = 0; connIndex < node->numConnections; connIndex++ ) {
        if( node->connections[connIndex].value == value )
            return node->connections[connIndex].ptr;
    }
    return NULL;
}

bool isNodeAccepting(Node* node) {
    return node->accepting;
}

void setNodeAccepting(Node* node, bool accepting) {
    node->accepting = accepting;
}

uint32_t getNodeConnectionCount(Node* node) {
    return node->numConnections;
}

EdgePtrVector getNodeConnections(Node* node) {
    EdgePtrVector vec = initEdgePtrVector( node->numConnections );
    for( uint32_t index = 0; index < node->numConnections; index++ ) {
        pushEdgePtrVector( &vec, &node->connections[index] );
    }
    return vec;
}

Edge* getNthConnection(Node* node, uint32_t connectionIndex) {
    return (connectionIndex < node->numConnections) ? &node->connections[connectionIndex] : (Edge*) NULL;
}

NodePtrVector getAcceptingNodes(Node* node) {
    NodePtrVector vec = getDepthFirstList( true, node );
    for( uint32_t index = vec.size - 1; index != 0; index-- ) {
        if( !getNodePtrVector( &vec, index )->accepting )
            swapRemoveNodePtrVector( &vec, index );
    }
    if( !getNodePtrVector( &vec, 0 )->accepting )
            swapRemoveNodePtrVector( &vec, 0 );
    return vec;
}

NodePtrVector getDepthFirstList(bool newOp, Node* node) {
    static NodePtrVector vec;

    if( newOp ) {
        vec = initNodePtrVector( 100 );
    }

    pushNodePtrVector( &vec, node );

    for( uint32_t connIndex = 0; connIndex < node->numConnections; connIndex++ ) {
        bool visited = false;
        for( uint32_t searchIndex = 0; searchIndex < vec.size; searchIndex++ ) {
            if( getNodePtrVector( &vec, searchIndex ) == node->connections[connIndex].ptr ) {
                visited = true;
                break;
            }
        }
        if( !visited ) {
            getDepthFirstList( false, node->connections[connIndex].ptr );
        }
    }
    if( newOp )
        shrinkNodePtrVector( &vec );
    return vec;
}

void addNodeString(Node* node, const char* str) {
    node->str = realloc( node->str, (node->numStrs + 1) * sizeof( char* ) );
    node->str[node->numStrs] = str;
    node->numStrs++;
}

const char** getNodeStrings(Node* node, uint32_t* numStrs) {
    *numStrs = node->numStrs;
    return node->str;
}

//Not in depth first order
NodePtrVector getConnectionlessNodes(Node* node) {
    NodePtrVector vec = getDepthFirstList( true, node );
    for( uint32_t index = vec.size - 1; index != 0; index-- ) {
        if( getNodePtrVector( &vec, index )->numConnections != 0 )
            swapRemoveNodePtrVector( &vec, index );
    }
    if( getNodePtrVector( &vec, 0 )->numConnections != 0 )
            swapRemoveNodePtrVector( &vec, 0 );
    return vec;
}

void freeNodeTree(Node* node) {
    NodePtrVector vec = getDepthFirstList( true, node );
    for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
        free( getNodePtrVector( &vec, nodeIndex ) );
    }
    freeNodePtrVector( &vec );
}

static void printChar(uint8_t character ) {
    if( character >= 33 && character <= 126 && (character != '\\' && character != '\'') ) {
        printf("%c", character );
    } else {
        switch( character ) {
            case '\n':
                printf("\\n");
                break;
            case '\r':
                printf("\\r");
                break;
            case '\t':
                printf("\\t");
                break;
            case '\\':
                printf("\\\\");
                break;
            case '\'':
                printf("\\\'");
                break;
            default:
                printf("\\%o", character);
                break;
        }
    }
}

void printNewLine(uint32_t tabCount, const char* fmt, ...) {
    va_list ap;

    for( int index = 0; index < tabCount; index++ ) {
        printf("\t");
    }

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void printNodeTree(bool newNode, Node* rootNode) {
    
    static uint32_t tabCount;
    static NodePtrVector vec;
    if( newNode ) {
        freeNodePtrVector( &vec );
        vec = initNodePtrVector( 100 );
        tabCount = 0;
        sortNodeTreeConnections( rootNode );
    }
    
    if( rootNode->numConnections == 0 ) {
        printf("Node %d ", rootNode->state);
        if( rootNode->accepting ) {
            uint32_t numStrs = 0;
            const char** strList = getNodeStrings( rootNode, &numStrs );
            if( numStrs > 0 ) {
                printf("{ ");
                for( uint32_t index = 0; index - 1 < numStrs; index++ ) {
                    printf("\"%s\", ", strList[index]);
                }
                printf("\"%s\"", strList[numStrs - 1]);
                printf(" }");
            } else {
                printf("X");
            }
        }
        printf("\n");
        return;
    }

    printf("Node %d: {\n", rootNode->state);
    tabCount++;
    printNewLine( tabCount, "NumConnections: %u\n", rootNode->numConnections);
    printNewLine( tabCount, "Accepting: %s\n", rootNode->accepting ? "true" : "false");
    if( rootNode->accepting ) {
        uint32_t numStrs = 0;
        const char** strList = getNodeStrings( rootNode, &numStrs );
        if( numStrs > 0 ) {
            printNewLine( tabCount, "Strings: { ");
            for( uint32_t index = 0; index < numStrs - 1; index++ ) {
                printf("\"%s\", ", strList[index]);
            }
            printf("\"%s\"", strList[numStrs - 1]);
            printf(" }");
        }
        printf("\n");
    }
    printNewLine( tabCount, "Connections: {\n");
    tabCount++;
    
    pushNodePtrVector( &vec, rootNode );

    for( uint32_t index = 0; index < rootNode->numConnections; ) {
        bool visited = containsNodePtrVector( &vec, rootNode->connections[index].ptr );
        uint32_t sameNode = consecutiveSameNodeConnections( rootNode, index );
        if( sameNode == 1 ) {
            printNewLine( tabCount, "\'");
            printChar( rootNode->connections[index].value );
            printf("\': ");
        } else {
            printNewLine( tabCount, "\'");
            printChar( rootNode->connections[index].value );
            printf("-");
            printChar( rootNode->connections[index + sameNode - 1].value );
            printf("\': ");
        }
        
        if( !visited ) {
            pushNodePtrVector( &vec, rootNode->connections[index].ptr );
            printNodeTree( false, rootNode->connections[index].ptr );
        } else {
            printf("Node %d ", rootNode->connections[index].ptr->state);
            if( rootNode->connections[index].ptr->accepting  ) {
                uint32_t numStrs = 0;
                const char** strList = getNodeStrings( rootNode->connections[index].ptr, &numStrs );
                if( numStrs > 0 ) {
                    printf("{ ");
                    for( uint32_t index = 0; index < numStrs - 1; index++ ) {
                        printf("\"%s\", ", strList[index]);
                    }
                    printf("\"%s\" ) }", strList[numStrs - 1]);
                } else {
                    printf("X");
                }
            }
            printf("\n");
        }
        index += sameNode;
    }

    tabCount--;
    printNewLine( tabCount, "}\n");
    tabCount--;
    printNewLine( tabCount, "}\n");
}

uint32_t renumberNodes(Node* node) {
    NodePtrVector vec = getDepthFirstList( true, node );
    for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
        getNodePtrVector( &vec, nodeIndex )->state = nodeIndex;
    }
    uint32_t nodeCount = vec.size;
    freeNodePtrVector( &vec );
    return nodeCount;
}

int64_t partitionChar(Node* node, int64_t low, int64_t high) {
    char pivot = node->connections[high].value; 
    char pivit = pivot;

    int64_t i = low - 1;
    for( int64_t j = low; j <= high - 1; j++ ) {
        if( getNthConnection( node, j )->value < pivot ) {
            i++;
            char ip = node->connections[i].value;
            char jp = node->connections[j].value;
            Edge tmp = node->connections[i];
            node->connections[i] = node->connections[j];
            node->connections[j] = tmp;
        }
    }
    char ip = node->connections[i + 1].value;
    char jp = node->connections[high].value;
    Edge tmp = node->connections[i + 1];
    node->connections[i + 1] = node->connections[high];
    node->connections[high] = tmp;

    return i + 1;
}

void sortNodeConnectionsChar(Node* node, int64_t low, int64_t high) {
    if( low < high) {
        int64_t pi = partitionChar( node, low, high );
        sortNodeConnectionsChar( node, low, pi - 1 );
        sortNodeConnectionsChar( node, pi + 1, high );
    }
}

int64_t partitionPtr(Node* node, int64_t low, int64_t high) {
    uintptr_t pivot = (uintptr_t)node->connections[high].ptr; 

    int64_t i = low - 1;
    for( int64_t j = low; j <= high - 1; j++ ) {
        if( ((uintptr_t)getNthConnection( node, j )->ptr) < pivot ) {
            i++;
            Edge tmp = node->connections[i];
            node->connections[i] = node->connections[j];
            node->connections[j] = tmp;
        }
    }
    Edge tmp = node->connections[i + 1];
    node->connections[i + 1] = node->connections[high];
    node->connections[high] = tmp;

    return i + 1;
}

void sortNodeConnectionsPtr(Node* node, int64_t low, int64_t high) {
    if( low < high) {
        int64_t pi = partitionPtr( node, low, high );
        sortNodeConnectionsPtr( node, low, pi - 1 );
        sortNodeConnectionsPtr( node, pi + 1, high );
    }
}

void partitionThenSortNodeConnections(Node* node) {
    sortNodeConnectionsPtr( node, 0, ((int64_t)node->numConnections) - 1 );
    if( node->numConnections == 0 )
        return;
    uint32_t min = 0;
    uint32_t max = 0;
    uintptr_t currPtr = (uintptr_t)node->connections[max].ptr;
    while( max < node->numConnections - 1 && node->numConnections != 0 ) {
        while( max < node->numConnections - 1 && currPtr == ((uintptr_t)node->connections[max].ptr) ) {
            max++;
        }
        sortNodeConnectionsChar( node, min, max - 1 );
        currPtr = ((uintptr_t)node->connections[max].ptr);
        min = max;
    }
}

void sortNodeTreeConnections(Node* node) {
    NodePtrVector vec = getDepthFirstList( true, node );
    for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
        partitionThenSortNodeConnections( getNodePtrVector( &vec, nodeIndex ) );
    }
}

Node* getEdgeConnectee(Edge* edge) {
    return edge->ptr;
}

uint8_t getEdgeKey(Edge* edge) {
    return edge->value;
}

uint32_t consecutiveSameNodeConnections(Node* node, uint32_t index) {
    EdgePtrVector edges = getNodeConnections( node );
    Node* currConnection = getEdgeConnectee( getEdgePtrVector( &edges, index ) );

    uint32_t numConnections = 1;
    char currChar = getEdgeKey( getEdgePtrVector( &edges, index ) );
    while( 
        index + numConnections < edges.size && 
        getEdgeConnectee( getEdgePtrVector( &edges, index + numConnections ) ) == currConnection &&
        getEdgeKey( getEdgePtrVector( &edges, index + numConnections ) ) == currChar + 1
    ) {
        numConnections++;
        currChar++;
    }
    return numConnections;
}
