#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
FILE *g_output;

void generateAlignment();
void generateHead(char *name);
int generatePrologue(char *name);
void generateEpilogue(char *name, int frameSize);

void generateProgram(AST_NODE *root);
void generateGlobalVarDecl(AST_NODE *varDeclListNode);
void generateFunctionDecl(AST_NODE *funcDeclNode);
int generateLocalVarDecl(AST_NODE *varDeclListNode);
int generateStmtList(AST_NODE *stmtListNode);
void generateStmt(AST_NODE *stmtNode);
void generateBlock(AST_NODE *blockNode);
void generateWhileStmt(AST_NODE *whileNode);
void generateForStmt(AST_NODE *forNode);
void generateAssignStmt(AST_NODE *assignNode);
void generateIfStmt(AST_NODE *ifNode);
void generateFunctionCall(AST_NODE *funcNode);
void generateReturnStmt(AST_NODE *returnNode);
int generateExpr(AST_NODE *exprNode);
void generateWrite(AST_NODE *node);
void generateRead(AST_NODE *node);
void generateIntBinaryOp(AST_NODE *exprNode);
void generateFloatBinaryOp(AST_NODE *exprNode);


void generateAlignment()
{
    fprintf(g_output, ".align 3\n");
}


void generateHead(char *name)
{
    fprintf(g_output, ".text\n");
    fprintf(g_output, "_start_%s:\n", name);
}


int generatePrologue(char *name)
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
    return offset;
}


void generateEpilogue(char *name, int frameSize)
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
    fprintf(g_output, "_frameSize_%s: .word %d\n", name, frameSize);
}


int generateLocalVarDecl(AST_NODE *varDeclListNode)
{
    return 0;
}


int generateStmtList(AST_NODE *stmtListNode)
{
    return 0;
}


void generateFunctionDecl(AST_NODE *funcDeclNode)
{
    AST_NODE* typeNode = funcDeclNode->child;
    AST_NODE* idNode = typeNode->rightSibling;
    AST_NODE* paramListNode = idNode->rightSibling;
    AST_NODE* blockNode = paramListNode->rightSibling;
    char *name = idNode->semantic_value.identifierSemanticValue.identifierName;
    generateHead(name);
    int frameSize = generatePrologue(name);
    for (AST_NODE *i = blockNode->child; i; i = i->rightSibling)
        if (i->nodeType == VARIABLE_DECL_LIST_NODE)
            frameSize += generateLocalVarDecl(i);
        else if (i->nodeType == STMT_LIST_NODE)
            frameSize += generateStmtList(i);
    generateEpilogue(name, frameSize);
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
