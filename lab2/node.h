#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

typedef enum nodeType
{
    INT_TOKEN,
    FLOAT_TOKEN,
    ID_TOKEN,
    TYPE_TOKEN,
    OTHER_TOKEN,
    NOT_A_TOKEN
} NodeType;

typedef struct node
{
    int line;
    NodeType type;
    char* name;
    char* val;
    struct node* child;
    struct node* sibling;
} Node;

typedef Node* pNode;

static inline pNode newNode(int line, NodeType type, char* name, int argc, ...)
{
    pNode curNode = (pNode)malloc(sizeof(Node));
    assert(curNode != NULL);
    
    curNode->line = line;
    curNode->type = type;
    
    int nameLength = strlen(name) + 1;
    curNode->name = (char*)malloc(sizeof(char) * nameLength);
    assert(curNode->name != NULL);
    strncpy(curNode->name, name, nameLength);

    va_list arg_ptr;
    va_start(arg_ptr, argc);
    pNode tempNode = va_arg(arg_ptr, pNode);

    curNode->child = tempNode;
    for (int i = 1; i < argc; i++)
    {
        tempNode->sibling = va_arg(arg_ptr, pNode);
        if (tempNode->sibling != NULL)
        {
            tempNode = tempNode->sibling;
        }
    }

    va_end(arg_ptr);
    return curNode;
}

static inline pNode newTokenNode(int line, NodeType type, char* tokenName, char* tokenText)
{
    pNode tokenNode = (pNode)malloc(sizeof(Node));
    assert(tokenNode != NULL);

    tokenNode->line = line;
    tokenNode->type = type;

    int nameLength = strlen(tokenName) + 1;
    int textLength = strlen(tokenText) + 1;
        
    tokenNode->name = (char*)malloc(sizeof(char) * nameLength);
    tokenNode->val = (char*)malloc(sizeof(char) * textLength);

    assert(tokenNode->name != NULL);
    assert(tokenNode->val != NULL);

    strncpy(tokenNode->name, tokenName, nameLength);
    strncpy(tokenNode->val, tokenText, textLength);

    tokenNode->child = NULL;
    tokenNode->sibling = NULL;

    return tokenNode;
}

static inline void delNode(pNode node)
{
    if (node == NULL) return;
    while (node->child != NULL)
    {
        pNode temp = node->child;
        node->child = node->child->sibling;
        delNode(temp);
    }
    free(node);
    node = NULL;
    return;
}

static inline void printTree(pNode curNode,int line)
{
    if (curNode == NULL) return;
    for (int i = 1; i < line; i++)
    {
        printf("  ");
    }
    printf("%s", curNode->name);
    if (curNode->type == NOT_A_TOKEN)
    {
        printf(" (%d)", curNode->line);
    }
    else if (curNode->type == TYPE_TOKEN || curNode->type == ID_TOKEN || curNode->type == INT_TOKEN)
    {
        printf(": %s", curNode->val);
    }
    else if (curNode->type == FLOAT_TOKEN)
    {
        printf(": %lf", atof(curNode->val));
    }
    printf("\n");
    printTree(curNode->child, line + 1);
    printTree(curNode->sibling, line);
}

#endif
