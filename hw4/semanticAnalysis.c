#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int g_anyErrorOccur = 0;
extern SymbolTable symbolTable;

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void processProgramNode(AST_NODE *programNode);
void processDeclarationNode(AST_NODE* declarationNode);
void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize);
void declareFunction(AST_NODE* returnTypeNode);
void processDeclDimList(AST_NODE* variableDeclDimList, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize);
DATA_TYPE processTypeNode(AST_NODE* typeNode);
void processBlockNode(AST_NODE* blockNode);
void processStmtNode(AST_NODE* stmtNode);
void processGeneralNode(AST_NODE *node);
void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void checkWhileStmt(AST_NODE* whileNode);
void checkForStmt(AST_NODE* forNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void checkIfStmt(AST_NODE* ifNode);
void checkWriteFunction(AST_NODE* functionCallNode);
void checkFunctionCall(AST_NODE* functionCallNode);
void processExprRelatedNode(AST_NODE* exprRelatedNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter);
void checkReturnStmt(AST_NODE* returnNode);
void processExprNode(AST_NODE* exprNode);
void processVariableLValue(AST_NODE* idNode);
void processVariableRValue(AST_NODE* idNode);
void processConstValueNode(AST_NODE* constValueNode);
void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue);
void evaluateExprValue(AST_NODE* exprNode);

void dfs(AST_NODE* node);
void checkIdentifierNode(AST_NODE* identifierNode);
char* idName(AST_NODE* idNode);
char* dataTypeName(DATA_TYPE dataType);

typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_UNDECLARED,
    NOT_FUNCTION_NAME,
    TRY_TO_INIT_ARRAY,
    EXCESSIVE_ARRAY_DIM_DECLARATION,
    RETURN_ARRAY,
    VOID_VARIABLE,
    TYPEDEF_VOID_ARRAY,
    PARAMETER_TYPE_UNMATCH,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    RETURN_TYPE_UNMATCH,
    INCOMPATIBLE_ARRAY_DIMENSION,
    NOT_ASSIGNABLE,
    NOT_ARRAY,
    IS_TYPE_NOT_VARIABLE,
    IS_FUNCTION_NOT_VARIABLE,
    STRING_OPERATION,
    ARRAY_SIZE_NOT_INT,
    ARRAY_SIZE_NEGATIVE,
    ARRAY_SUBSCRIPT_NOT_INT,
    PASS_ARRAY_TO_SCALAR,
    PASS_SCALAR_TO_ARRAY
} ErrorMsgKind;

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    switch (errorMsgKind) {
    case SYMBOL_REDECLARE:
        printf("redeclaration of ‘%s %s’\n", name2, idName(node1));
        break;
    default:
        printf("Unhandled case in void printErrorMsgSpecial(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    switch (errorMsgKind) {
    case SYMBOL_IS_NOT_TYPE: //not required
        printf("%s is not type\n", idName(node));
        break;
    case SYMBOL_UNDECLARED:
        printf("‘%s’ was not declared in this scope\n", idName(node));
        break;
    case NOT_FUNCTION_NAME:
        break;
    case TRY_TO_INIT_ARRAY:
        break;
    case EXCESSIVE_ARRAY_DIM_DECLARATION: //not required
        printf("dimension of array ‘%s’ is over the MAX_ARRAY_DIMENSION=%d\n", idName(node), MAX_ARRAY_DIMENSION);
        break;
    case RETURN_ARRAY:
        break;
    case VOID_VARIABLE:
        break;
    case TYPEDEF_VOID_ARRAY:
        break;
    case PARAMETER_TYPE_UNMATCH:
        break;
    case TOO_FEW_ARGUMENTS:
        break;
    case TOO_MANY_ARGUMENTS:
        break;
    case RETURN_TYPE_UNMATCH:
        break;
    case INCOMPATIBLE_ARRAY_DIMENSION:
        break;
    case NOT_ASSIGNABLE:
        break;
    case NOT_ARRAY:
        break;
    case IS_TYPE_NOT_VARIABLE:
        break;
    case IS_FUNCTION_NOT_VARIABLE:
        break;
    case STRING_OPERATION:
        break;
    case ARRAY_SIZE_NOT_INT: //not required
        printf("size of array ‘%s’ is not integer\n", idName(node));
        break;
    case ARRAY_SIZE_NEGATIVE:
        printf("size of array ‘%s’ is negative\n", idName(node));
        break;
    case ARRAY_SUBSCRIPT_NOT_INT:
        break;
    case PASS_ARRAY_TO_SCALAR:
        break;
    case PASS_SCALAR_TO_ARRAY:
        break;
    default:
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}


void semanticAnalysis(AST_NODE* root)
{
    processProgramNode(root);
}


DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2)
{
    if(dataType1 == FLOAT_TYPE || dataType2 == FLOAT_TYPE) {
        return FLOAT_TYPE;
    } else {
        return INT_TYPE;
    }
}

void dfs(AST_NODE* node)
{
    for (AST_NODE* i = node->child; i; i = i->rightSibling) {
        switch (i->nodeType) {
        case PROGRAM_NODE: //?
            processProgramNode(i);
            break;
        case DECLARATION_NODE:
            processDeclarationNode(i);
            break;
        case IDENTIFIER_NODE:
            checkIdentifierNode(i);
            break;
        case PARAM_LIST_NODE: //?
            dfs(i);
            break;
        case NUL_NODE:
            break;
        case BLOCK_NODE:
            processBlockNode(i);
            break;
        case VARIABLE_DECL_LIST_NODE: //?
            dfs(i);
            break;
        case STMT_LIST_NODE: //?
            dfs(i);
            break;
        case STMT_NODE:
            processStmtNode(i);
            break;
        case EXPR_NODE:
            processExprNode(i);
            break;
        case CONST_VALUE_NODE:
            processConstValueNode(i);
            break;
        case NONEMPTY_ASSIGN_EXPR_LIST_NODE: //?
            dfs(i);
            break;
        case NONEMPTY_RELOP_EXPR_LIST_NODE: //?
            dfs(i);
            break;
        }
    }
}

void processProgramNode(AST_NODE* programNode)
{
    dfs(programNode);
}

void processDeclarationNode(AST_NODE* declarationNode)
{
    switch (declarationNode->semantic_value.declSemanticValue.kind) {
    case VARIABLE_DECL:
        declareIdList(declarationNode->child, VARIABLE_ATTRIBUTE, 0);
        break;
    case TYPE_DECL:
        declareIdList(declarationNode->child, TYPE_ATTRIBUTE, 0);
        break;
    case FUNCTION_DECL:
        declareFunction(declarationNode->child);
        break;
    case FUNCTION_PARAMETER_DECL:
        declareIdList(declarationNode->child, VARIABLE_ATTRIBUTE, 1);
        break;
    }
}


DATA_TYPE processTypeNode(AST_NODE* idNodeAsType)
{
    SymbolTableEntry* ptr = retrieveSymbol(idName(idNodeAsType));
    if (!ptr) {
        printErrorMsg(idNodeAsType, SYMBOL_UNDECLARED);
        return ERROR_TYPE;
    }
    if (ptr->attribute->attributeKind != TYPE_ATTRIBUTE) { //?
        printErrorMsg(idNodeAsType, SYMBOL_IS_NOT_TYPE);
        return ERROR_TYPE;
    }
    idNodeAsType->semantic_value.identifierSemanticValue.symbolTableEntry = ptr; //?
    return ptr->attribute->attr.typeDescriptor->properties.dataType;
}

char* idName(AST_NODE* idNode)
{
    return idNode->semantic_value.identifierSemanticValue.identifierName;
}

char* dataTypeName(DATA_TYPE dataType)
{
    switch (dataType) {
    case INT_TYPE:
        return "int";
    case FLOAT_TYPE:
        return "float";
    case VOID_TYPE:
        return "void";
    case INT_PTR_TYPE:
        return "int*";
    case FLOAT_PTR_TYPE:
        return "float*";
    case CONST_STRING_TYPE:
        return "const char*";
    case NONE_TYPE:
        return "NONE_TYPE";
    case ERROR_TYPE:
        return "ERROR_TYPE";
    default:
        return "not in DATA_TYPE";
    }
}

void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
    AST_NODE* idList = typeNode->rightSibling;
    DATA_TYPE type = processTypeNode(typeNode);
    SymbolAttribute* attr;
    for (; idList; idList = idList->rightSibling) {
        SymbolTableEntry* ptr = retrieveSymbol(idName(idList));
        if (ptr && ptr->nestingLevel == symbolTable.currentLevel) {
            if (ptr->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR)
                printErrorMsgSpecial(idList, dataTypeName(ptr->attribute->attr.typeDescriptor->properties.dataType), SYMBOL_REDECLARE);
            else
                printErrorMsgSpecial(idList, dataTypeName(ptr->attribute->attr.typeDescriptor->properties.arrayProperties.elementType), SYMBOL_REDECLARE);
            idList->semantic_value.identifierSemanticValue.symbolTableEntry = ptr;
            continue;
        }
        switch (idList->semantic_value.identifierSemanticValue.kind) {
            case ARRAY_ID:
                attr = createAttrType(isVariableOrTypeAttribute, ARRAY_TYPE_DESCRIPTOR, type);
                processDeclDimList(idList, attr->attr.typeDescriptor, ignoreArrayFirstDimSize);
                break;
            case NORMAL_ID:
                attr = createAttrType(isVariableOrTypeAttribute, SCALAR_TYPE_DESCRIPTOR, type);
                break;
            case WITH_INIT_ID: //?
                attr = createAttrType(isVariableOrTypeAttribute, SCALAR_TYPE_DESCRIPTOR, type);
                dfs(idList);
                break;
        }
        idList->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName(idList), attr);
    }
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
}

void checkWhileStmt(AST_NODE* whileNode)
{
}


void checkForStmt(AST_NODE* forNode)
{
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
}


void checkIfStmt(AST_NODE* ifNode)
{
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
}


void processExprRelatedNode(AST_NODE* exprRelatedNode)
{
}

void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
}

void evaluateExprValue(AST_NODE* exprNode)
{
}


void processExprNode(AST_NODE* exprNode)
{
}


void processVariableLValue(AST_NODE* idNode)
{
}

void processVariableRValue(AST_NODE* idNode)
{
}


void processConstValueNode(AST_NODE* constValueNode)
{
}


void checkReturnStmt(AST_NODE* returnNode)
{
}


void processBlockNode(AST_NODE* blockNode)
{
}


void processStmtNode(AST_NODE* stmtNode)
{
}


void processGeneralNode(AST_NODE *node)
{
}

void processDeclDimList(AST_NODE* idNode, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize)
{
    int dim = 0;
    for (AST_NODE* i = idNode->child; i; i = i->rightSibling, ++dim) {
        if (dim >= MAX_ARRAY_DIMENSION)
            continue;
        if (dim == 0 && ignoreFirstDimSize) {
            typeDescriptor->properties.arrayProperties.sizeInEachDimension[dim] = -1;
        } else if (i->nodeType != CONST_VALUE_NODE || i->semantic_value.const1->const_type != INTEGERC) {
            typeDescriptor->properties.arrayProperties.sizeInEachDimension[dim] = -1;
            printErrorMsg(idNode, ARRAY_SIZE_NOT_INT);
        } else {
            int size = i->semantic_value.const1->const_u.intval;
            if (size < 0) {
                printErrorMsg(idNode, ARRAY_SIZE_NEGATIVE);
                typeDescriptor->properties.arrayProperties.sizeInEachDimension[dim] = -1;
            } else {
                typeDescriptor->properties.arrayProperties.sizeInEachDimension[dim] = size;
            }
        }
    }
    typeDescriptor->properties.arrayProperties.dimension = dim;
    if (dim > MAX_ARRAY_DIMENSION)
        printErrorMsg(idNode, EXCESSIVE_ARRAY_DIM_DECLARATION);
}


void declareFunction(AST_NODE* declarationNode)
{
}
