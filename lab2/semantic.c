#include "semantic.h"

pTable table;

// Global function
static inline char* newString(char* src) {
    if (src == NULL) return NULL;
    int length = strlen(src) + 1;
    char* p = (char*)malloc(sizeof(char) * length);
    assert(p != NULL);
    strncpy(p, src, length);
    return p;
}

static inline unsigned int getHashCode(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if (i = val & ~HASH_TABLE_SIZE)
            val = (val ^ (i >> 12)) & HASH_TABLE_SIZE;
    }
    return val;
}

static inline void pError(ErrorType type, int line, char* msg) {
    printf("Error type %d at Line %d: %s\n", type, line, msg);
}

void traverseTree(pNode node) {
    if (node == NULL) return;
    if (!strcmp(node->name, "ExtDef")) 
        ExtDef(node);
    traverseTree(node->child);
    traverseTree(node->sibling);
}

// Type functions
pType newType(Kind kind, int argc, ...) {
    pType p = (pType)malloc(sizeof(Type));
    assert(p != NULL);
    p->kind = kind;
    va_list arg_ptr;
    assert(kind == BASIC || kind == ARRAY || kind == STRUCTURE || kind == FUNCTION);
    switch (kind) {
        case BASIC:
            va_start(arg_ptr, argc);
            p->u.basic = va_arg(arg_ptr, BasicType);
            break;
        case ARRAY:
            va_start(arg_ptr, argc);
            p->u.array.elem = va_arg(arg_ptr, pType);
            p->u.array.size = va_arg(arg_ptr, int);
            break;
        case STRUCTURE:
            va_start(arg_ptr, argc);
            p->u.structure.structName = va_arg(arg_ptr, char*);
            p->u.structure.field = va_arg(arg_ptr, pFieldList);
            break;
        case FUNCTION:
            va_start(arg_ptr, argc);
            p->u.function.argc = va_arg(arg_ptr, int);
            p->u.function.argv = va_arg(arg_ptr, pFieldList);
            p->u.function.returnType = va_arg(arg_ptr, pType);
            break;
    }
    va_end(arg_ptr);
    return p;
}

pType copyType(pType src) {
    if (src == NULL) return NULL;
    pType p = (pType)malloc(sizeof(Type));
    assert(p != NULL);
    p->kind = src->kind;
    assert(p->kind == BASIC || p->kind == ARRAY || p->kind == STRUCTURE || p->kind == FUNCTION);
    switch (p->kind) {
        case BASIC:
            p->u.basic = src->u.basic;
            break;
        case ARRAY:
            p->u.array.elem = copyType(src->u.array.elem);
            p->u.array.size = src->u.array.size;
            break;
        case STRUCTURE:
            p->u.structure.structName = newString(src->u.structure.structName);
            p->u.structure.field = copyFieldList(src->u.structure.field);
            break;
        case FUNCTION:
            p->u.function.argc = src->u.function.argc;
            p->u.function.argv = copyFieldList(src->u.function.argv);
            p->u.function.returnType = copyType(src->u.function.returnType);
            break;
    }
    return p;
}

void deleteType(pType type) {
    assert(type != NULL);
    assert(type->kind == BASIC || type->kind == ARRAY || type->kind == STRUCTURE || type->kind == FUNCTION);
    pFieldList temp = NULL;
    switch (type->kind) {
        case BASIC:
            break;
        case ARRAY:
            deleteType(type->u.array.elem);
            type->u.array.elem = NULL;
            break;
        case STRUCTURE:
            if (!type->u.structure.structName)
                free(type->u.structure.structName);
            type->u.structure.structName = NULL;

            temp = type->u.structure.field;
            while (temp) {
                pFieldList tDelete = temp;
                temp = temp->tail;
                deleteFieldList(tDelete);
            }
            type->u.structure.field = NULL;
            break;
        case FUNCTION:          
            temp = type->u.function.argv;
            while (temp) {
                pFieldList tDelete = temp;
                temp = temp->tail;
                deleteFieldList(tDelete);
            }
            type->u.function.argv = NULL;
            
            deleteType(type->u.function.returnType);
            type->u.function.returnType = NULL;
            break;
    }
    free(type);
    type = NULL;
}

boolean checkType(pType type1, pType type2) {
    if (type1 == NULL || type2 == NULL) return TRUE;
    if (type1->kind == FUNCTION || type2->kind == FUNCTION) return FALSE;
    if (type1->kind != type2->kind)
        return FALSE;
    else {
        assert(type1->kind == BASIC || type1->kind == ARRAY || type1->kind == STRUCTURE);
        switch (type1->kind) {
            case BASIC:
                return type1->u.basic == type2->u.basic;
            case ARRAY:
                return checkType(type1->u.array.elem, type2->u.array.elem);
            case STRUCTURE:
                return !strcmp(type1->u.structure.structName, type2->u.structure.structName);
        }
    }
}

// FieldList functions
pFieldList newFieldList(char* newName, pType newType) {
    pFieldList p = (pFieldList)malloc(sizeof(FieldList));
    assert(p != NULL);
    p->name = newString(newName);
    p->type = newType;
    p->tail = NULL;
    return p;
}

pFieldList copyFieldList(pFieldList src) {
    assert(src != NULL);
    pFieldList head = NULL, cur = NULL;
    pFieldList temp = src;

    while (temp) {
        if (!head) {
            head = newFieldList(temp->name, copyType(temp->type));
            cur = head;
            temp = temp->tail;
        }
        else {
            cur->tail = newFieldList(temp->name, copyType(temp->type));
            cur = cur->tail;
            temp = temp->tail;
        }
    }
    return head;
}

void deleteFieldList(pFieldList fieldList) {
    assert(fieldList != NULL);
    if (fieldList->name) {
        free(fieldList->name);
        fieldList->name = NULL;
    }
    if (fieldList->type) 
        deleteType(fieldList->type);
    fieldList->type = NULL;
    free(fieldList);
    fieldList = NULL;
}

void setFieldListName(pFieldList p, char* newName) {
    assert(p != NULL && newName != NULL);
    if (p->name != NULL)
        free(p->name);
    p->name = newString(newName);
}

// tableItem functions
pItem newItem(int symbolDepth, pFieldList pfield) {
    pItem p = (pItem)malloc(sizeof(TableItem));
    assert(p != NULL);
    p->symbolDepth = symbolDepth;
    p->field = pfield;
    p->nextHash = NULL;
    p->nextSymbol = NULL;
    return p;
}

void deleteItem(pItem item) {
    assert(item != NULL);
    if (item->field != NULL) 
        deleteFieldList(item->field);
    free(item);
    item = NULL;
}

boolean isStructDef(pItem src) {
    if (src == NULL) 
        return FALSE;
    if (src->field->type->kind != STRUCTURE) 
        return FALSE;
    if (src->field->type->u.structure.structName) 
        return FALSE;
    return TRUE;
}

// Hash functions
pHash newHash() {
    pHash p = (pHash)malloc(sizeof(HashTable));
    assert(p != NULL);
    p->hashArray = (pItem*)malloc(sizeof(pItem) * HASH_TABLE_SIZE);
    assert(p->hashArray != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
        p->hashArray[i] = NULL;
    return p;
}

void deleteHash(pHash hash) {
    assert(hash != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        pItem temp = hash->hashArray[i];
        while (temp) {
            pItem tdelete = temp;
            temp = temp->nextHash;
            deleteItem(tdelete);
        }
        hash->hashArray[i] = NULL;
    }
    free(hash->hashArray);
    hash->hashArray = NULL;
    free(hash);
    hash  = NULL;
}

pItem getHashHead(pHash hash, int index) {
    assert(hash != NULL);
    return hash->hashArray[index];
}

void setHashHead(pHash hash, int index, pItem newVal) {
    assert(hash != NULL);
    hash->hashArray[index] = newVal;
}

// Stack functions
pStack newStack() {
    pStack p = (pStack)malloc(sizeof(Stack));
    assert(p != NULL);
    p->stackArray = (pItem*)malloc(sizeof(pItem) * HASH_TABLE_SIZE);
    assert(p->stackArray != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
        p->stackArray[i] = NULL;
    p->curStackDepth = 0;
    return p;
}

void deleteStack(pStack stack) {
    assert(stack != NULL);
    free(stack->stackArray);
    stack->stackArray = NULL;
    stack->curStackDepth = 0;
    free(stack);
    stack = NULL;
}

void addStackDepth(pStack stack) {
    assert(stack != NULL);
    stack->curStackDepth++;
}

void minusStackDepth(pStack stack) {
    assert(stack != NULL);
    stack->curStackDepth--;
}

pItem getCurDepthStackHead(pStack stack) {
    assert(stack != NULL);
    return stack->stackArray[stack->curStackDepth];
}

void setCurDepthStackHead(pStack stack, pItem newVal) {
    assert(stack != NULL);
    stack->stackArray[stack->curStackDepth] = newVal;
}

// Table functions
pTable initTable() {
    pTable table = (pTable)malloc(sizeof(Table));
    assert(table != NULL);
    table->hash = newHash();
    table->stack = newStack();
    table->unNamedStructNum = 0;
    return table;
};

void deleteTable(pTable table) {
    deleteHash(table->hash);
    table->hash = NULL;
    deleteStack(table->stack);
    table->stack = NULL;
    free(table);
    table = NULL;
};

pItem searchTableItem(pTable table, char* name) {
    unsigned hashCode = getHashCode(name);
    pItem temp = getHashHead(table->hash, hashCode);
    if (temp == NULL) 
        return NULL;
    while (temp) {
        if (!strcmp(temp->field->name, name)) 
            return temp;
        temp = temp->nextHash;
    }
    return NULL;
}

boolean checkTableItemConflict(pTable table, pItem item) {
    pItem temp = searchTableItem(table, item->field->name);
    if (temp == NULL) 
        return FALSE;
    while (temp) {
        if (!strcmp(temp->field->name, item->field->name)) {
            if (temp->field->type->kind == STRUCTURE || item->field->type->kind == STRUCTURE)
                return TRUE;
            if (temp->symbolDepth == table->stack->curStackDepth) 
                return TRUE;
        }
        temp = temp->nextHash;
    }
    return FALSE;
}

void addTableItem(pTable table, pItem item) {
    assert(table != NULL && item != NULL);
    unsigned hashCode = getHashCode(item->field->name);
    pHash hash = table->hash;
    pStack stack = table->stack;

    item->nextSymbol = getCurDepthStackHead(stack);
    setCurDepthStackHead(stack, item);

    item->nextHash = getHashHead(hash, hashCode);
    setHashHead(hash, hashCode, item);
}

void deleteTableItem(pTable table, pItem item) {
    assert(table != NULL && item != NULL);
    unsigned hashCode = getHashCode(item->field->name);
    if (item == getHashHead(table->hash, hashCode))
        setHashHead(table->hash, hashCode, item->nextHash);
    else {
        pItem cur = getHashHead(table->hash, hashCode), last;
        while (cur != item) {
            last = cur;
            cur = cur->nextHash;
        }
        last->nextHash = cur->nextHash;
    }
    deleteItem(item);
}

void clearCurDepthStackList(pTable table) {
    assert(table != NULL);
    pStack stack = table->stack;
    pItem temp = getCurDepthStackHead(stack);
    while (temp) {
        pItem tDelete = temp;
        temp = temp->nextSymbol;
        deleteTableItem(table, tDelete);
    }
    setCurDepthStackHead(stack, NULL);
    minusStackDepth(stack);
}

// Generate symbol table functions
void ExtDef(pNode node) {
    assert(node != NULL);
    // ExtDef -> Specifier ExtDecList SEMI
    //         | Specifier SEMI
    //         | Specifier FunDec CompSt
    pType specifierType = Specifier(node->child);
    char* secondName = node->child->sibling->name;

    // ExtDef -> Specifier ExtDecList SEMI
    if (!strcmp(secondName, "ExtDecList"))
        ExtDecList(node->child->sibling, specifierType);

    // ExtDef -> Specifier FunDec CompSt
    else if (!strcmp(secondName, "FunDec")) {
        FunDec(node->child->sibling, specifierType);
        CompSt(node->child->sibling->sibling, specifierType);
    }
    if (specifierType)
        deleteType(specifierType);

    // ExtDef -> Specifier SEMI
    // this situation has no meaning
    // or is struct define(have been processe in Specifier())
}

void ExtDecList(pNode node, pType specifier) {
    assert(node != NULL);
    // ExtDecList -> VarDec
    //             | VarDec COMMA ExtDecList
    pNode temp = node;
    while (temp) {
        pItem item = VarDec(temp->child, specifier);
        if (checkTableItemConflict(table, item)) {
            char msg[100] = {0};
            sprintf(msg, "Redefined variable \"%s\".", item->field->name);
            pError(REDEF_VAR, temp->line, msg);
            deleteItem(item);
        }
        else
            addTableItem(table, item);
        if (temp->child->sibling)
            temp = temp->sibling->sibling->child;
        else
            break;
    }
}

pType Specifier(pNode node) {
    assert(node != NULL);
    // Specifier -> TYPE
    //            | StructSpecifier
    pNode t = node->child;
    // Specifier -> TYPE
    if (!strcmp(t->name, "TYPE")) {
        if (!strcmp(t->val, "float"))
            return newType(BASIC, 1, FLOAT_TYPE);
        else
            return newType(BASIC, 1, INT_TYPE);
    }
    // Specifier -> StructSpecifier
    else
        return StructSpecifier(t);
}

pType StructSpecifier(pNode node) {
    assert(node != NULL);
    // StructSpecifier -> STRUCT OptTag LC DefList RC
    //                  | STRUCT Tag

    // OptTag -> ID | e
    // Tag -> ID
    pType returnType = NULL;
    pNode t = node->child->sibling;
    // StructSpecifier->STRUCT OptTag LC DefList RC
    if (strcmp(t->name, "Tag")) {
        pItem structItem = newItem(table->stack->curStackDepth, newFieldList("", newType(STRUCTURE, 2, NULL, NULL)));
        if (!strcmp(t->name, "OptTag")) {
            setFieldListName(structItem->field, t->child->val);
            t = t->sibling;
        }
        else {
            table->unNamedStructNum++;
            char structName[20] = {0};
            sprintf(structName, "%d", table->unNamedStructNum);
            setFieldListName(structItem->field, structName);
        }

        if (!strcmp(t->sibling->name, "DefList"))
            DefList(t->sibling, structItem);

        if (checkTableItemConflict(table, structItem)) {
            char msg[100] = {0};
            sprintf(msg, "Duplicated name \"%s\".", structItem->field->name);
            pError(DUPLICATED_NAME, node->line, msg);
            deleteItem(structItem);
        }
        else {
            returnType = newType(STRUCTURE, 2, newString(structItem->field->name), copyFieldList(structItem->field->type->u.structure.field));
            if (!strcmp(node->child->sibling->name, "OptTag"))
                addTableItem(table, structItem);
            // OptTag -> e
            else
                deleteItem(structItem);
        }
    }

    // StructSpecifier->STRUCT Tag
    else {
        pItem structItem = searchTableItem(table, t->child->val);
        if (structItem == NULL || !isStructDef(structItem)) {
            char msg[100] = {0};
            sprintf(msg, "Undefined structure \"%s\".", t->child->val);
            pError(UNDEF_STRUCT, node->line, msg);
        } 
        else
            returnType = newType(STRUCTURE, 2, newString(structItem->field->name), copyFieldList(structItem->field->type->u.structure.field));
    }
    return returnType;
}

pItem VarDec(pNode node, pType specifier) {
    assert(node != NULL);
    // VarDec -> ID
    //         | VarDec LB INT RB
    pNode id = node;
    while (id->child) id = id->child;
    pItem p = newItem(table->stack->curStackDepth, newFieldList(id->val, NULL));

    // VarDec -> ID
    if (!strcmp(node->child->name, "ID"))
        p->field->type = copyType(specifier);
    // VarDec -> VarDec LB INT RB
    else {
        pNode varDec = node->child;
        pType temp = specifier;
        while (varDec->sibling) {
            p->field->type = newType(ARRAY, 2, copyType(temp), atoi(varDec->sibling->sibling->val));
            temp = p->field->type;
            varDec = varDec->child;
        }
    }
    return p;
}

void FunDec(pNode node, pType returnType) {
    assert(node != NULL);
    // FunDec -> ID LP VarList RP
    //         | ID LP RP
    pItem p = newItem(table->stack->curStackDepth, newFieldList(node->child->val, newType(FUNCTION, 3, 0, NULL, copyType(returnType))));

    // FunDec -> ID LP VarList RP
    if (!strcmp(node->child->sibling->sibling->name, "VarList"))
        VarList(node->child->sibling->sibling, p);

    // FunDec -> ID LP RP don't need process

    // check redefine
    if (checkTableItemConflict(table, p)) {
        char msg[100] = {0};
        sprintf(msg, "Redefined function \"%s\".", p->field->name);
        pError(REDEF_FUNC, node->line, msg);
        deleteItem(p);
        p = NULL;
    }
    else
        addTableItem(table, p);
}

void VarList(pNode node, pItem func) {
    assert(node != NULL);
    // VarList -> ParamDec COMMA VarList
    //          | ParamDec
    addStackDepth(table->stack);
    int argc = 0;
    pNode temp = node->child;
    pFieldList cur = NULL;

    // VarList -> ParamDec
    pFieldList paramDec = ParamDec(temp);
    func->field->type->u.function.argv = copyFieldList(paramDec);
    cur = func->field->type->u.function.argv;
    argc++;

    // VarList -> ParamDec COMMA VarList
    while (temp->sibling) {
        temp = temp->sibling->sibling->child;
        paramDec = ParamDec(temp);
        if (paramDec) {
            cur->tail = copyFieldList(paramDec);
            cur = cur->tail;
            argc++;
        }
    }

    func->field->type->u.function.argc = argc;
    minusStackDepth(table->stack);
}

pFieldList ParamDec(pNode node) {
    assert(node != NULL);
    // ParamDec -> Specifier VarDec
    pType specifierType = Specifier(node->child);
    pItem p = VarDec(node->child->sibling, specifierType);
    if (specifierType) 
        deleteType(specifierType);
    if (checkTableItemConflict(table, p)) {
        char msg[100] = {0};
        sprintf(msg, "Redefined variable \"%s\".", p->field->name);
        pError(REDEF_VAR, node->line, msg);
        deleteItem(p);
        return NULL;
    } 
    else {
        addTableItem(table, p);
        return p->field;
    }
}

void CompSt(pNode node, pType returnType) {
    assert(node != NULL);
    // CompSt -> LC DefList StmtList RC
    addStackDepth(table->stack);
    pNode temp = node->child->sibling;
    if (!strcmp(temp->name, "DefList")) {
        DefList(temp, NULL);
        temp = temp->sibling;
    }
    if (!strcmp(temp->name, "StmtList")) {
        StmtList(temp, returnType);
    }

    clearCurDepthStackList(table);
}

void StmtList(pNode node, pType returnType) {
    // StmtList -> Stmt StmtList
    //           | e
    while (node) {
        Stmt(node->child, returnType);
        node = node->child->sibling;
    }
}

void Stmt(pNode node, pType returnType) {
    assert(node != NULL);
    // Stmt -> Exp SEMI
    //       | CompSt
    //       | RETURN Exp SEMI
    //       | IF LP Exp RP Stmt
    //       | IF LP Exp RP Stmt ELSE Stmt
    //       | WHILE LP Exp RP Stmt

    pType expType = NULL;
    // Stmt -> Exp SEMI
    if (!strcmp(node->child->name, "Exp")) 
        expType = Exp(node->child);

    // Stmt -> CompSt
    else if (!strcmp(node->child->name, "CompSt"))
        CompSt(node->child, returnType);

    // Stmt -> RETURN Exp SEMI
    else if (!strcmp(node->child->name, "RETURN")) {
        expType = Exp(node->child->sibling);

        // check return type
        if (!checkType(returnType, expType))
            pError(TYPE_MISMATCH_RETURN, node->line, "Type mismatched for return.");
    }

    // Stmt -> IF LP Exp RP Stmt
    else if (!strcmp(node->child->name, "IF")) {
        pNode stmt = node->child->sibling->sibling->sibling->sibling;
        expType = Exp(node->child->sibling->sibling);
        Stmt(stmt, returnType);
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        if (stmt->sibling != NULL)
            Stmt(stmt->sibling->sibling, returnType);
    }

    // Stmt -> WHILE LP Exp RP Stmt
    else if (!strcmp(node->child->name, "WHILE")) {
        expType = Exp(node->child->sibling->sibling);
        Stmt(node->child->sibling->sibling->sibling->sibling, returnType);
    }

    if (expType)
        deleteType(expType);
}

void DefList(pNode node, pItem structInfo) {
    // DefList -> Def DefList
    //          | e
    while (node) {
        Def(node->child, structInfo);
        node = node->child->sibling;
    }
}

void Def(pNode node, pItem structInfo) {
    assert(node != NULL);
    // Def -> Specifier DecList SEMI
    pType dectype = Specifier(node->child);

    DecList(node->child->sibling, dectype, structInfo);
    if (dectype)
        deleteType(dectype);
}

void DecList(pNode node, pType specifier, pItem structInfo) {
    assert(node != NULL);
    // DecList -> Dec
    //          | Dec COMMA DecList
    pNode temp = node;
    while (temp) {
        Dec(temp->child, specifier, structInfo);
        if (temp->child->sibling)
            temp = temp->child->sibling->sibling;
        else
            break;
    }
}

void Dec(pNode node, pType specifier, pItem structInfo) {
    assert(node != NULL);
    // Dec -> VarDec
    //      | VarDec ASSIGNOP Exp

    // Dec -> VarDec
    if (node->child->sibling == NULL) {
        if (structInfo != NULL) {
            pItem decitem = VarDec(node->child, specifier);
            pFieldList payload = decitem->field;
            pFieldList structField = structInfo->field->type->u.structure.field;
            pFieldList last = NULL;
            while (structField != NULL) {
                if (!strcmp(payload->name, structField->name)) {
                    char msg[100] = {0};
                    sprintf(msg, "Redefined field \"%s\".", decitem->field->name);
                    pError(REDEF_FEILD, node->line, msg);
                    deleteItem(decitem);
                    return;
                }
                else {
                    last = structField;
                    structField = structField->tail;
                }
            }
            if (last == NULL)
                structInfo->field->type->u.structure.field = copyFieldList(decitem->field);
            else
                last->tail = copyFieldList(decitem->field);
            deleteItem(decitem);
        } 
        else {
            pItem decitem = VarDec(node->child, specifier);
            if (checkTableItemConflict(table, decitem)) {
                char msg[100] = {0};
                sprintf(msg, "Redefined variable \"%s\".", decitem->field->name);
                pError(REDEF_VAR, node->line, msg);
                deleteItem(decitem);
            }
            else
                addTableItem(table, decitem);
        }
    }
    // Dec -> VarDec ASSIGNOP Exp
    else {
        if (structInfo != NULL)
            pError(REDEF_FEILD, node->line, "Illegal initialize variable in struct.");
        else {
            pItem decitem = VarDec(node->child, specifier);
            pType exptype = Exp(node->child->sibling->sibling);
            if (checkTableItemConflict(table, decitem)) {
                char msg[100] = {0};
                sprintf(msg, "Redefined variable \"%s\".", decitem->field->name);
                pError(REDEF_VAR, node->line, msg);
                deleteItem(decitem);
            }
            if (!checkType(decitem->field->type, exptype)) {
                pError(TYPE_MISMATCH_ASSIGN, node->line, "Type mismatchedfor assignment.");
                deleteItem(decitem);
            }
            if (decitem->field->type && decitem->field->type->kind == ARRAY) {
                pError(TYPE_MISMATCH_ASSIGN, node->line, "Illegal initialize variable.");
                deleteItem(decitem);
            } 
            else
                addTableItem(table, decitem);
            if (exptype) 
                deleteType(exptype);
        }
    }
}

pType Exp(pNode node) {
    assert(node != NULL);
    // Exp -> Exp ASSIGNOP Exp
    //      | Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp
    //      | Exp PLUS Exp
    //      | Exp MINUS Exp
    //      | Exp STAR Exp
    //      | Exp DIV Exp
    //      | LP Exp RP
    //      | MINUS Exp
    //      | NOT Exp
    //      | ID LP Args RP
    //      | ID LP RP
    //      | Exp LB Exp RB
    //      | Exp DOT ID
    //      | ID
    //      | INT
    //      | FLOAT
    pNode t = node->child;
    if (!strcmp(t->name, "Exp")) {
        if (strcmp(t->sibling->name, "LB") && strcmp(t->sibling->name, "DOT")) {
            pType p1 = Exp(t);
            pType p2 = Exp(t->sibling->sibling);
            pType returnType = NULL;

            // Exp -> Exp ASSIGNOP Exp
            if (!strcmp(t->sibling->name, "ASSIGNOP")) {
                pNode tchild = t->child;

                if (!strcmp(tchild->name, "FLOAT") || !strcmp(tchild->name, "INT")) {
                    pError(LEFT_VAR_ASSIGN, t->line, "The left-hand side of an assignment must be  avariable.");

                } 
                else if (!strcmp(tchild->name, "ID") || !strcmp(tchild->sibling->name, "LB") || !strcmp(tchild->sibling->name, "DOT")) {
                    if (!checkType(p1, p2))
                        pError(TYPE_MISMATCH_ASSIGN, t->line, "Type mismatched for assignment.");
                    else
                        returnType = copyType(p1);
                } 
                else {
                    pError(LEFT_VAR_ASSIGN, t->line, "The left-hand side of an assignment must be avariable.");
                }

            }
            // Exp -> Exp AND Exp
            //      | Exp OR Exp
            //      | Exp RELOP Exp
            //      | Exp PLUS Exp
            //      | Exp MINUS Exp
            //      | Exp STAR Exp
            //      | Exp DIV Exp
            else {
                if (p1 && p2 && (p1->kind == ARRAY || p2->kind == ARRAY))
                    pError(TYPE_MISMATCH_OP, t->line, "Type mismatched for operands.");
                else if (!checkType(p1, p2))
                    pError(TYPE_MISMATCH_OP, t->line, "Type mismatched for operands.");
                else {
                    if (p1 && p2)
                        returnType = copyType(p1);
                }
            }

            if (p1)
                deleteType(p1);
            if (p2)
                deleteType(p2);
            return returnType;
        }
        else {
            // Exp -> Exp LB Exp RB
            if (!strcmp(t->sibling->name, "LB")) {
                pType p1 = Exp(t);
                pType p2 = Exp(t->sibling->sibling);
                pType returnType = NULL;

                if (!p1) { }
                else if (p1 && p1->kind != ARRAY) {
                    char msg[100] = {0};
                    sprintf(msg, "\"%s\" is not an array.", t->child->val);
                    pError(NOT_A_ARRAY, t->line, msg);
                } 
                else if (!p2 || p2->kind != BASIC || p2->u.basic != INT_TYPE) {
                    char msg[100] = {0};
                    sprintf(msg, "\"%s\" is not an integer.", t->sibling->sibling->child->val);
                    pError(NOT_A_INT, t->line, msg);
                }
                else
                    returnType = copyType(p1->u.array.elem);
                if (p1)
                    deleteType(p1);
                if (p2)
                    deleteType(p2);
                return returnType;
            }
            // Exp -> Exp DOT ID
            else {
                pType p1 = Exp(t);
                pType returnType = NULL;
                if (!p1 || p1->kind != STRUCTURE || !p1->u.structure.structName)
                    pError(ILLEGAL_USE_DOT, t->line, "Illegal use of \".\".");
                else {
                    pNode ref_id = t->sibling->sibling;
                    pFieldList structfield = p1->u.structure.field;
                    while (structfield != NULL) {
                        if (!strcmp(structfield->name, ref_id->val))
                            break;
                        structfield = structfield->tail;
                    }
                    if (structfield == NULL) {
                        char msg[100] = {0};
                        sprintf(msg, "Non-existent field \"%s\".", ref_id->val);
                        pError(NONEXISTFIELD, t->line, msg);
                    }
                    else
                        returnType = copyType(structfield->type);
                }
                if (p1) deleteType(p1);
                return returnType;
            }
        }
    }
    // Exp -> MINUS Exp
    //      | NOT Exp
    else if (!strcmp(t->name, "MINUS") || !strcmp(t->name, "NOT")) {
        pType p1 = Exp(t->sibling);
        pType returnType = NULL;
        if (!p1 || p1->kind != BASIC)
            printf("Error type %d at Line %d: %s.\n", 7, t->line, "TYPE_MISMATCH_OP");
        else
            returnType = copyType(p1);
        if (p1) 
            deleteType(p1);
        return returnType;
    } 
    else if (!strcmp(t->name, "LP"))
        return Exp(t->sibling);
    // Exp -> ID LP Args RP
    //		| ID LP RP
    else if (!strcmp(t->name, "ID") && t->sibling) {
        pItem funcInfo = searchTableItem(table, t->val);

        if (funcInfo == NULL) {
            char msg[100] = {0};
            sprintf(msg, "Undefined function \"%s\".", t->val);
            pError(UNDEF_FUNC, node->line, msg);
            return NULL;
        } 
        else if (funcInfo->field->type->kind != FUNCTION) {
            char msg[100] = {0};
            sprintf(msg, "\"%s\" is not a function.", t->val);
            pError(NOT_A_FUNC, node->line, msg);
            return NULL;
        }
        // Exp -> ID LP Args RP
        else if (!strcmp(t->sibling->sibling->name, "Args")) {
            Args(t->sibling->sibling, funcInfo);
            return copyType(funcInfo->field->type->u.function.returnType);
        }
        // Exp -> ID LP RP
        else {
            if (funcInfo->field->type->u.function.argc != 0) {
                char msg[100] = {0};
                sprintf(msg, "too few arguments to function \"%s\", except %d args.", funcInfo->field->name, funcInfo->field->type->u.function.argc);
                pError(FUNC_AGRC_MISMATCH, node->line, msg);
            }
            return copyType(funcInfo->field->type->u.function.returnType);
        }
    }
    // Exp -> ID
    else if (!strcmp(t->name, "ID")) {
        pItem tp = searchTableItem(table, t->val);
        if (tp == NULL || isStructDef(tp)) {
            char msg[100] = {0};
            sprintf(msg, "Undefined variable \"%s\".", t->val);
            pError(UNDEF_VAR, t->line, msg);
            return NULL;
        } 
        else
            return copyType(tp->field->type);
    } 
    else {
        // Exp -> FLOAT
        if (!strcmp(t->name, "FLOAT"))
            return newType(BASIC, 1, FLOAT_TYPE);
        // Exp -> INT
        else
            return newType(BASIC, 1, INT_TYPE);
    }
}

void Args(pNode node, pItem funcInfo) {
    assert(node != NULL);
    // Args -> Exp COMMA Args
    //       | Exp
    pNode temp = node;
    pFieldList arg = funcInfo->field->type->u.function.argv;

    while (temp) {
        if (arg == NULL) {
            char msg[100] = {0};
            sprintf(msg, "too many arguments to function \"%s\", except %d args.", funcInfo->field->name, funcInfo->field->type->u.function.argc);
            pError(FUNC_AGRC_MISMATCH, node->line, msg);
            break;
        }
        pType realType = Exp(temp->child);

        if (!checkType(realType, arg->type)) {
            char msg[100] = {0};
            sprintf(msg, "Function \"%s\" is not applicable for arguments.", funcInfo->field->name);
            pError(FUNC_AGRC_MISMATCH, node->line, msg);
            if (realType) 
                deleteType(realType);
            return;
        }
        if (realType) 
            deleteType(realType);

        arg = arg->tail;
        if (temp->child->sibling)
            temp = temp->child->sibling->sibling;
        else
            break;
    }
    if (arg != NULL) {
        char msg[100] = {0};
        sprintf(msg, "too few arguments to function \"%s\", except %d args.", funcInfo->field->name, funcInfo->field->type->u.function.argc);
        pError(FUNC_AGRC_MISMATCH, node->line, msg);
    }
}
