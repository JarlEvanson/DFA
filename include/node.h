#ifndef COMPILER_NODE
#define COMPILER_NODE

#include "common.h"
#include "vector.h"
#include "stack.h"

typedef struct Node Node;
typedef struct Edge Edge;

DECLARE_VECTOR(NodePtr, Node*);
DECLARE_CONTAINS_VECTOR(NodePtrVector, Node*);
DECLARE_DEDUPLICATE_VECTOR(NodePtrVector, Node*);
DECLARE_EQUALS_VECTOR(NodePtrVector, Node*);

DECLARE_STACK(NodePtr, Node*);

DECLARE_VECTOR(EdgePtr, Edge*);


Node* createNode(uint32_t state, bool accepting);
void freeNode(Node* node);
void addConnection(Node* node, Node* connectee, uint8_t value);
void addConnectionEdge(Node* node, Edge* edge);
uint32_t getNodeState(Node* node);
Node* getNodeConnection(Node* node, uint8_t value);
Edge* getNthConnection(Node* node, uint32_t connectionIndex);
bool isNodeAccepting(Node* node);
void setNodeAccepting(Node* node, bool accepting);
void addNodeString(Node* node, const char* str);
const char** getNodeStrings(Node* node, uint32_t* numStrs);
uint32_t getNodeConnectionCount(Node* node);
NodePtrVector getDepthFirstList(bool newOp, Node* node);
EdgePtrVector getNodeConnections(Node* node);
NodePtrVector getAcceptingNodes(Node* node);
NodePtrVector getConnectionlessNodes(Node* node);
void freeNodeTree(Node* node);
void printNodeTree(bool newNode, Node* rootNode);
uint32_t renumberNodes(Node* node);

void sortNodeTreeConnections(Node* node);
uint32_t consecutiveSameNodeConnections(Node* node, uint32_t index);

Node* getEdgeConnectee(Edge* edge);
uint8_t getEdgeKey(Edge* edge);

#endif