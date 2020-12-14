#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

/* djb2 */
int HASH(char* str)
{
    size_t idx = 5381;
    while (*str)
        idx = idx * 33 + *str++;
    return idx & (HASH_TABLE_SIZE - 1);
}

SymbolTable symbolTable;

SymbolTableEntry* newSymbolTableEntry(int nestingLevel)
{
    SymbolTableEntry* symbolTableEntry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    symbolTableEntry->nextInHashChain = NULL;
    symbolTableEntry->prevInHashChain = NULL;
    symbolTableEntry->nextInSameLevel = NULL;
    symbolTableEntry->sameNameInOuterLevel = NULL;
    symbolTableEntry->attribute = NULL;
    symbolTableEntry->name = NULL;
    symbolTableEntry->nestingLevel = nestingLevel;
    return symbolTableEntry;
}

void removeFromHashTrain(int hashIndex, SymbolTableEntry* entry)
{
}

void enterIntoHashTrain(int hashIndex, SymbolTableEntry* entry)
{
}

void initializeSymbolTable()
{
    memset(&symbolTable, 0, sizeof(symbolTable));
    symbolTable.scopeDisplay = (SymbolTableEntry**)malloc(sizeof(SymbolTableEntry*));
    symbolTable.scopeDisplay[0] = NULL;
    symbolTable.scopeDisplayCapacity = 1;
    enterSymbol("int", createAttrType(TYPE_ATTRIBUTE, SCALAR_TYPE_DESCRIPTOR, INT_TYPE));
    enterSymbol("float", createAttrType(TYPE_ATTRIBUTE, SCALAR_TYPE_DESCRIPTOR, FLOAT_TYPE));
    enterSymbol("void", createAttrType(TYPE_ATTRIBUTE, SCALAR_TYPE_DESCRIPTOR, VOID_TYPE));
    enterSymbol("read", createAttrFunc(FUNCTION_SIGNATURE, INT_TYPE));
    enterSymbol("fread", createAttrFunc(FUNCTION_SIGNATURE, FLOAT_TYPE));
    enterSymbol("write", createAttrFunc(FUNCTION_SIGNATURE, VOID_TYPE));
}

void symbolTableEnd()
{
    while (symbolTable.currentLevel >= 0)
        closeScope();
    free(symbolTable.scopeDisplay);
}

SymbolTableEntry* retrieveSymbol(char* symbolName)
{
    int idx = HASH(symbolName);
    SymbolTableEntry* ptr = symbolTable.hashTable[idx];
    while (ptr && strcmp(symbolName, ptr->name))
        ptr = ptr->nextInHashChain;
    return ptr;
}

SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute)
{
    int idx = HASH(symbolName);
    SymbolTableEntry* old = retrieveSymbol(symbolName);
    SymbolTableEntry* new = newSymbolTableEntry(symbolTable.currentLevel);
    new->name = strdup(symbolName);
    new->attribute = attribute;
    if (old == NULL) {
        new->nextInHashChain = symbolTable.hashTable[idx];
        if (new->nextInHashChain)
            new->nextInHashChain->prevInHashChain = new;
        symbolTable.hashTable[idx] = new;
        new->nextInSameLevel = symbolTable.scopeDisplay[symbolTable.currentLevel];
        symbolTable.scopeDisplay[symbolTable.currentLevel] = new;
    } else {
        new->sameNameInOuterLevel = old;
        new->prevInHashChain = old->prevInHashChain;
        new->nextInHashChain = old->nextInHashChain;
        if (new->prevInHashChain)
            new->prevInHashChain->nextInHashChain = new;
        else
            symbolTable.hashTable[idx] = new;
        if (new->nextInHashChain)
            new->nextInHashChain->prevInHashChain = new;
    }
    return new;
}

//remove the symbol from the current scope
void removeSymbolEntry(SymbolTableEntry* ptr)
{
    int idx = HASH(ptr->name);
    SymbolTableEntry* old = ptr->sameNameInOuterLevel;
    SymbolTableEntry* prev = ptr->prevInHashChain;
    SymbolTableEntry* next = ptr->nextInHashChain;
    free(ptr->name);
    deleteAttr(ptr->attribute);
    free(ptr);
    if (old) {
        old->prevInHashChain = prev;
        old->nextInHashChain = next;
        if (prev)
            prev->nextInHashChain = old;
        else
            symbolTable.hashTable[idx] = old;
        if (next)
            next->prevInHashChain = old;
    } else {
        if (prev)
            prev->nextInHashChain = next;
        else
            symbolTable.hashTable[idx] = next;
        if (next)
            next->prevInHashChain = prev;
    }
}

int declaredLocally(char* symbolName)
{
}

void openScope()
{
    symbolTable.currentLevel++;
    if (symbolTable.currentLevel + 1 > symbolTable.scopeDisplayCapacity)
        symbolTable.scopeDisplay = realloc(symbolTable.scopeDisplay, (symbolTable.scopeDisplayCapacity *= 2) * sizeof(SymbolTableEntry*));
    symbolTable.scopeDisplay[symbolTable.currentLevel] = NULL;
}

void closeScope()
{
    SymbolTableEntry* ptr = symbolTable.scopeDisplay[symbolTable.currentLevel--];
    SymbolTableEntry* next;
    for (; ptr; ptr = next) {
        next = ptr->nextInSameLevel;
        removeSymbolEntry(ptr);
    }
}


SymbolAttribute* createAttrType(SymbolAttributeKind attrKind, TypeDescriptorKind tdKind, DATA_TYPE type)
{
    SymbolAttribute* Attr = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
    Attr->attributeKind = attrKind;
    TypeDescriptor* TD = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
    TD->kind = tdKind;
    if (tdKind == SCALAR_TYPE_DESCRIPTOR)
        TD->properties.dataType = type;
    else if (tdKind == ARRAY_TYPE_DESCRIPTOR)
        TD->properties.arrayProperties.elementType = type;
    Attr->attr.typeDescriptor = TD;
    return Attr;
}

SymbolAttribute* createAttrFunc(SymbolAttributeKind attrKind, DATA_TYPE type)
{
    SymbolAttribute* Attr = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
    Attr->attributeKind = attrKind;
    Attr->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    Attr->attr.functionSignature->returnType = type;
    Attr->attr.functionSignature->parameterList = NULL;
    Attr->attr.functionSignature->parametersCount = 0;
    return Attr;
}

void deleteAttr(SymbolAttribute* attr)
{
    Parameter* ptr;
    Parameter* next;
    switch (attr->attributeKind) {
    case VARIABLE_ATTRIBUTE:
    case TYPE_ATTRIBUTE:
        free(attr->attr.typeDescriptor);
        break;
    case FUNCTION_SIGNATURE:
        ptr = attr->attr.functionSignature->parameterList;
        for (; ptr; ptr = next) {
            next = ptr->next;
            free(ptr->parameterName);
            free(ptr);
        }
        free(attr->attr.functionSignature);
        break;
    }
    free(attr);
}
