#ifndef INTER_H
#define INTER_H
#include "node.h"
#include "semantic.h"

typedef struct operand* pOperand;
typedef struct interCode* pInterCode;
typedef struct interCodes* pInterCodes;
typedef struct arg* pArg;
typedef struct argList* pArgList;
typedef struct interCodeList* pInterCodeList;

typedef struct operand {
    enum {
        OP_VARIABLE,
        OP_CONSTANT,
        OP_ADDRESS,
        OP_LABEL,
        OP_FUNCTION,
        OP_RELOP,
    } kind;

    union {
        int value;
        char* name;
    } u;
} Operand;

typedef struct interCode {
    enum {
        IR_LABEL,
        IR_FUNCTION,
        IR_ASSIGN,
        IR_ADD,
        IR_SUB,
        IR_MUL,
        IR_DIV,
        IR_GET_ADDR,
        IR_READ_ADDR,
        IR_WRITE_ADDR,
        IR_GOTO,
        IR_IF_GOTO,
        IR_RETURN,
        IR_DEC,
        IR_ARG,
        IR_CALL,
        IR_PARAM,
        IR_READ,
        IR_WRITE,
    } kind;

    union {
        struct {
            pOperand op;
        } oneOp;
        struct {
            pOperand right, left;
        } assign;
        struct {
            pOperand result, op1, op2;
        } binOp;
        struct {
            pOperand x, relop, y, z;
        } ifGoto;
        struct {
            pOperand op;
            int size;
        } dec;
    } u;
} InterCode;

typedef struct interCodes {
    pInterCode code;
    pInterCodes *prev, *next;
} InterCodes;

typedef struct arg {
    pOperand op;
    pArg next;
} Arg;

typedef struct argList {
    pArg head;
    pArg cur;
} ArgList;

typedef struct interCodeList {
    pInterCodes head;
    pInterCodes cur;
    char* lastArrayName;
    int tempVarNum;
    int labelNum;
} InterCodeList;

extern boolean interError;
extern pInterCodeList interCodeList;

// Operand func
pOperand newOperand(int kind, ...);
void setOperand(pOperand p, int kind, void* val);
void deleteOperand(pOperand p);
void printOp(FILE* fp, pOperand op);

// InterCode func
pInterCode newInterCode(int kind, ...);
void deleteInterCode(pInterCode p);
void printInterCode(FILE* fp, pInterCodeList interCodeList);

// InterCodes func
pInterCodes newInterCodes(pInterCode code);
void deleteInterCodes(pInterCodes p);

// Arg and ArgList func
pArg newArg(pOperand op);
pArgList newArgList();
void deleteArg(pArg p);
void deleteArgList(pArgList p);
void addArg(pArgList argList, pArg arg);

// InterCodeList func
pInterCodeList newInterCodeList();
void deleteInterCodeList(pInterCodeList p);
void addInterCode(pInterCodeList interCodeList, pInterCodes newCode);

// traverse func
pOperand newTemp();
pOperand newLabel();
int getSize(pType type);
void genInterCodes(pNode node);
void genInterCode(int kind, ...);
void translateExp(pNode node, pOperand place);
void translateArgs(pNode node, pArgList argList);
void translateCond(pNode node, pOperand labelTrue, pOperand labelFalse);
void translateVarDec(pNode node, pOperand place);
void translateDec(pNode node);
void translateDecList(pNode node);
void translateDef(pNode node);
void translateDefList(pNode node);
void translateCompSt(pNode node);
void translateStmt(pNode node);
void translateStmtList(pNode node);
void translateFunDec(pNode node);
void translateExtDef(pNode node);
void translateExtDefList(pNode node);

#endif
