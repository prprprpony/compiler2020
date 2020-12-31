#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
FILE *g_output;
int g_offset;
int g_max_offset;
int g_cnt;
int g_regx[32];
int g_regf[32];

void exitError(char *msg);
int getIntReg();
int getFloatReg();
void freeIntReg(int reg);
void freeFloatReg(int reg);
void generateAlignment();
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
int generateExprGeneral(AST_NODE *exprNode);
int generateExpr(AST_NODE *exprNode);
void generateWrite(AST_NODE *node);
void generateRead(AST_NODE *node);
void generateIntBinaryOp(AST_NODE *exprNode);
void generateFloatBinaryOp(AST_NODE *exprNode);


void exitError(char *msg)
{
    printf("%s\n", msg);
    exit(1);
}


int getIntReg()
{
    static int tid[7] = {5,6,7,28,29,30,31}; // t0~t6
    for (int i = 0; i <= 6; ++i)
        if (g_regx[tid[i]] == 0) {
            g_regx[tid[i]] = 1;
            return tid[i];
        }
    for (int i = 18; i <= 27; ++i) //s2~s11
        if (g_regx[i] == 0) {
            g_regx[i] = 1;
            return i;
        }
    exitError("out of integer registers");
}

int getFloatReg()
{
    for (int i = 0; i <= 7; ++i) //ft0~ft7
        if (g_regf[i] == 0) {
            g_regf[i] = 1;
            return i;
        }
    exitError("out of float registers");
}


void freeIntReg(int reg)
{
    g_regx[reg] = 0;
}


void freeFloatReg(int reg)
{
    g_regf[reg] = 0;
}


void generateAlignment()
{
    fprintf(g_output, ".align 3\n");
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
                entry->offset = g_offset;
                g_offset += size;
                if (g_offset > g_max_offset)
                    g_max_offset = g_offset;
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


void generateWhileStmt(AST_NODE *whileNode)
{
    int cnt = g_cnt++;
    AST_NODE *test = whileNode->child;
    AST_NODE *stmt = test->rightSibling;
    fprintf(g_output, "_while_%d:", cnt);
    int reg = generateExprGeneral(test);
    if (test->dataType == FLOAT_TYPE) {
        freeFloatReg(reg);
        int tmp = getIntReg();
        fprintf(g_output, "fcvt.w.s f%d,x%d", reg, tmp); // float to int
        reg = tmp;
    }
    freeIntReg(reg);
    fprintf(g_output, "beqz x%d,_end_while_%d", reg, cnt);
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
void generateFunctionCall(AST_NODE *funcNode);
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
