#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"
FILE *g_output;
int g_offset;
int g_max_offset;
int g_cnt;
int g_regx[32];
int g_regf[32];

typedef struct Reg
{
    DATA_TYPE type; // int of float
    int i;
} Reg;

void exitError(char *msg);
Reg getIntReg();
Reg getFloatReg();
void freeReg(Reg reg);
int push(int size);
void pop(int size);
Reg floatToBool(Reg reg);
void generateAlignment();
void generateDwordData(Reg reg, size_t value);
void generateIntData(Reg reg, int value);
void generateFloatData(Reg reg, float value);
void generateStrData(Reg reg, char *value);
void generateHead(char *name);
void generatePrologue(char *name);
void generateEpilogue(char *name);

void generateProgram(AST_NODE *root);
void generateGlobalVarDecl(AST_NODE *varDeclListNode);
void generateFunctionDecl(AST_NODE *funcDeclNode);
void generateLocalVarDecl(AST_NODE *varDeclListNode);
void generateStmtList(AST_NODE *stmtListNode);
void generateStmt(AST_NODE *stmtNode);
void generateBlock(AST_NODE *blockNode);
void generateWhileStmt(AST_NODE *whileNode);
void generateForStmt(AST_NODE *forNode);
void generateAssignStmt(AST_NODE *assignNode);
void generateIfStmt(AST_NODE *ifNode);
void generateFunctionCall(AST_NODE *funcNode);
void generateReturnStmt(AST_NODE *returnNode);
Reg generateExprTest(AST_NODE *exprNode);
Reg generateExprGeneral(AST_NODE *exprNode);
Reg generateExpr(AST_NODE *exprNode);
void generateWrite(AST_NODE *node);
void generateIntBinaryOp(AST_NODE *exprNode);
void generateFloatBinaryOp(AST_NODE *exprNode);


void exitError(char *msg)
{
    printf("%s\n", msg);
    exit(1);
}


Reg getIntReg()
{
    Reg ret;
    ret.type = INT_TYPE;
    static int tid[7] = {5,6,7,28,29,30,31}; // t0~t6
    for (int i = 0; i <= 6; ++i)
        if (g_regx[tid[i]] == 0) {
            g_regx[tid[i]] = 1;
            ret.i = tid[i];
            return ret;
        }
    for (int i = 18; i <= 27; ++i) //s2~s11
        if (g_regx[i] == 0) {
            g_regx[i] = 1;
            ret.i = i;
            return ret;
        }
    exitError("out of integer registers");
}

Reg getFloatReg()
{
    Reg ret;
    ret.type = FLOAT_TYPE;
    for (int i = 0; i <= 7; ++i) //ft0~ft7
        if (g_regf[i] == 0) {
            g_regf[i] = 1;
            ret.i = i;
            return ret;
        }
    exitError("out of float registers");
}


void freeReg(Reg reg)
{
    if (reg.type == INT_TYPE)
        g_regx[reg.i] = 0;
    else
        g_regf[reg.i] = 0;
}


int push(int size)
{
    int ret = g_offset;
    g_offset += size;
    if (g_offset > g_max_offset)
        g_max_offset = g_offset;
    return ret;
}


void pop(int size)
{
    g_offset -= size;
}


Reg floatToBool(Reg reg)
{
    Reg fz = getFloatReg();
    Reg b = getIntReg();
    fprintf(g_output, "fmv.w.x f%d,x0\n", fz.i);
    fprintf(g_output, "feq.s x%d,f%d,f%d\n", b.i, reg.i, fz.i);
    fprintf(g_output, "xori x%d,x%d,1\n", b.i, b.i);
    freeReg(fz);
    freeReg(reg);
    return b;
}


void generateAlignment()
{
    fprintf(g_output, ".align 3\n");
}


void generateDwordData(Reg reg, size_t value)
{
    assert(reg.type == INT_TYPE);
    fprintf(g_output, ".data\n");
    fprintf(g_output, "_const_%d: .dword %zu\n", g_cnt, value);
    generateAlignment();
    fprintf(g_output, ".text\n");
    fprintf(g_output, "ld x%d,_const_%d\n", reg.i, g_cnt++);
}


void generateIntData(Reg reg, int value)
{
    assert(reg.type == INT_TYPE);
    fprintf(g_output, ".data\n");
    fprintf(g_output, "_int_const_%d: .word %d\n", g_cnt, value);
    generateAlignment();
    fprintf(g_output, ".text\n");
    fprintf(g_output, "lw x%d,_int_const_%d\n", reg.i, g_cnt++);
}


void generateFloatData(Reg reg, float value)
{
    assert(reg.type == FLOAT_TYPE);
    fprintf(g_output, ".data\n");
    fprintf(g_output, "_float_const_%d: .word %d\n", g_cnt, *(int *)&value);
    generateAlignment();
    fprintf(g_output, ".text\n");
    fprintf(g_output, "flw f%d,_float_const_%d\n", reg.i, g_cnt++);
}


void generateStrData(Reg reg, char *value)
{
    assert(reg.type == INT_TYPE);
    fprintf(g_output, ".data\n");
    fprintf(g_output, "_str_const_%d: .ascii \"%s\\000\"\n", g_cnt, value);
    generateAlignment();
    fprintf(g_output, ".text\n");
    fprintf(g_output, "la x%d,_str_const_%d\n", reg.i, g_cnt++);
}


void generateHead(char *name)
{
    fprintf(g_output, ".text\n");
    fprintf(g_output, "_start_%s:\n", name);
}


void generatePrologue(char *name)
{
    fprintf(g_output, "sd ra,0(sp)\n"); // save return address
    fprintf(g_output, "sd fp,-8(sp)\n"); // save old fp
    fprintf(g_output, "addi fp,sp,-8\n"); // new fp
    fprintf(g_output, "addi sp,sp,-16\n"); // new sp
    fprintf(g_output, "la ra,_frameSize_%s\n", name);
    fprintf(g_output, "lw ra,0(ra)\n");
    fprintf(g_output, "sub sp,sp,ra\n");
    int offset = 8;
    for (int i = 0; i <= 6; ++i, offset += 8)
        fprintf(g_output, "sd t%d,%d(sp)\n", i, offset);
    for (int i = 2; i <= 11; ++i, offset += 8)
        fprintf(g_output, "sd s%d,%d(sp)\n", i, offset);
    fprintf(g_output, "sd fp,%d(sp)\n", offset);
    offset += 8;
    for (int i = 0; i <= 7; ++i, offset += 4)
        fprintf(g_output, "fsw ft%d,%d(sp)\n", i, offset);
    g_max_offset = g_offset = offset;
}


void generateEpilogue(char *name)
{
    fprintf(g_output, "_end_%s:\n", name);
    int offset = 8;
    for (int i = 0; i <= 6; ++i, offset += 8)
        fprintf(g_output, "ld t%d,%d(sp)\n", i, offset);
    for (int i = 2; i <= 11; ++i, offset += 8)
        fprintf(g_output, "ld s%d,%d(sp)\n", i, offset);
    fprintf(g_output, "ld fp,%d(sp)\n", offset);
    offset += 8;
    for (int i = 0; i <= 7; ++i, offset += 4)
        fprintf(g_output, "flw ft%d,%d(sp)\n", i, offset);
    fprintf(g_output, "ld ra,8(fp)\n");
    fprintf(g_output, "addi sp,fp,8\n");
    fprintf(g_output, "ld fp,0(fp)\n");
    fprintf(g_output, "jr ra\n");
    fprintf(g_output, ".data\n");
    fprintf(g_output, "_frameSize_%s: .word %d\n", name, g_max_offset);
}


void generateLocalVarDecl(AST_NODE *varDeclListNode)
{
    for (AST_NODE *declNode = varDeclListNode->child; declNode; declNode = declNode->rightSibling) 
        if (declNode->semantic_value.declSemanticValue.kind == VARIABLE_DECL)
            for (AST_NODE * idNode = declNode->child->rightSibling; idNode; idNode = idNode->rightSibling) {
                SymbolTableEntry *entry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
                TypeDescriptor *TD = entry->attribute->attr.typeDescriptor;
                int size = 4;
                if (TD->kind == ARRAY_TYPE_DESCRIPTOR)
                    for (int i = 0; i < TD->properties.arrayProperties.dimension; ++i)
                        size *= TD->properties.arrayProperties.sizeInEachDimension[i];
                entry->offset = push(size);
            }
}


void generateBlock(AST_NODE *blockNode)
{
    int old_offset = g_offset;
    for (AST_NODE *i = blockNode->child; i; i = i->rightSibling)
        if (i->nodeType == VARIABLE_DECL_LIST_NODE)
            generateLocalVarDecl(i);
        else if (i->nodeType == STMT_LIST_NODE)
            generateStmtList(i);
    g_offset = old_offset;
}


Reg generateExprGeneral(AST_NODE *exprNode)
{
    Reg reg, reg2;
    SymbolTableEntry *entry;
    ArrayProperties AP;
    int i, old_i;
    size_t size;
    switch (exprNode->nodeType) {
        case EXPR_NODE:
            return generateExpr(exprNode);
        case STMT_NODE: // function call
            generateFunctionCall(exprNode);
            if (exprNode->dataType == INT_TYPE) {
                reg = getIntReg();
                fprintf(g_output, "mv x%d,a0\n", reg.i);
            } else {
                reg = getFloatReg();
                fprintf(g_output, "fmv.s f%d,a0\n", reg.i);
            }
            break;
        case IDENTIFIER_NODE:
            entry = exprNode->semantic_value.identifierSemanticValue.symbolTableEntry;
            if (exprNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID) {
                exprNode->dataType = entry->attribute->attr.typeDescriptor->properties.dataType;
                reg = exprNode->dataType == INT_TYPE ? getIntReg() : getFloatReg();
                if (entry->nestingLevel == 0) { // global variable
                    if (reg.type == INT_TYPE)
                        fprintf(g_output, "lw x%d,_g_%s\n", reg.i, exprNode->semantic_value.identifierSemanticValue.identifierName);
                    else
                        fprintf(g_output, "flw f%d,_g_%s\n", reg.i, exprNode->semantic_value.identifierSemanticValue.identifierName);
                } else {
                    if (reg.type == INT_TYPE)
                        fprintf(g_output, "lw x%d,%d(sp)\n", reg.i, entry->offset);
                    else
                        fprintf(g_output, "flw f%d,%d(sp)\n", reg.i, entry->offset);
                }
            } else { // array access
                AP = entry->attribute->attr.typeDescriptor->properties.arrayProperties;
                exprNode->dataType = AP.elementType;
                i = 0;
                reg = getIntReg();
                fprintf(g_output, "add x%d,x0,x0\n", reg.i);
                for (AST_NODE *dimListNode = exprNode->child; dimListNode; dimListNode = dimListNode->rightSibling, ++i) {
                    if (i) {
                        reg2 = getIntReg();
                        generateDwordData(reg2, AP.sizeInEachDimension[i]);
                        fprintf(g_output, "mul x%d,x%d,x%d\n", reg.i, reg.i, reg2.i);
                        freeReg(reg2);
                    }
                    reg2 = generateExprGeneral(dimListNode);
                    fprintf(g_output, "add x%d,x%d,x%d\n", reg.i, reg.i, reg2.i);
                    freeReg(reg2);
                }
                old_i = i;
                size = 4;
                for (; i < AP.dimension; ++i)
                    if (i)
                        size *= AP.sizeInEachDimension[i];
                if (old_i != i) 
                    exprNode->dataType = exprNode->dataType == INT_TYPE ? INT_PTR_TYPE : FLOAT_PTR_TYPE;
                reg2 = getIntReg();
                generateDwordData(reg2, size);
                fprintf(g_output, "mul x%d,x%d,x%d\n", reg.i, reg.i, reg2.i);
                if (entry->nestingLevel == 0)
                    fprintf(g_output, "la x%d,_g_%s\n", reg2.i, exprNode->semantic_value.identifierSemanticValue.identifierName);
                else
                    fprintf(g_output, "addi x%d,sp,%d\n", reg2.i, entry->offset);
                fprintf(g_output, "add x%d,x%d,x%d\n", reg.i, reg.i, reg2.i);
                freeReg(reg2);
                // now reg store the address
                if (exprNode->dataType == INT_TYPE) {
                    fprintf(g_output, "lw x%d,x%d\n", reg.i, reg.i);
                } else if (exprNode->dataType == FLOAT_TYPE) {
                    reg2 = getFloatReg();
                    fprintf(g_output, "flw f%d,x%d\n", reg2.i, reg.i);
                    freeReg(reg);
                    reg = reg2;
                } else { // pointer
                }
            }
            break;
        case CONST_VALUE_NODE:
            switch (exprNode->semantic_value.const1->const_type) {
                case INTEGERC:
                    exprNode->dataType = INT_TYPE;
                    reg = getIntReg();
                    generateIntData(reg, exprNode->semantic_value.const1->const_u.intval);
                    break;
                case FLOATC:
                    exprNode->dataType = FLOAT_TYPE;
                    reg = getFloatReg();
                    generateFloatData(reg, exprNode->semantic_value.const1->const_u.fval);
                    break;
                case STRINGC:
                    exprNode->dataType = CONST_STRING_TYPE;
                    reg = getIntReg();
                    generateStrData(reg, exprNode->semantic_value.const1->const_u.sc);
                    break;
            }
            break;
    }
    return reg;
}


Reg generateExpr(AST_NODE *exprNode)
{
    Reg reg, reg1, reg2;
    if (exprNode->semantic_value.exprSemanticValue.isConstEval) {
        if (exprNode->dataType == INT_TYPE)
            generateIntData(reg = getIntReg(), exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
        else
            generateFloatData(reg = getFloatReg(), exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
        return reg;
    }
    if (exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
        AST_NODE *left = exprNode->child;
        AST_NODE *right = left->rightSibling;
        exprNode->dataType = (left->dataType == FLOAT_TYPE || right->dataType == FLOAT_TYPE) ? FLOAT_TYPE : INT_TYPE;
        Reg reg = generateExprGeneral(left);
        int offset = push(4);
        fprintf(g_output, "sw %c%d,%d(sp)\n", reg.type == INT_TYPE ? 'x' : 'f', reg.i, offset);
        freeReg(reg);
        reg2 = generateExprGeneral(right);
        if (left->dataType == INT_TYPE) {
            reg1 = getIntReg();
            fprintf(g_output, "lw x%d,%d(sp)\n", reg.i, offset);
        } else {
            reg1 = getFloatReg();
            fprintf(g_output, "flw f%d,%d(sp)\n", reg.i, offset);
        }
        pop(4);
        if (left->dataType == INT_TYPE && right->dataType == INT_TYPE) {
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    fprintf(g_output, "add x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_SUB:
                    fprintf(g_output, "sub x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_MUL:
                    fprintf(g_output, "mul x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_DIV:
                    fprintf(g_output, "div x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_EQ:
                    fprintf(g_output, "sub x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    fprintf(g_output, "seqz x%d,x%d\n", reg1.i, reg1.i);
                    break;
                case BINARY_OP_GE:
                    fprintf(g_output, "slt x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    fprintf(g_output, "xori x%d,x%d,1\n", reg1.i, reg1.i);
                    break;
                case BINARY_OP_LE:
                    fprintf(g_output, "slt x%d,x%d,x%d\n", reg1.i, reg2.i, reg1.i);
                    fprintf(g_output, "xori x%d,x%d,1\n", reg1.i, reg1.i);
                    break;
                case BINARY_OP_NE:
                    fprintf(g_output, "sub x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    fprintf(g_output, "snez x%d,x%d\n", reg1.i, reg1.i);
                    break;
                case BINARY_OP_GT:
                    fprintf(g_output, "sgt x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_LT:
                    fprintf(g_output, "slt x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_AND: //TODO:short circuit
                    fprintf(g_output, "snez x%d,x%d\n", reg1.i, reg1.i);
                    fprintf(g_output, "snez x%d,x%d\n", reg2.i, reg2.i);
                    fprintf(g_output, "and x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_OR://TODO:short circuit
                    fprintf(g_output, "snez x%d,x%d\n", reg1.i, reg1.i);
                    fprintf(g_output, "snez x%d,x%d\n", reg2.i, reg2.i);
                    fprintf(g_output, "or x%d,x%d,x%d\n", reg1.i, reg1.i, reg2.i);
                    break;
            }
            freeReg(reg2);
            return reg1;
        } else {//float
            if (reg1.type == INT_TYPE) {
                reg = getFloatReg();
                fprintf(g_output, "fcvt.s.w f%d,x%d\n", reg.i, reg1.i); // int to float
                freeReg(reg1);
                reg1 = reg;
            }
            if (reg2.type == INT_TYPE) {
                reg = getFloatReg();
                fprintf(g_output, "fcvt.s.w f%d,x%d\n", reg.i, reg2.i); // int to float
                freeReg(reg2);
                reg2 = reg;
            }
            int arithmetic = 1;
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    fprintf(g_output, "fadd.s f%d,f%d,f%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_SUB:
                    fprintf(g_output, "fsub.s f%d,f%d,f%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_MUL:
                    fprintf(g_output, "fmul.s f%d,f%d,f%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_DIV:
                    fprintf(g_output, "fdiv.s f%d,f%d,f%d\n", reg1.i, reg1.i, reg2.i);
                    break;
                default:
                    arithmetic = 0;
                    break;
            }
            if (arithmetic) {
                freeReg(reg2);
                return reg1;
            }
            reg = getIntReg(); // for compare
            exprNode->dataType = INT_TYPE;
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_EQ:
                    fprintf(g_output, "feq.s x%d,f%d,f%d\n", reg.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_GE:
                    fprintf(g_output, "fle.s x%d,f%d,f%d\n", reg.i, reg2.i, reg1.i);
                    break;
                case BINARY_OP_LE:
                    fprintf(g_output, "fle.s x%d,f%d,f%d\n", reg.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_NE:
                    fprintf(g_output, "feq.s x%d,f%d,f%d\n", reg.i, reg1.i, reg2.i);
                    fprintf(g_output, "xori x%d,x%d,1\n", reg.i, reg.i);
                    break;
                case BINARY_OP_GT:
                    fprintf(g_output, "flt.s x%d,f%d,f%d\n", reg.i, reg2.i, reg1.i);
                    break;
                case BINARY_OP_LT:
                    fprintf(g_output, "flt.s x%d,f%d,f%d\n", reg.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_AND: //TODO:short circuit
                    reg1 = floatToBool(reg1);
                    reg2 = floatToBool(reg2);
                    fprintf(g_output, "and x%d,x%d,x%d\n", reg.i, reg1.i, reg2.i);
                    break;
                case BINARY_OP_OR://TODO:short circuit
                    reg1 = floatToBool(reg1);
                    reg2 = floatToBool(reg2);
                    fprintf(g_output, "or x%d,x%d,x%d\n", reg.i, reg1.i, reg2.i);
                    break;
            }
            freeReg(reg1);
            freeReg(reg2);
            return reg;
        }
    }
    // Unary
    AST_NODE *operand = exprNode->child;
    exprNode->dataType = operand->dataType;
    reg = generateExprGeneral(operand);
    if (reg.type == INT_TYPE) {
        switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
        case UNARY_OP_POSITIVE:
            break;
        case UNARY_OP_NEGATIVE:
            fprintf(g_output, "neg x%d,x%d\n", reg.i, reg.i);
            break;
        case UNARY_OP_LOGICAL_NEGATION:
            fprintf(g_output, "seqz x%d,x%d\n", reg.i, reg.i);
            break;
        }
    } else {
        switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
        case UNARY_OP_POSITIVE:
            break;
        case UNARY_OP_NEGATIVE:
            fprintf(g_output, "fneg.s f%d,f%d\n", reg.i, reg.i);
            break;
        case UNARY_OP_LOGICAL_NEGATION:
            exprNode->dataType = INT_TYPE;
            reg = floatToBool(reg);
            fprintf(g_output, "seqz x%d,x%d\n", reg.i, reg.i);
            break;
        }
    }
    return reg;
}


Reg generateExprTest(AST_NODE *exprNode)
{
    Reg reg = generateExprGeneral(exprNode);
    if (reg.type == FLOAT_TYPE)
        reg = floatToBool(reg);
    return reg;
}


void generateWhileStmt(AST_NODE *whileNode)
{
    int cnt = g_cnt++;
    AST_NODE *test = whileNode->child;
    AST_NODE *stmt = test->rightSibling;
    fprintf(g_output, "_while_%d:\n", cnt);
    Reg reg = generateExprTest(test);
    freeReg(reg);
    fprintf(g_output, "beqz x%d,_end_while_%d\n", reg.i, cnt);
    generateStmt(stmt);
    fprintf(g_output, "j _while_%d\n",  cnt);
    fprintf(g_output, "_end_while_%d:\n",  cnt);
}


void generateForStmt(AST_NODE *forNode)//TODO
{
    return;
}


void generateAssignStmt(AST_NODE *assignNode)
{
}


void generateIfStmt(AST_NODE *ifNode);


void generateWrite(AST_NODE *node)
{
    AST_NODE *paramNode = node->rightSibling->child;
    Reg reg = generateExprGeneral(paramNode);
    if (paramNode->dataType == INT_TYPE) {
        fprintf(g_output, "mv a0,x%d\n", reg.i);
        fprintf(g_output, "jal _write_int\n");
    } else if (paramNode->dataType == FLOAT_TYPE) {
        fprintf(g_output, "mv a0,f%d\n", reg.i);
        fprintf(g_output, "jal _write_float\n");
    } else if (paramNode->dataType == CONST_STRING_TYPE) {
        fprintf(g_output, "mv a0,x%d\n", reg.i);
        fprintf(g_output, "jal _write_str\n");
    }
    freeReg(reg);
}


void generateFunctionCall(AST_NODE *funcNode)//Parameterless procedure calls only
{
    AST_NODE *idNode = funcNode->child;
    char *name = idNode->semantic_value.identifierSemanticValue.identifierName;
    if (strcmp(name, "write") == 0) {
        generateWrite(idNode);
        return;
    }
    if (strcmp(name, "read") == 0)
        fprintf(g_output, "jal _read_int\n");
    else if (strcmp(name, "fread") == 0)
        fprintf(g_output, "jal _read_float\n");
    else
        fprintf(g_output, "jal _start_%s\n", name);
    funcNode->dataType = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.functionSignature->returnType;
}


void generateReturnStmt(AST_NODE *returnNode);


void generateStmt(AST_NODE *stmtNode)
{
    switch (stmtNode->nodeType) {
        case NUL_NODE:
            return;
        case BLOCK_NODE:
            generateBlock(stmtNode);
            return;
        case STMT_NODE:
            switch (stmtNode->semantic_value.stmtSemanticValue.kind) {
                case WHILE_STMT:
                    generateWhileStmt(stmtNode);
                    break;
                case FOR_STMT:
                    generateForStmt(stmtNode);
                    break;
                case ASSIGN_STMT:
                    generateAssignStmt(stmtNode);
                    break;
                case IF_STMT:
                    generateIfStmt(stmtNode);
                    break;
                case FUNCTION_CALL_STMT:
                    generateFunctionCall(stmtNode);
                    break;
                case RETURN_STMT:
                    generateReturnStmt(stmtNode);
                    break;
            }
            return;
        default:
            exitError("generateStmt(): stmtNode is not stmt");
    }
}


void generateStmtList(AST_NODE *stmtListNode)
{
    for (AST_NODE *stmtNode = stmtListNode->child; stmtNode; stmtNode = stmtNode->rightSibling)
        generateStmt(stmtNode);
}


void generateFunctionDecl(AST_NODE *funcDeclNode)
{
    AST_NODE* typeNode = funcDeclNode->child;
    AST_NODE* idNode = typeNode->rightSibling;
    AST_NODE* paramListNode = idNode->rightSibling;
    AST_NODE* blockNode = paramListNode->rightSibling;
    char *name = idNode->semantic_value.identifierSemanticValue.identifierName;
    generateHead(name);
    generatePrologue(name);
    generateBlock(blockNode);
    generateEpilogue(name);
}


void generateGlobalVarDecl(AST_NODE *varDeclListNode)
{
    char name[999];
    fprintf(g_output, ".data\n");
    for (AST_NODE *declNode = varDeclListNode->child; declNode; declNode = declNode->rightSibling) 
        if (declNode->semantic_value.declSemanticValue.kind == VARIABLE_DECL)
            for (AST_NODE * idNode = declNode->child->rightSibling; idNode; idNode = idNode->rightSibling) {
                sprintf(name, "_g_%s", idNode->semantic_value.identifierSemanticValue.identifierName);
                TypeDescriptor *TD = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                DATA_TYPE type = TD->properties.dataType;
                if (TD->kind == SCALAR_TYPE_DESCRIPTOR) {
                    if (type == INT_TYPE) {
                        int value = idNode->child ? idNode->child->semantic_value.const1->const_u.intval : 0;
                        fprintf(g_output, "%s: .word %d\n", name, value);
                    } else if (type == FLOAT_TYPE) {
                        float value = idNode->child ? idNode->child->semantic_value.const1->const_u.fval : 0;
                        fprintf(g_output, "%s: .word %d\n", name, *(int *)&value);
                    } else {
                        exitError("global variable not int or float");
                    }
                } else { // array
                    int size = 4;
                    for (int i = 0; i < TD->properties.arrayProperties.dimension; ++i)
                        size *= TD->properties.arrayProperties.sizeInEachDimension[i];
                    fprintf(g_output, "%s: .zero %d\n", name, size);
                }
            }
    generateAlignment();
}


void generateProgram(AST_NODE *root)
{
    for (AST_NODE *i = root->child; i; i = i->rightSibling)
        if (i->nodeType == VARIABLE_DECL_LIST_NODE)
            generateGlobalVarDecl(i);
        else
            generateFunctionDecl(i);
}


void codeGeneration(AST_NODE *root)
{
    g_output = fopen("output.s", "w");
    generateProgram(root);
    fclose(g_output);
}
