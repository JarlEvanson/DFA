#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "common.h"
#include "node.h"
#include "vector.h"
#include "stack.h"
#include "regex.h"
#include "map.h"

//Misc
#pragma region

void printCharacter(char character) {
    if( character >= 33 && character <= 126 ) {
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
            default:
                printf("\\%o", character );
        }
    }
}

#pragma endregion

//Regex Tokens
#pragma region

typedef enum TokenTypeRegex {
    REGEX_LITERAL,
    REGEX_GROUPING,
    REGEX_OPERATOR,
    REGEX_SEPERATOR,
    REGEX_END_OF_STREAM,
    REGEX_INVALID,
} TokenTypeRegex;

typedef struct RegexToken {
    TokenTypeRegex type;
    char character;
} RegexToken;

DECLARE_VECTOR(RegexToken, RegexToken);
DEFINE_VECTOR(RegexToken, RegexToken);

DECLARE_STACK(RegexToken, RegexToken);
DEFINE_STACK(RegexToken, RegexToken);

DECLARE_VECTOR(Char, char );
DECLARE_CONTAINS_VECTOR( CharVector, char );
DECLARE_DEDUPLICATE_VECTOR( CharVector, char );

DEFINE_VECTOR(Char, char) ;
DEFINE_CONTAINS_VECTOR( CharVector, char );
DEFINE_DEDUPLICATE_VECTOR( CharVector, char );

//Pointer to RegexTokenStack or RegexTokenVec
void printRegexTokenStream(void* stack) {
    RegexTokenVector* vec = (RegexTokenVector*) stack;
    for( uint32_t index = 0; index < vec->size; index++ ) {
        if( getRegexTokenVector( vec, index ).type != REGEX_END_OF_STREAM ) {
            printCharacter( getRegexTokenVector( vec, index ).character );
            continue;
        }
        break;
    }
    printf("\n");
}

#pragma endregion

//Thompsons
#pragma region

Node* concatNodes( Node* root, Node* secondNode ) {
    NodePtrVector nodes = getConnectionlessNodes( root );

    EdgePtrVector secondNodeConnections = getNodeConnections( secondNode );

    for( uint32_t nodeIndex = 0; nodeIndex < nodes.size; nodeIndex++ ) {
        Node* currEnd = getNodePtrVector( &nodes, nodeIndex );
        setNodeAccepting( currEnd, false );
        for( uint32_t connIndex = 0; connIndex < secondNodeConnections.size; connIndex++ ) {
            addConnectionEdge( currEnd, getEdgePtrVector( &secondNodeConnections, connIndex) );
        }
    }

    freeNode( secondNode );

    freeEdgePtrVector( &secondNodeConnections );
    freeNodePtrVector( &nodes );

    return root;
}

void connectEndNodes(Node* node, uint32_t numConnectees, Node** connectees) {
    NodePtrVector endNodes = getConnectionlessNodes( node );
    for( uint32_t index = 0; index < endNodes.size; index++ ) {
        for( uint32_t connIndex = 0; connIndex < numConnectees; connIndex++ ) {
            addConnection( getNodePtrVector( &endNodes, index ), connectees[connIndex], '\0' );
        }
    }
    freeNodePtrVector( &endNodes );
}

Node* orNodes(uint32_t* currentState, Node* firstNode, Node* secondNode) {
    Node* newRoot = createNode( (*currentState)++, false ); //(*currentState)++ returns *currentState, then increments *currentState
    addConnection( newRoot, firstNode, '\0' );
    addConnection( newRoot, secondNode, '\0' );

    Node* endNode = createNode( (*currentState)++, true );

    connectEndNodes( newRoot, 1, &endNode );

    NodePtrVector accepting = getAcceptingNodes( newRoot );
    for( uint32_t index = 0; index < accepting.size; index++ ) {
        if( getNodeConnectionCount( getNodePtrVector( &accepting, index ) ) != 0 ) {
            setNodeAccepting( getNodePtrVector( &accepting, index ), false );
        }
    }

    return newRoot;
}

Node* kleeneStar(uint32_t* currentState, Node* node) {
    Node* newRoot = createNode( (*currentState)++, false ); //(*currentState)++ returns *currentState, then increments *currentState
    Node* endNode = createNode( (*currentState)++, true );

    addConnection( newRoot, node, '\0' );
    addConnection( newRoot, endNode, '\0' );

    Node* connectees[2] = { node, endNode };

    connectEndNodes( node, 2, connectees );

    NodePtrVector accepting = getAcceptingNodes( newRoot );
    for( uint32_t index = 0; index < accepting.size; index++ ) {
        if( getNodeConnectionCount( getNodePtrVector( &accepting, index ) ) != 0 ) {
            setNodeAccepting( getNodePtrVector( &accepting, index ), false );
        }
    }

    return newRoot;
}

char* getCharsBetween(char v1, char v2) {
    if( v1 > v2 ) {
        v1 = v1 ^ v2;
        v2 = v2 ^ v1;
        v1 = v1 ^ v2;
    }

    uint8_t difference = v2 - v1;
    char* str = (char*) calloc( difference + 2, sizeof(char) );
    for( uint32_t index = 0; index < difference + 1; index++ ) {
        str[index] = v1 + index;
    }
    str[difference + 1] = '\0';
    return str;
}

void parseString(RegexTokenStack* stack, const char* string) {
    uint32_t groupingLevel = 0;

    bool currentlyEscaped = false;
    for( int index = 0; index < strlen( string ); index++ ) {
        RegexToken token;
        if( !currentlyEscaped ) {
            switch( string[index] ) {
                case '\\':
                    currentlyEscaped = true;
                    break;
                case '?':
                    RegexTokenStack groupStack = initRegexTokenStack( stack->size );
                    token.type = REGEX_GROUPING;
                    token.character = ')';
                    pushRegexTokenStack( &groupStack, token );
                    uint32_t currGroupingLevel = groupingLevel;
                    for( int32_t currIndex = stack->size; currIndex > 0; currIndex-- ) {
                        pushRegexTokenStack( &groupStack, popRegexTokenStack( stack ) );
                        RegexToken tok = peekRegexTokenStack( &groupStack );
                        if( tok.type == REGEX_GROUPING ) {
                            if( tok.character == ')' ) {
                                currGroupingLevel++;
                            } else if( tok.character == '(' ) {
                                currGroupingLevel--;
                            }
                        }    
                        if( currGroupingLevel == groupingLevel )
                            break;
                    }
                    token.type = REGEX_OPERATOR;
                    token.character = '|';
                    pushRegexTokenStack( &groupStack, token );
                    token.type = REGEX_LITERAL;
                    token.character = '\0';
                    pushRegexTokenStack( &groupStack, token );
                    token.type = REGEX_GROUPING;
                    token.character = '(';
                    pushRegexTokenStack( &groupStack, token );
                    while( canPopRegexTokenStack( &groupStack ) ) {
                        pushRegexTokenStack( stack, popRegexTokenStack( &groupStack ) );
                    }
                    freeRegexTokenStack( &groupStack );
                    break;
                case '.':
                    groupingLevel++;
                    token.type = REGEX_GROUPING;
                    token.character = '(';
                    pushRegexTokenStack(stack, token);
                    for( uint32_t index = 1; index < 127; index++ ) {
                        if( index == '\n' )
                            continue;
                        token.type = REGEX_LITERAL;
                        token.character = index;
                        pushRegexTokenStack( stack, token );

                        token.type = REGEX_OPERATOR;
                        token.character = '|';
                        pushRegexTokenStack( stack, token );
                    }
                    token.type = REGEX_LITERAL;
                    token.character = 127;
                    pushRegexTokenStack( stack, token );
                    groupingLevel--;
                    token.type = REGEX_GROUPING;
                    token.character = ')';
                    pushRegexTokenStack(stack, token);
                    break;
                case '[':
                    groupingLevel++;
                    token.type = REGEX_GROUPING;
                    token.character = '(';
                    pushRegexTokenStack(stack, token);

                    bool flip = string[index + 1] == '^' ? true: false;
                    if( flip ) {
                        index += 2;
                    } else {
                        index++;
                    }

                    CharVector charVec = initCharVector( 10 );

                    while( string[index] != ']' ) {
                        if( string[index + 1] == '-' && string[index + 2] != ']' ) {
                            char* ch = getCharsBetween( string[index], string[index + 2] );
                            CharVector temp;
                            temp.capacity = strlen( ch );
                            temp.size = temp.capacity;
                            temp.vec = ch;
                            appendCharVector( &charVec, temp );
                            index += 3;
                            continue;
                        }
                        pushCharVector( &charVec, string[index] );
                        index++;
                    }

                    if( flip ) {
                        for( uint32_t charIndex = 0; charIndex < 128; charIndex++ ) {
                            
                            if( !containsCharVector( &charVec, charIndex ) ) {
                                if( peekRegexTokenStack( stack ).type == REGEX_LITERAL ) {
                                    token.type = REGEX_OPERATOR;
                                    token.character = '|';
                                    pushRegexTokenStack( stack, token );
                                }
                                token.type = REGEX_LITERAL;
                                token.character = charIndex;
                                pushRegexTokenStack( stack, token );
                            }
                        }
                    } else {
                        token.type = REGEX_LITERAL;
                        token.character = popCharVector( &charVec );
                        pushRegexTokenStack( stack, token );
                        while( canPopCharVector( &charVec ) ) {
                            token.type = REGEX_OPERATOR;
                            token.character = '|';
                            pushRegexTokenStack( stack, token );

                            token.type = REGEX_LITERAL;
                            token.character = popCharVector( &charVec );
                            pushRegexTokenStack( stack, token );
                        }
                    }

                    groupingLevel--;
                    token.type = REGEX_GROUPING;
                    token.character = ')';
                    pushRegexTokenStack(stack, token);
                    break;
                case ']':
                    if( groupingLevel == 0 ) {
                        printf("Invalid regex: Missing \'[\' before %u\n", index);
                        exit( 1 );
                    }
                    groupingLevel--;
                    token.type = REGEX_GROUPING;
                    token.character = ')';
                    pushRegexTokenStack(stack, token);
                    break;
                case '(':
                    groupingLevel++;
                    token.type = REGEX_GROUPING;
                    token.character = string[index];
                    pushRegexTokenStack(stack, token);
                    break;
                case ')':
                    if( groupingLevel == 0 ) {
                        printf("Invalid regex: Missing \'(\' before %u\n", index);
                        exit( 1 );
                    }
                    groupingLevel--;
                    token.type = REGEX_GROUPING;
                    token.character = string[index];
                    pushRegexTokenStack(stack, token);
                    break;
                case '|':
                    token.type = REGEX_OPERATOR;
                    token.character = string[index];
                    pushRegexTokenStack(stack, token);
                    break;
                case '+':
                    if( peekRegexTokenStack( stack ).type == REGEX_OPERATOR && peekRegexTokenStack( stack).character == '*' )
                        break;
                    if( peekRegexTokenStack( stack ).type == REGEX_GROUPING ) {
                        RegexTokenStack groupStack = initRegexTokenStack( stack->size );
                        uint32_t currGroupingLevel = groupingLevel;
                        RegexTokenVector* vec = (RegexTokenVector*) stack;
                        for( int32_t currIndex = stack->size; currIndex > 0; currIndex-- ) {
                            pushRegexTokenStack( &groupStack, getRegexTokenVector( vec, currIndex - 1 ) );
                            RegexToken tok = peekRegexTokenStack( &groupStack );
                            if( tok.type == REGEX_GROUPING ) {
                                if( tok.character == ')' ) {
                                    currGroupingLevel++;
                                } else if( tok.character == '(' ) {
                                    currGroupingLevel--;
                                }
                                if( currGroupingLevel == groupingLevel )
                                    break;
                            }
                        }
                        while( canPopRegexTokenStack( &groupStack ) ) {
                            pushRegexTokenStack( stack, popRegexTokenStack( &groupStack ) );
                        }
                        freeRegexTokenStack( &groupStack );
                    } else {
                        pushRegexTokenStack( stack, peekRegexTokenStack( stack ) );
                    }
                    token.type = REGEX_OPERATOR;
                    token.character = '*';
                    pushRegexTokenStack(stack, token);
                    break;
                case '*':
                    if( peekRegexTokenStack( stack ).type == REGEX_OPERATOR && peekRegexTokenStack( stack).character == '*' )
                        break;
                    token.type = REGEX_OPERATOR;
                    token.character = '*';
                    pushRegexTokenStack(stack, token);
                    break;
                default:
                    token.type = REGEX_LITERAL;
                    token.character = string[index];
                    pushRegexTokenStack(stack, token);
            }
        } else {
            currentlyEscaped = false;
            token.type = REGEX_LITERAL;
            const char* chars;
            switch( string[index] ) {
                case 'n':
                    token.character = '\n';
                    break;
                case 't':
                    token.character = '\t';
                    break;
                case 'r':
                    token.character = '\r';
                    break;
                case 'w':
                    token.type = REGEX_GROUPING;
                    token.character = '(';
                    pushRegexTokenStack( stack, token );
                    chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
                    for( uint32_t index = 0; index < strlen( chars ) - 1; index++ ) {
                        token.type = REGEX_LITERAL;
                        token.character = chars[index];
                        pushRegexTokenStack( stack, token );

                        token.type = REGEX_OPERATOR;
                        token.character = '|';
                        pushRegexTokenStack( stack, token );
                    } 
                    token.type = REGEX_LITERAL;
                    token.character = chars[strlen(chars) - 1];
                    pushRegexTokenStack( stack, token );
                    token.type = REGEX_GROUPING;
                    token.character = ')';
                    break;
                case 'x':
                    token.type = REGEX_GROUPING;
                    token.character = '(';
                    pushRegexTokenStack( stack, token );
                    chars = "0123456789abcdefABCDEF";
                    for( uint32_t index = 0; index < strlen( chars ) - 1; index++ ) {
                        token.type = REGEX_LITERAL;
                        token.character = chars[index];
                        pushRegexTokenStack( stack, token );

                        token.type = REGEX_OPERATOR;
                        token.character = '|';
                        pushRegexTokenStack( stack, token );
                    } 
                    token.type = REGEX_LITERAL;
                    token.character = chars[strlen(chars) - 1];
                    pushRegexTokenStack( stack, token );
                    token.type = REGEX_GROUPING;
                    token.character = ')';
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    char* end = NULL;
                    token.character = strtol( &string[index], &end, 8 );
                    index = end - &string[0] - 1;
                    break;
                default:
                    token.character = string[index];
            }
            pushRegexTokenStack(stack, token);
        }
    }
    if( groupingLevel > 0 ) {
        printf("Invalid regex: Missing \')\'parenthesis\n");
        exit( 1 );
    }

    RegexToken tok; 
    tok.type = REGEX_END_OF_STREAM; 
    tok.character = 0; 
    pushRegexTokenStack( stack, tok );

    RegexTokenVector* vec = (RegexTokenVector*) stack;

    RegexTokenStack concatStack = initRegexTokenStack( vec->size + vec->size / 2 );

    for( uint32_t index = 0; index + 1 < vec->size; index++ ) {
        RegexToken tok = getRegexTokenVector( vec, index );
        RegexToken lookahead = getRegexTokenVector( vec, index + 1 );
        pushRegexTokenStack( &concatStack, tok );
        if( 
            (tok.type == REGEX_LITERAL || tok.type == REGEX_GROUPING || (tok.type == REGEX_OPERATOR && tok.character == '*')) && 
            (lookahead.type == REGEX_LITERAL || lookahead.type == REGEX_GROUPING)
        ) {
            if( (tok.type == REGEX_GROUPING && tok.character == '(') || (lookahead.type == REGEX_GROUPING && lookahead.character ==')') ) {
                continue;
            }
            RegexToken concat;
            concat.type = REGEX_OPERATOR;
            concat.character = '+';
            pushRegexTokenStack( &concatStack, concat );
        }
    }

    pushRegexTokenStack( &concatStack, getRegexTokenVector( vec, vec->size - 1 ) );

    freeRegexTokenStack( stack );

    *stack = concatStack;
}

RegexTokenStack toPostFix(RegexTokenStack* stack) {
    RegexTokenStack postfixStack = initRegexTokenStack( stack->size );

    reverseRegexTokenStack( stack );
    
    RegexTokenStack opStack = initRegexTokenStack( stack->size );

    while( stack->size > 0 ) {
        RegexToken token = popRegexTokenStack( stack );
        switch( token.type ) {
            case REGEX_LITERAL:
                pushRegexTokenStack( &postfixStack, token );
                break;
            case REGEX_OPERATOR:
                RegexToken peeked; 
                while( opStack.size > 0 && ( peeked = peekRegexTokenStack( &opStack ) ).character != '(' ) {
                    if( peeked.character == '|' && token.character == '|' ) {
                        pushRegexTokenStack( &postfixStack, popRegexTokenStack( &opStack ) );
                    } else if( peeked.character == '+' && ( token.character == '|' || token.character == '+' ) ) {
                        pushRegexTokenStack( &postfixStack, popRegexTokenStack( &opStack ) );
                    } else if( peeked.character == '*' ) {
                        pushRegexTokenStack( &postfixStack, popRegexTokenStack( &opStack ) );
                    } else {
                        break;
                    }
                }
                pushRegexTokenStack( &opStack, token );
                break;
            case REGEX_GROUPING:
                if( token.character == '(' ) {
                    pushRegexTokenStack( &opStack, token );
                } else if( token.character == ')' ) {
                    while( peekRegexTokenStack( &opStack ).character != '(' ) {
                        pushRegexTokenStack( &postfixStack, popRegexTokenStack( &opStack ) );
                    }
                    popRegexTokenStack( &opStack );
                }
                break;
        }
    }
    while( opStack.size > 0 ) {
        pushRegexTokenStack( &postfixStack, popRegexTokenStack( &opStack ) );
    }

    freeRegexTokenStack( &opStack );

    return postfixStack;
}

Node* thompsonsConstruction(const char* regex) {
    #ifdef DEBUG
    const char* j = regex;
    printf("Regex: ");
    for(; *j != '\0'; j++ ) {
        printCharacter( *j );
    }
    printf("\n");
    #endif

    RegexTokenStack stack = initRegexTokenStack( 100 );
    parseString( &stack, regex );

    printf("Regex Parsed: ");
    printRegexTokenStream( &stack );

    RegexTokenStack postfixStack = toPostFix( &stack );

    #ifdef DEBUG
    printf("Token Stream Parsed: ");
    printRegexTokenStream( &postfixStack );
    #endif

    freeRegexTokenStack( &stack );

    reverseRegexTokenStack( &postfixStack );

    NodePtrStack rpnStack = initNodePtrStack( postfixStack.size );

    uint32_t currentState = 0;

    while( canPopRegexTokenStack( &postfixStack )  ) {
        #ifdef DEBUG
        printf("Token Stack: ");
        printRegexTokenStream( &postfixStack );
        printf("RPN Stack Size: %u\n", rpnStack.size);
        #endif
        RegexToken token = popRegexTokenStack( &postfixStack );
        if( token.type == REGEX_LITERAL ) {
            Node* node = createNode( currentState, false );
            addConnection( node, createNode( currentState + 1, true ), token.character );
            pushNodePtrStack( &rpnStack, node );
            #ifdef DEBUG
            printNodeTree( true, peekNodePtrStack( &rpnStack ) );
            #endif
            currentState += 2;
        } else if( token.type == REGEX_OPERATOR ) {
            Node* operand2;
            Node* operand1;
            switch( token.character ) {
                case '+':
                    operand2 = popNodePtrStack( &rpnStack );
                    operand1 = popNodePtrStack( &rpnStack );
                    pushNodePtrStack( &rpnStack, concatNodes( operand1, operand2 ) );
                    #ifdef DEBUG
                    printNodeTree( true, peekNodePtrStack( &rpnStack ) );
                    #endif
                    break;
                case '|':
                    operand2 = popNodePtrStack( &rpnStack );
                    operand1 = popNodePtrStack( &rpnStack );
                    pushNodePtrStack( &rpnStack, orNodes( &currentState, operand1, operand2 ) );
                    #ifdef DEBUG
                    printNodeTree( true, peekNodePtrStack( &rpnStack ) );
                    #endif
                    break;
                case '*':
                    operand1 = popNodePtrStack( &rpnStack );
                    pushNodePtrStack( &rpnStack, kleeneStar( &currentState, operand1 ) );
                    #ifdef DEBUG
                    printNodeTree( true, peekNodePtrStack( &rpnStack ) );
                    #endif
                    break;
            }
        }
    }

    Node* root = popNodePtrStack( &rpnStack );

    freeNodePtrStack( &rpnStack );
    freeRegexTokenStack( &postfixStack );

    renumberNodes( root );

    return root;
}

#pragma endregion

//Subset Construction
#pragma region

#pragma region

DECLARE_VECTOR(CharPtr, char* );
DECLARE_CONTAINS_VECTOR( CharPtrVector, char* );
DECLARE_DEDUPLICATE_VECTOR( CharPtrVector, char* );

DEFINE_VECTOR(CharPtr, char* );
DEFINE_CONTAINS_VECTOR( CharPtrVector, char* );
DEFINE_DEDUPLICATE_VECTOR( CharPtrVector, char* );

DECLARE_VECTOR(Uint32_t, uint32_t);
DECLARE_CONTAINS_VECTOR( Uint32_tVector, uint32_t );
DECLARE_DEDUPLICATE_VECTOR( Uint32_tVector, uint32_t );

DEFINE_VECTOR(Uint32_t, uint32_t);
DEFINE_CONTAINS_VECTOR( Uint32_tVector, uint32_t );
DEFINE_DEDUPLICATE_VECTOR( Uint32_tVector, uint32_t );

DECLARE_VECTOR(NodePtrVector, NodePtrVector);
DEFINE_VECTOR(NodePtrVector, NodePtrVector);

typedef struct TableEntry {
    NodePtrVector data;
} TableEntry;

typedef struct Table {
    TableEntry* entries;
    uint32_t capacity;
    uint32_t highestUsed;
} Table;

void initTable( Table* table, uint32_t estimatedStates) {
    table->entries = (TableEntry*) calloc( 128 * estimatedStates, sizeof(TableEntry) );
    memset( table->entries, 0, 128 * estimatedStates * sizeof(TableEntry) );
    table->capacity = estimatedStates;
    table->highestUsed = 0;
}

void freeTable( Table* table ) {
    if( table->entries != NULL ) {
        free( table->entries );
    }
    table->capacity = 0;
    table->highestUsed = 0;
}

void insertTable( Table* table, uint32_t setNum, char transition, NodePtrVector data ) {
    if( setNum >= table->capacity ) {
        table->entries = (TableEntry*) realloc( table->entries, 128 * (setNum + 1) * sizeof(TableEntry) );
        memset( &table->entries[128 * table->capacity], 0, 128 * sizeof(TableEntry) * ( setNum - table->capacity + 1 ) );
        table->capacity = setNum + 1;
    }
    table->entries[setNum * 128 + ((uint8_t) transition)].data = data;
    table->highestUsed = (setNum > table->highestUsed) ? setNum : table->highestUsed; 
} 

NodePtrVector getTable( Table* table, uint32_t setName, char transition) {
    assert( setName < table->capacity );
    return table->entries[setName * 128 + ((uint8_t) transition)].data;
}

void shrinkToHighest( Table* table ) {
    table->entries = (TableEntry*) realloc( table->entries, 128 * (table->highestUsed + 1) * sizeof(TableEntry) );
    table->capacity = table->highestUsed + 1;
}

#pragma endregion

NodePtrVector emptyClosure(Node* root) {
    static uint32_t used = 0;
    used++;
    NodePtrVector eReachable = initNodePtrVector( 100 );
    pushNodePtrVector( &eReachable, root );

    NodePtrVector unvisited = initNodePtrVector( 100 );
    pushNodePtrVector( &unvisited, root );

    do {
        Node* currentNode = popNodePtrVector( &unvisited );

        EdgePtrVector edges =  getNodeConnections( currentNode );
        for( uint32_t edgeIndex = 0; edgeIndex < edges.size; edgeIndex++ ) {
            if( getEdgeKey( getEdgePtrVector( &edges, edgeIndex ) ) == '\0' ) {
                if( !containsNodePtrVector( &eReachable, getEdgeConnectee( getEdgePtrVector( &edges, edgeIndex ) ) ) ) {
                    pushNodePtrVector( &unvisited, getEdgeConnectee( getEdgePtrVector( &edges, edgeIndex ) ) );
                    pushNodePtrVector( &eReachable, getEdgeConnectee( getEdgePtrVector( &edges, edgeIndex ) ) );
                }
            }
        }
        freeEdgePtrVector( &edges );
    } while( canPopNodePtrVector( &unvisited ) );

    freeNodePtrVector( &unvisited );

    shrinkNodePtrVector( &eReachable );
    return eReachable;
}

CharVector findNonEmptyTransitions(Node* root) {
    CharVector chars = initCharVector( 100 );

    NodePtrVector nodes = getDepthFirstList( true, root );
    for( uint32_t nodeIndex = 0; nodeIndex < nodes.size; nodeIndex++ ) {
        EdgePtrVector edges = getNodeConnections( getNodePtrVector( &nodes, nodeIndex ) );

        for( uint32_t edgeIndex = 0; edgeIndex < edges.size; edgeIndex++ ) {
            if( getEdgeKey( getEdgePtrVector( &edges, edgeIndex ) ) != '\0' )
                pushCharVector( &chars, getEdgeKey( getEdgePtrVector( &edges, edgeIndex ) ) );
        }

        freeEdgePtrVector( &edges );
    }

    deduplicateCharVector( &chars );
    shrinkCharVector( &chars );

    return chars;
}

NodePtrVector delta(NodePtrVector q, char character) {
    NodePtrVector validTransitions;
    validTransitions = initNodePtrVector( 100 );
    for( uint32_t qIndex = 0; qIndex < q.size; qIndex++ ) {
        Node* node = getNodeConnection( getNodePtrVector( &q, qIndex), character );
        if ( node != NULL )
            pushNodePtrVector( &validTransitions, node );
        
    }
    shrinkNodePtrVector( &validTransitions );
    return validTransitions;
}

Node* subsetConstruction(Node* root) {
    renumberNodes( root );

    CharVector nonEmptyTransitions = findNonEmptyTransitions( root );
    #ifdef DEBUG
    printf("Transitions: ");
    for( uint32_t index = 0; index < nonEmptyTransitions.size; index++ ) {
        printCharacter( getCharVector( &nonEmptyTransitions, index) );
        printf(" ");
    }
    printf("\n");
    #endif

    NodePtrVector q0 = emptyClosure( root );

    #ifdef DEBUG
        #define PRINT_ROW(obj)  \
            { \
                for( uint32_t lds3sd = 0; lds3sd < (obj).size; lds3sd++ ) { \
                    printf("%u ", getNodeState( getNodePtrVector( &obj, lds3sd ) ) );  \
                } \
                printf("\n"); \
            }

        #define PRINT_STATES(name, obj) \
            { \
                printf( #name ": "); \
                PRINT_ROW(obj); \
            }

        #define PRINT_Q() \
            { \
                printf("\nQ\n"); \
                for( uint32_t lds1s = 0; lds1s < Q.size; lds1s++ ) { \
                    printf("%u: ", lds1s); \
                    NodePtrVector temp = getNodePtrVectorVector( &Q, lds1s ); \
                    PRINT_ROW( temp ); \
                } \
            }

        #define PRINT_TABLE() \
            { \
                printf("\nTable:\n"); \
                for( uint32_t setNum = 0; setNum < T.capacity; setNum++ ) { \
                    for( uint16_t ch = 0; ch < 128; ch++ ) { \
                        NodePtrVector tableVec = getTable( &T, setNum, ch ); \
                        if( tableVec.size == 0 ) \
                            continue; \
                        printf("[%u][", setNum); \
                        printCharacter( ch ); \
                        printf("]: "); \
                        PRINT_ROW( tableVec ); \
                    } \
                } \
                printf("\n"); \
            }
    #endif
    
    NodePtrVectorVector Q = initNodePtrVectorVector( 10 );
    pushNodePtrVectorVector( &Q, q0 );

    NodePtrVectorVector workList = cloneNodePtrVectorVector( &Q );

    Table T;
    initTable( &T, 10 );
    #ifdef DEBUG
    PRINT_TABLE();
    #endif

    uint32_t currentSetName = 0;

    while( currentSetName < workList.size ) {
        NodePtrVector q = getNodePtrVectorVector( &workList, currentSetName );
        #ifdef DEBUG
        PRINT_Q();
        printf("\n");
        PRINT_TABLE();
        printf("\n");
        PRINT_STATES(q, q);
        #endif
        for( uint32_t transIndex = 0; transIndex < nonEmptyTransitions.size; transIndex++ ) {
            char ch = getCharVector( &nonEmptyTransitions, transIndex );
            NodePtrVector reachableNodes = delta( q, getCharVector( &nonEmptyTransitions, transIndex ) );
            #ifdef DEBUG
            printCharacter( getCharVector( &nonEmptyTransitions, transIndex ) );
            printf(": ");
            PRINT_STATES(reachableNodes, reachableNodes);
            #endif
            NodePtrVector temp;
            temp = initNodePtrVector( 100 );
            for( uint32_t nodeIndex = 0; nodeIndex < reachableNodes.size; nodeIndex++ ) {
                appendNodePtrVector( &temp, emptyClosure( getNodePtrVector( &reachableNodes, nodeIndex ) ) );
            }
            #ifdef DEBUG
            PRINT_STATES(t, temp);
            #endif
            if( temp.size == 0 ) {
                freeNodePtrVector( &temp );
                freeNodePtrVector( &reachableNodes );
                continue;
            }
            insertTable( &T, currentSetName, getCharVector( &nonEmptyTransitions, transIndex ), temp );
            bool found = false;
            for( uint32_t searchSets = 0; searchSets < Q.size; searchSets++ ) {
                NodePtrVector vec = getNodePtrVectorVector( &Q, searchSets );
                if( vec.size != temp.size )
                    continue;
                bool equal = true;
                for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
                    if( getNodePtrVector( &vec, nodeIndex) != getNodePtrVector( &temp, nodeIndex) ) {
                        equal = false;
                        break;
                    }
                }
                if( equal )
                    found = true;
            }
            if( !found ) {
                pushNodePtrVectorVector( &Q, temp );
                pushNodePtrVectorVector( &workList, temp );
            } 
            freeNodePtrVector( &reachableNodes );
        }
        currentSetName++;
    }

    shrinkToHighest( &T );

    #ifdef DEBUG
    PRINT_Q();
    PRINT_TABLE();
    printf("QSize: %u, TCap: %u\n", Q.size, T.capacity);
    #endif

    //In an NFA generated by thompson's algorithm, only the end node is connectionless and only the end node is accepting
            
    NodePtrVector acceptingNodes = getAcceptingNodes( root );

    Node** nodes = (Node**) calloc( Q.size, sizeof(Node*) );

    for( uint32_t nodeIndex = 0; nodeIndex < Q.size; nodeIndex++ ) {
        NodePtrVector vec = getNodePtrVectorVector( &Q, nodeIndex );

        CharPtrVector strs = initCharPtrVector( 10 );
        for( uint32_t index = 0; index < vec.size; index++ ) {
            uint32_t strCount = 0;
            const char** ch = getNodeStrings( getNodePtrVector( &vec, index ), &strCount );
            for( uint32_t strIndex = 0; strIndex < strCount; strIndex++ ) {
                if( !containsCharPtrVector( &strs, (char*) ch[strIndex] ) ) {
                    pushCharPtrVector( &strs, (char*)ch[strIndex] );
                }
            }
        }

        //If a group contains an accepting node, then the node representing the group is accepting
        bool accepting = false;
        for( uint32_t acceptIndex = 0; acceptIndex < acceptingNodes.size; acceptIndex++ ) {
            if( containsNodePtrVector( &vec, getNodePtrVector( &acceptingNodes, acceptIndex ) ) ) {
                accepting = true;
                break;
            }
        }
        nodes[nodeIndex] = createNode( nodeIndex, accepting );
        for( uint32_t strIndex = 0; strIndex < strs.size; strIndex++ ) {
            addNodeString( nodes[nodeIndex], getCharPtrVector( &strs, strIndex ) );
        }
    }

    freeNodePtrVector( &acceptingNodes );

    for( uint8_t ch = 0; ch < 128; ch++ ) {
        for( uint32_t setNum = 0; setNum < T.capacity; setNum++ ) {
            NodePtrVector tableVec = getTable( &T, setNum, ch );
            if( tableVec.size == 0 )
                continue;

            for( uint32_t nodeIndex = 0; nodeIndex < Q.size; nodeIndex++ ) {
                NodePtrVector nodeVec = getNodePtrVectorVector( &Q, nodeIndex );
                if( areEqualNodePtrVector( &tableVec, &nodeVec ) ) {
                    addConnection( nodes[setNum], nodes[nodeIndex], ch );
                    break;
                }
            }
        }
    }

    freeNodePtrVector( &q0 );
    freeNodePtrVectorVector( &Q );
    freeCharVector( &nonEmptyTransitions );

    for( uint32_t index = 0; index < T.capacity; index++ ) {
        freeNodePtrVector( &T.entries[index].data );
    }
    freeTable( &T );

    return nodes[0];
}

#pragma endregion

//Hopcroft's
#pragma region

NodePtrVectorVector initialPartition(Node* node) {
    NodePtrVector accepting = initNodePtrVector( 6 );
    NodePtrVector unaccepting = initNodePtrVector( 6 );

    NodePtrVector nodeList = getDepthFirstList( true, node );
    for( uint32_t nodeIndex = 0; nodeIndex < nodeList.size; nodeIndex++ ) {
        Node* currNode = getNodePtrVector( &nodeList, nodeIndex );
        if( isNodeAccepting( currNode ) ) {
            pushNodePtrVector( &accepting, currNode );
        } else {
            pushNodePtrVector( &unaccepting, currNode );
        }
    }

    NodePtrVectorVector result = initNodePtrVectorVector( 2 );
    pushNodePtrVectorVector( &result, accepting );
    pushNodePtrVectorVector( &result, unaccepting );
    return result;
}

uint32_t getPartition(NodePtrVectorVector P, Node* node) {
    if( node == NULL ) 
        return P.size;
    for( uint32_t paritionIndex = 0; paritionIndex < P.size; paritionIndex++ ) {
        NodePtrVector parition = getNodePtrVectorVector( &P, paritionIndex );
        for( uint32_t nodeIndex = 0; nodeIndex < parition.size; nodeIndex++ ) {
            if( node == getNodePtrVector( &parition, nodeIndex ) ) {
                return paritionIndex;
            }
        }
    }
}

void splitPartition(NodePtrVectorVector* P, CharVector transitions, uint32_t partitionNum) {
    for( uint32_t charIndex = 0; charIndex < transitions.size; charIndex++ ) {
        if( P->vec[partitionNum].size == 0 ) 
            break;
        char ch = getCharVector( &transitions, charIndex );
        NodePtrVector split = initNodePtrVector( 10 );
        uint32_t activeParition = getPartition( *P, getNodeConnection( getNodePtrVector( &P->vec[partitionNum], 0 ), ch ) );
        for( uint32_t nodeIndex = P->vec[partitionNum].size - 1; nodeIndex >= 1; nodeIndex-- ) {
            Node* node = getNodePtrVector( &P->vec[partitionNum], nodeIndex );
            Node* trans = getNodeConnection( node, ch );
            uint32_t partition = getPartition( *P, trans );
            if( getPartition( *P, getNodeConnection( getNodePtrVector( &P->vec[partitionNum], nodeIndex ), ch ) ) != activeParition ) {
                pushNodePtrVector( &split, getNodePtrVector( &P->vec[partitionNum], nodeIndex ) );
                swapRemoveNodePtrVector( &P->vec[partitionNum], nodeIndex );
            }
        }
        if( split.size == 0 ) {
            freeNodePtrVector( &split );
        } else {
            pushNodePtrVectorVector( P, split );
        }
    }
}

Node* hopcraft(Node* root) {
    CharVector transitionChars = findNonEmptyTransitions( root );
    NodePtrVectorVector P = initialPartition( root );

    if( P.vec[1].size == 0 )
        swapRemoveNodePtrVectorVector( &P, 1 );

    if( P.vec[0].size == 0 )
        swapRemoveNodePtrVectorVector( &P, 0 );

    #ifdef DEBUG
    #define PRINT_PARTITIONS() \
        for( uint32_t ind1s = 0; ind1s < P.size; ind1s++ ) { \
            NodePtrVector pCurr = getNodePtrVectorVector( &P, ind1s ); \
            printf("Partition %u: ", ind1s); \
            for( uint32_t ind2s = 0; ind2s < pCurr.size; ind2s++ ) { \
                printf("%u ", getNodeState( getNodePtrVector( &pCurr, ind2s ) ) ); \
            } \
            printf("\n"); \
        } \
        printf("\n");

    PRINT_PARTITIONS();
    #endif

    if( P.size == 2 ) {
        bool split = true;
        while( split ) {
            split = false;

            uint32_t size = P.size;
            for( uint32_t pIndex = 0; pIndex < size; pIndex++ ) {
                splitPartition( &P, transitionChars, pIndex );
                #ifdef DEBUG
                PRINT_PARTITIONS();
                #endif
            }

            if( size != P.size )
                split = true;
        }
    }

    Node** nodes = (Node**) calloc( P.size, sizeof(Node*) );

    Node* rootNode;

    for( uint32_t nodeIndex = 0; nodeIndex < P.size; nodeIndex++ ) {
        if( P.vec[nodeIndex].size == 0 )
            continue;
        nodes[nodeIndex] = createNode( nodeIndex, isNodeAccepting( P.vec[nodeIndex].vec[0] ) );
        if( containsNodePtrVector( &P.vec[nodeIndex], root ) )
            rootNode = nodes[nodeIndex];
    }

    for( uint32_t nodeIndex = 0; nodeIndex < P.size; nodeIndex++ ) {
        Node* node = P.vec[nodeIndex].vec[0];
        
        for( uint32_t charIndex = 0; charIndex < transitionChars.size; charIndex++ ) {
            char ch = transitionChars.vec[charIndex];

            if( getNodeConnection( node, ch ) != NULL ) {
                addConnection( nodes[nodeIndex], nodes[getPartition( P, getNodeConnection( node, ch ) )], ch );
            }
        }
    }
    
    renumberNodes( rootNode );

    return rootNode;
}

#pragma endregion

/*
NodePtrVector buildNodeList(bool newOp, Node* node) {
    static NodePtrVector vec;

    if( newOp ) {
        vec = initNodePtrVector( 100 );
    }

    pushNodePtrVector( &vec, node );

    for( uint32_t index = 0; index < node->numConnections; index++ ) {
        bool visited = false;
        for( uint32_t checkIndex = 0; checkIndex < vec.size; checkIndex++ ) {
            if( getNodePtrVector( &vec, checkIndex )->state == node->connections[index].ptr->state ) {
                visited = true;
                break;
            }
        }
        if( !visited ) {
            buildNodeList( false, node->connections[index].ptr );
        }
    }
    return vec;
}

#define PRINT_CHAR(character) \
    if( character >= 33 && character <= 126 ) { \
        PRINT "%c", character ); \
    } else { \
        switch( character ) { \
            case '\n': \
                PRINT "\\n"); \
                break; \
            case '\r': \
                PRINT "\\r"); \
                break; \
            default: \
                PRINT "\\%o", character ); \
        } \
    }

void buildNode( Node* node, char* str, uint32_t* currPos) {
    #define PRINT \
        *currPos += sprintf(str + *currPos,

    PRINT "s%u:\t", node->state);
    PRINT "ch = scannerNextChar(scanner);\n");
    bool hasIf = false;
    for( uint32_t connIndex = 0; connIndex < node->numConnections; connIndex++ ) {
        PRINT "\t");
        if( hasIf ) {
            PRINT "} else ");
        }
        PRINT "if(ch == \'");
        PRINT_CHAR( node->connections[connIndex].value );
        PRINT "\') {\n\t\tgoto s%u;\n", node->connections[connIndex].ptr->state);
        hasIf = true;
    }
    if( node->accepting ) {
        PRINT "\t");
        if( hasIf ) {
            PRINT "} else ");
        }
        PRINT "if(ch == %u) {\n\t\ttoken.type = %s;\n\t\treturn token;\n", '\0', "ACCEPT");
    }
    PRINT "\t} else {\n\t\tgoto sE;\n\t}\n");
    #undef PRINT
}

Node* runRegex( Node* node, const char* string ) {
    while( string[0] != '\0' ) {
        node = getNodeConnection( node, *string );
        if( node == NULL )
            return NULL;
        string++;
    }
    return node;
}

void buildRegexFunction( Node* root, const char* fileName, Map map ) {
    char* str = (char*) malloc( 10000 );
    uint32_t strCap = 1000;

    uint32_t strCharsWritten = 0;

    #define PRINT \
        strCharsWritten += sprintf(str + strCharsWritten,

    PRINT "RegexToken regex(TextScanner* scanner) {\n\tToken token;\n\tchar ch;\n\tgoto s0;\n");

    NodePtrVector nodes = buildNodeList( true, root );
    TokenTypeVector correspondingIds = initTokenTypeVector( nodes.size );

    for( uint32_t mapIndex = 0; mapIndex < map.numPairs; mapIndex++ ) {
        Node* correspondingNode = runRegex( root, map.pair[mapIndex].key );
        for( uint32_t nodeIndex = 0; nodeIndex < nodes.size; nodeIndex++ ) {
            if( getNodePtrVector( &nodes, nodeIndex ) == correspondingNode ) {
                correspondingIds.vec[nodeIndex] = map.pair[mapIndex].type;
                break;
            }
        }
    }

    for( uint32_t nodeIndex = 0; nodeIndex < nodes.size; nodeIndex++ ) {
        buildNode( nodes.vec[nodeIndex], str, &strCharsWritten );
    }

    PRINT "sE:\ttoken.type = REGEX_INVALID;\n\treturn token;\n");

    PRINT "}");

    freeNodePtrVector( &nodes );

    FILE* file = fopen( fileName, "w" );
    fwrite( str, sizeof(char), strCharsWritten, file );
    fclose( file );
}

Node genRegexes( Map map ) {
    Node* nodes = (Node*) calloc( map.numPairs, sizeof(Node) );

    Node root;
    root.accepting = false;
    root.numConnections = map.numPairs;
    root.state = 0;
    root.connections = (Edge*) calloc( map.numPairs, sizeof(Edge) );

    for( uint32_t nodeIndex = 0; nodeIndex < map.numPairs; nodeIndex++ ) {
        nodes[nodeIndex] = genMinimalDFA( map.pair[nodeIndex].key );
        root.connections[nodeIndex].ptr = &nodes[nodeIndex];
        root.connections[nodeIndex].value = '\0';
    }

    renumberNodes( true, &root );

    printNodeTree( true, &root );

    Node sub = subsetConstruction( &root );

    renumberNodes( true, &sub );

    printNodeTree( true, &sub );

}
*/

Node* genMinimalDFA( const char* str ) {
    Node* NFA = thompsonsConstruction( str );

    #ifdef DEBUG   
    printf("Completed NFA\n");
    printNodeTree( true, NFA );
    #endif

    Node* DFA = subsetConstruction( NFA );

    #ifdef DEBUG
    printf("Completed DFA\n");
    printNodeTree( true, DFA );
    #endif
    
    Node* MDFA = hopcraft( DFA );

    #ifdef DEBUG
    printf("Completed MDFA\n");
    printNodeTree( true, MDFA );
    #endif

    return MDFA;
}

Node* buildRegexSet( Map* map ) {
    Node** nodes = (Node**) calloc( map->numPairs, sizeof( Node* ) ); 
    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        nodes[index] = genMinimalDFA( map->pairs[index].key ); 
        NodePtrVector vec = getAcceptingNodes( nodes[index] );
        for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
            addNodeString( getNodePtrVector( &vec, nodeIndex ), map->pairs[index].type );
        }
    }
    
    Node* root = createNode( 0, false );
    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        addConnection( root, nodes[index], '\0' ); 
    }

    //Node* empty = createNode( 0, false );
    //addConnection( empty, createNode( 1, true ), '\0' );
    //addNodeString( getNodeConnection( empty, '\0' ), "ERROR" );
    //addConnection( root, empty, '\0' );

    renumberNodes( root );

    #ifdef DEBUG
    printf("NFA connected\n");
    printNodeTree( true, root );
    #endif

    root = subsetConstruction( root );
    renumberNodes( root );

    #ifdef DEBUG
    printf("Final Tree\n");
    printNodeTree( true, root );
    #endif

    return root;
}
