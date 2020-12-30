#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int g_anyErrorOccur = 0;
int g_ignoreDim = 0;
DATA_TYPE g_currentReturnType;
int g_inAssignmentStmt = 0;
int g_LValue = 0;
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
char* arrayTypeName(TypeDescriptor* type);

typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_REDECLARE_TYPEDEF,
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
    case SYMBOL_REDECLARE_TYPEDEF:
        printf("conflicting types for ‘%s’\n", idName(node1));
        break;
    case INCOMPATIBLE_ARRAY_DIMENSION:
        if (strcmp(name2, ">") == 0)
            printf("subscripted value is neither array nor pointer nor vector\n");
        else if (strcmp(name2, "<L") == 0)
            printf("assignment to expression with array type\n");
        else if (strcmp(name2, "<") == 0) //not required?
            printf("incompatible (not enough) array dimension for array %s\n", idName(node1));
        break;
    case PASS_ARRAY_TO_SCALAR:
        assert(node1->semantic_value.identifierSemanticValue.symbolTableEntry);
        assert(node1->semantic_value.identifierSemanticValue.symbolTableEntry->attribute);
        assert(node1->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor);
        printf("invalid conversion from ‘%s’ to ‘%s’\n", arrayTypeName(node1->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor), name2);
        break;
    case PASS_SCALAR_TO_ARRAY:
        printf("invalid conversion from ‘%s’ to ‘%s’\n", dataTypeName(node1->dataType), name2);
        break;
    default:
        printf("Unhandled case in void printErrorMsgSpecial(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    if (errorMsgKind == RETURN_TYPE_UNMATCH) { //not required?
        printf("Warning: in line %d\n", node->linenumber);
        printf("return type unmatch\n"); 
        return;
    }
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    switch (errorMsgKind) {
    case SYMBOL_IS_NOT_TYPE: //not required
        printf("%s is not type\n", idName(node));
        break;
    case SYMBOL_UNDECLARED:
        printf("‘%s’ was not declared in this scope\n", idName(node));
        break;
    case SYMBOL_REDECLARE: // function redeclare, not required
        printf("redefinition of ‘%s’\n", idName(node));
        break;
    case NOT_FUNCTION_NAME:
        printf("called object ‘%s’ is not a function or function pointer\n", idName(node));
        break;
    case TRY_TO_INIT_ARRAY://not checked
        printf("try to init array %s\n", idName(node));
        break;
    case EXCESSIVE_ARRAY_DIM_DECLARATION: //not required
        printf("dimension of array ‘%s’ is over the MAX_ARRAY_DIMENSION=%d\n", idName(node), MAX_ARRAY_DIMENSION);
        break;
    case RETURN_ARRAY: //not required
        printf("return array\n"); 
        break;
    case VOID_VARIABLE://not checked
        printf("void variable %s\n", idName(node));
        break;
    case TYPEDEF_VOID_ARRAY://not checked
        printf("typedef void array %s\n", idName(node));
        break;
    case PARAMETER_TYPE_UNMATCH://not required
        printf("parameter type unmatch %s\n", idName(node));
        break;
    case TOO_FEW_ARGUMENTS:
        printf("too few arguments to function ‘%s’\n", idName(node));
        break;
    case TOO_MANY_ARGUMENTS:
        printf("too many arguments to function ‘%s’\n", idName(node));
        break;
    case NOT_ASSIGNABLE: //not required
        printf("%s not assignable\n", idName(node));
        break;
    case NOT_ARRAY: //not checked
        printf("%s not array\n", idName(node));
        break;
    case IS_TYPE_NOT_VARIABLE://not required
        printf("%s is type, not variable\n", idName(node));
        break;
    case IS_FUNCTION_NOT_VARIABLE://not checked
        printf("%s is function not variable\n", idName(node));
        break;
    case STRING_OPERATION://not checked
        printf("string operation");
        break;
    case ARRAY_SIZE_NOT_INT: //not required
        printf("size of array ‘%s’ is not integer\n", idName(node));
        break;
    case ARRAY_SIZE_NEGATIVE:
        printf("size of array ‘%s’ is negative\n", idName(node));
        break;
    case ARRAY_SUBSCRIPT_NOT_INT:
        printf("array subscript is not an integer\n");
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
        g_LValue = g_inAssignmentStmt ? i == node->child : 0;
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
                printErrorMsgSpecial(idList, dataTypeName(ptr->attribute->attr.typeDescriptor->properties.dataType), isVariableOrTypeAttribute == TYPE_ATTRIBUTE ? SYMBOL_REDECLARE_TYPEDEF : SYMBOL_REDECLARE);
            else
                printErrorMsgSpecial(idList, dataTypeName(ptr->attribute->attr.typeDescriptor->properties.arrayProperties.elementType), isVariableOrTypeAttribute == TYPE_ATTRIBUTE ? SYMBOL_REDECLARE_TYPEDEF : SYMBOL_REDECLARE);
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
    dfs(whileNode);
}


void checkForStmt(AST_NODE* forNode)
{
    dfs(forNode);
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
    g_inAssignmentStmt = 1;
    dfs(assignmentNode);
    g_inAssignmentStmt = 0;
    AST_NODE* left = assignmentNode->child;
    AST_NODE* right = left->rightSibling;
    processVariableLValue(left);
    processVariableRValue(right);
}


void checkIfStmt(AST_NODE* ifNode)
{
    dfs(ifNode);
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
    g_ignoreDim = 1;
    dfs(functionCallNode);
    g_ignoreDim = 0;
    AST_NODE* idNode = functionCallNode->child;
    AST_NODE* paramListNode = idNode->rightSibling;
    SymbolTableEntry* ptr = retrieveSymbol(idName(idNode));
    if (!ptr)
        return;
    if (ptr->attribute->attributeKind != FUNCTION_SIGNATURE) {
        printErrorMsg(idNode, NOT_FUNCTION_NAME);
        return;
    }
    functionCallNode->dataType = ptr->attribute->attr.functionSignature->returnType;
    Parameter* param = ptr->attribute->attr.functionSignature->parameterList;
    AST_NODE* i = paramListNode->child;
    for (; i && param; i = i->rightSibling, param = param->next)
        checkParameterPassing(param, i);
    if (i)
        printErrorMsg(idNode, TOO_MANY_ARGUMENTS);
    if (param)
        printErrorMsg(idNode, TOO_FEW_ARGUMENTS);
}


char* arrayTypeName(TypeDescriptor* type)
{
    static char buf[999];
    assert(type && type->kind == ARRAY_TYPE_DESCRIPTOR);
    int len = sprintf(buf, "%s", dataTypeName(type->properties.arrayProperties.elementType));
    for (int i = 0; i < type->properties.arrayProperties.dimension; ++i)
        len += sprintf(buf + len, "[%d]", type->properties.arrayProperties.sizeInEachDimension[i]);
    return buf;
}


void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
    switch (formalParameter->type->kind) {
    case SCALAR_TYPE_DESCRIPTOR:
        if (formalParameter->type->properties.dataType != actualParameter->dataType)
            printErrorMsg(actualParameter, PARAMETER_TYPE_UNMATCH);
        if (actualParameter->semantic_value.identifierSemanticValue.kind == ARRAY_ID)
            printErrorMsgSpecial(actualParameter, dataTypeName(formalParameter->type->properties.dataType), PASS_ARRAY_TO_SCALAR);
        break;
    case ARRAY_TYPE_DESCRIPTOR:
        if (formalParameter->type->properties.arrayProperties.elementType != actualParameter->dataType)
            printErrorMsg(actualParameter, PARAMETER_TYPE_UNMATCH);
        if (actualParameter->semantic_value.identifierSemanticValue.kind == NORMAL_ID)
            printErrorMsgSpecial(actualParameter, arrayTypeName(formalParameter->type), PASS_SCALAR_TO_ARRAY);
        break;
    }
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
    dfs(exprNode);
    AST_NODE* left = exprNode->child;
    AST_NODE* right = left->rightSibling;
    switch (exprNode->semantic_value.exprSemanticValue.kind) {
    case BINARY_OPERATION:
        switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
        case BINARY_OP_ADD:
        case BINARY_OP_SUB:
        case BINARY_OP_MUL:
        case BINARY_OP_DIV:
            exprNode->dataType = getBiggerType(left->dataType, right->dataType);
            break;
        default:
            exprNode->dataType = INT_TYPE;
            break;
        }
        break;
    case UNARY_OPERATION:
        exprNode->dataType = left->dataType;
        break;
    }
}


void processVariableLValue(AST_NODE* idNode)
{
    if (idNode->nodeType != IDENTIFIER_NODE)
        printErrorMsg(idNode, NOT_ASSIGNABLE);
}

void processVariableRValue(AST_NODE* idNode)
{
    switch (idNode->nodeType) {
    case IDENTIFIER_NODE:
    case EXPR_NODE:
    case CONST_VALUE_NODE:
        return;
    case STMT_NODE:
        if (idNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT)
            return;
    default:
        printErrorMsg(idNode, NOT_ASSIGNABLE);
    }
}


void processConstValueNode(AST_NODE* constValueNode)
{
    switch (constValueNode->semantic_value.const1->const_type) {
    case INTEGERC:
        constValueNode->dataType = INT_TYPE;
        break;
    case FLOATC:
        constValueNode->dataType = FLOAT_TYPE;
        break;
    case STRINGC:
        constValueNode->dataType = CONST_STRING_TYPE;
        break;
    }
}


void checkReturnStmt(AST_NODE* returnNode)
{
    dfs(returnNode);
    if (returnNode->child->dataType != g_currentReturnType) 
        if (!(returnNode->child->dataType == NONE_TYPE && g_currentReturnType == VOID_TYPE))
            printErrorMsg(returnNode, RETURN_TYPE_UNMATCH);
    if (returnNode->child->nodeType == IDENTIFIER_NODE)
       if (returnNode->child->semantic_value.identifierSemanticValue.kind == ARRAY_ID)
            printErrorMsg(returnNode, RETURN_ARRAY);
}


void processBlockNode(AST_NODE* blockNode)
{
    openScope();
    dfs(blockNode);
    closeScope();
}


void processStmtNode(AST_NODE* stmtNode)
{
    switch (stmtNode->semantic_value.stmtSemanticValue.kind) {
    case WHILE_STMT:
        checkWhileStmt(stmtNode);
        break;
    case FOR_STMT:
        checkForStmt(stmtNode);
        break;
    case ASSIGN_STMT:
        checkAssignmentStmt(stmtNode);
        break;
    case IF_STMT:
        checkIfStmt(stmtNode);
        break;
    case FUNCTION_CALL_STMT:
        checkFunctionCall(stmtNode);
        break;
    case RETURN_STMT:
        checkReturnStmt(stmtNode);
        break;
    }
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


void declareFunction(AST_NODE* returnTypeNode)
{
    AST_NODE* idNode = returnTypeNode->rightSibling;
    AST_NODE* paramListNode = idNode->rightSibling;
    AST_NODE* blockNode = paramListNode->rightSibling;
    DATA_TYPE returnType = g_currentReturnType = processTypeNode(returnTypeNode);
    SymbolAttribute* attr = createAttrFunc(FUNCTION_SIGNATURE, returnType);
    SymbolTableEntry* ptr = retrieveSymbol(idName(idNode));
    if (ptr) {
        idNode->linenumber = returnTypeNode->linenumber;
        printErrorMsg(idNode, SYMBOL_REDECLARE);
        idNode->semantic_value.identifierSemanticValue.symbolTableEntry = ptr;
    } else {
        idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName(idNode), attr);
    }
    openScope();
    dfs(paramListNode);
    Parameter* last = NULL;
    for (AST_NODE* i = paramListNode->child; i; i = i->rightSibling) {
        Parameter* cur = (Parameter*)malloc(sizeof(Parameter));
        cur->type = retrieveSymbol(idName(i->child->rightSibling))->attribute->attr.typeDescriptor;
        cur->parameterName = strdup(idName(i->child->rightSibling));
        attr->attr.functionSignature->parametersCount++;
        cur->next = NULL;
        if (last)
            last->next = cur;
        else
            attr->attr.functionSignature->parameterList = cur;
        last = cur;
    }
    dfs(blockNode);
    closeScope();
}


void checkIdentifierNode(AST_NODE* idNode)
{
    dfs(idNode);
    SymbolTableEntry* ptr = retrieveSymbol(idName(idNode));
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = ptr;
    if (!ptr) {
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
        return;
    } else if (ptr->attribute->attributeKind == TYPE_ATTRIBUTE) {
        printErrorMsg(idNode, IS_TYPE_NOT_VARIABLE);
    } else if (ptr->attribute->attributeKind == FUNCTION_SIGNATURE) {
        idNode->dataType = ptr->attribute->attr.functionSignature->returnType;
    } else if (ptr->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR) {
        idNode->dataType = ptr->attribute->attr.typeDescriptor->properties.dataType;
        idNode->semantic_value.identifierSemanticValue.kind = NORMAL_ID;
    } else if (ptr->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
        idNode->dataType = ptr->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
        idNode->semantic_value.identifierSemanticValue.kind = ARRAY_ID;
        int dim = 0;
        for (AST_NODE* i = idNode->child; i; i = i->rightSibling, ++dim)
            if (i->dataType != INT_TYPE)
                printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
        if (g_ignoreDim) {
            if (dim == ptr->attribute->attr.typeDescriptor->properties.arrayProperties.dimension) {
                idNode->semantic_value.identifierSemanticValue.kind = NORMAL_ID;
            } else if (dim < ptr->attribute->attr.typeDescriptor->properties.arrayProperties.dimension) {
                idNode->semantic_value.identifierSemanticValue.kind = ARRAY_ID;
            } else {
                printErrorMsgSpecial(idNode, ">", INCOMPATIBLE_ARRAY_DIMENSION);
            }
        } else {
            if (dim != ptr->attribute->attr.typeDescriptor->properties.arrayProperties.dimension) {
                if (dim > ptr->attribute->attr.typeDescriptor->properties.arrayProperties.dimension)
                    printErrorMsgSpecial(idNode, ">", INCOMPATIBLE_ARRAY_DIMENSION);
                else 
                    printErrorMsgSpecial(idNode, g_LValue ? "<L" : "<", INCOMPATIBLE_ARRAY_DIMENSION);
            } else {
                idNode->semantic_value.identifierSemanticValue.kind = NORMAL_ID;
            }
        }
    }
}
