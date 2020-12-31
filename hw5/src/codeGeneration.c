#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
FILE *g_output;

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
int generateExpr(AST_NODE *exprNode);
void generateWrite(AST_NODE *node);
void generateRead(AST_NODE *node);
void generateIntBinaryOp(AST_NODE *exprNode);
void generateFloatBinaryOp(AST_NODE *exprNode);


void generateProgram(AST_NODE *root)
{
}


void codeGeneration(AST_NODE *root)
{
    g_output = fopen("output.s", "w");
    generateProgram(root);
    fclose(g_output);
}
