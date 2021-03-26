#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include "header.h"


int main( int argc, char *argv[] )
{
    FILE *source, *target;
    Program program;
    SymbolTable symtab;

    if( argc == 3){
        source = fopen(argv[1], "r");
        target = fopen(argv[2], "w");
        if( !source ){
            printf("can't open the source file\n");
            exit(2);
        }
        else if( !target ){
            printf("can't open the target file\n");
            exit(2);
        }
        else{
            program = parser(source);
            fclose(source);
            symtab = build(program);
            check(&program, &symtab);
            gencode(program, target, &symtab);
        }
    }
    else{
        printf("Usage: %s source_file target_file\n", argv[0]);
    }


    return 0;
}

void ungets(char *str, FILE *source)
{
    int l = strlen(str);
    for (int i = l - 1; i >= 0; i--)
        ungetc(str[i], source);
    return;
}

/********************************************* 
  Scanning 
 *********************************************/
Token getNumericToken( FILE *source, char c )
{
    Token token;
    int i = 0;

    while( isdigit(c) ) {
        token.tok[i++] = c;
        c = fgetc(source);
    }

    if( c != '.' ){
        ungetc(c, source);
        token.tok[i] = '\0';
        token.type = IntValue;
        return token;
    }

    token.tok[i++] = '.';

    c = fgetc(source);
    if( !isdigit(c) ){
        ungetc(c, source);
        printf("Expect a digit : %c\n", c);
        exit(1);
    }

    while( isdigit(c) ){
        token.tok[i++] = c;
        c = fgetc(source);
    }

    ungetc(c, source);
    token.tok[i] = '\0';
    token.type = FloatValue;
    return token;
}

Token scanner( FILE *source )
{
    char c;
    Token token;

    while( !feof(source) ){
        c = fgetc(source);

        while( isspace(c) ) c = fgetc(source);

        if( isdigit(c) )
            return getNumericToken(source, c);

        token.tok[0] = c;
        token.tok[1] = '\0';
        if( isalpha(c) ){
            int i = 0;
            while( isalpha(c) ){
                token.tok[i++] = c;
                c = fgetc(source);
            }
            ungetc(c, source);
            token.tok[i] = '\0';
            if( strcmp(token.tok, "f") == 0 )
                token.type = FloatDeclaration;
            else if( strcmp(token.tok, "i") == 0 )
                token.type = IntegerDeclaration;
            else if( strcmp(token.tok, "p") == 0 )
                token.type = PrintOp;
            else{
                token.type = Alphabet;
            }
            return token;
        }

        switch(c){
            case '=':
                token.type = AssignmentOp;
                return token;
            case '+':
                token.type = PlusOp;
                return token;
            case '-':
                token.type = MinusOp;
                return token;
            case '*':
                token.type = MulOp;
                return token;
            case '/':
                token.type = DivOp;
                return token;
            case EOF:
                token.type = EOFsymbol;
                token.tok[0] = '\0';
                return token;
            case '(':
                token.type = LeftParenthesis;
                token.tok[0] = '(';
                token.tok[1] = '\0';
                return token;
            case ')':
                token.type = RightParenthesis;
                token.tok[0] = ')';
                token.tok[1] = '\0';
                return token;
            default:
                printf("Invalid character : %c\n", c);
                exit(1);
        }
    }

    token.tok[0] = '\0';
    token.type = EOFsymbol;
    return token;
}


/********************************************************
  Parsing
 *********************************************************/
Declaration parseDeclaration( FILE *source, Token token )
{
    Token token2;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            token2 = scanner(source);
            if (strcmp(token2.tok, "f") == 0 ||
                    strcmp(token2.tok, "i") == 0 ||
                    strcmp(token2.tok, "p") == 0) {
                printf("Syntax Error: %s cannot be used as id\n", token2.tok);
                exit(1);
            }
            return makeDeclarationNode( token, token2 );
        default:
            printf("Syntax Error: Expect Declaration %s\n", token.tok);
            exit(1);
    }
}

Declarations *parseDeclarations( FILE *source )
{
    Token token = scanner(source);
    Declaration decl;
    Declarations *decls;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            decl = parseDeclaration(source, token);
            decls = parseDeclarations(source);
            return makeDeclarationTree( decl, decls );
        case PrintOp:
        case Alphabet:
            ungets(token.tok, source);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect declarations %s\n", token.tok);
            exit(1);
    }
}

Expression *parseValue( FILE *source )
{
    Token token = scanner(source);
    Expression *value = (Expression *)malloc( sizeof(Expression) );
    value->leftOperand = value->rightOperand = NULL;

    switch(token.type){
        case Alphabet:
            (value->v).type = Identifier;
            strcpy((value->v).val.id, token.tok);
            break;
        case IntValue:
            (value->v).type = IntConst;
            (value->v).val.ivalue = atoi(token.tok);
            break;
        case FloatValue:
            (value->v).type = FloatConst;
            (value->v).val.fvalue = atof(token.tok);
            break;
        default:
            printf("Syntax Error: Expect Identifier or a Number %s\n", token.tok);
            exit(1);
    }

    return value;
}

Expression *parseExpressionTail( FILE *source, Expression *lvalue )
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case PlusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source, 0);
            return parseExpressionTail(source, expr);
        case MinusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source, 0);
            return parseExpressionTail(source, expr);
        case MulOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MulNode;
            (expr->v).val.op = Mul;
            if ((lvalue->v).type == MulNode || (lvalue->v).type == DivNode) {
                expr->leftOperand = lvalue;
                expr->rightOperand = parseTerm(source, 0);
                return parseExpressionTail(source, expr);
            } else {
                expr->leftOperand = lvalue->rightOperand;
                lvalue->rightOperand = expr;
                expr->rightOperand = parseTerm(source, 0);
                return parseExpressionTail(source, lvalue);
            }
        case DivOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = DivNode;
            (expr->v).val.op = Div;
            if ((lvalue->v).type == MulNode || (lvalue->v).type == DivNode) {
                expr->leftOperand = lvalue;
                expr->rightOperand = parseTerm(source, 0);
                return parseExpressionTail(source, expr);
            } else {
                expr->leftOperand = lvalue->rightOperand;
                lvalue->rightOperand = expr;
                expr->rightOperand = parseTerm(source, 0);
                return parseExpressionTail(source, lvalue);
            }
        case Alphabet:
        case PrintOp:
        case RightParenthesis:
            ungets(token.tok, source);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}

Expression *parseExpression( FILE *source, Expression *lvalue )
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case PlusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source, 0);
            return parseExpressionTail(source, expr);
        case MinusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source, 0);
            return parseExpressionTail(source, expr);
        case MulOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MulNode;
            (expr->v).val.op = Mul;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source, 0);
            return parseExpressionTail(source, expr);
        case DivOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = DivNode;
            (expr->v).val.op = Div;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source, 0);
            return parseExpressionTail(source, expr);
        case Alphabet:
        case PrintOp:
        case RightParenthesis:
            ungets(token.tok, source);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}

Expression *parseTerm( FILE *source , int first)
{
    Token token;
    Expression *val, *expr;
    token = scanner(source);
    if (token.type == LeftParenthesis) {
        val = parseTerm(source, 1);
        token = scanner(source);
        if (token.type != RightParenthesis) {
            printf("Syntax Error: Expect a right parenthesis ')'\n");
            exit(1);
        }
    } else {
        ungets(token.tok, source);
        val = parseValue(source);
    }
    assert(val);
    if (!first) return val;
    expr = parseExpression(source, val);
    return expr ? expr : val;
}

Statement parseStatement( FILE *source, Token token )
{
    Token next_token;
    Expression *term;

    switch(token.type){
        case Alphabet:
            next_token = scanner(source);
            if(next_token.type == AssignmentOp){
                term = parseTerm(source, 1);
                return makeAssignmentNode(token.tok, term);
            }
            else{
                printf("Syntax Error: Expect an assignment op %s\n", next_token.tok);
                exit(1);
            }
        case PrintOp:
            next_token = scanner(source);
            if(next_token.type == Alphabet)
                return makePrintNode(next_token.tok);
            else{
                printf("Syntax Error: Expect an identifier %s\n", next_token.tok);
                exit(1);
            }
            break;
        default:
            printf("Syntax Error: Expect a statement %s\n", token.tok);
            exit(1);
    }
}

Statements *parseStatements( FILE * source )
{

    Token token = scanner(source);
    Statement stmt;
    Statements *stmts;

    switch(token.type){
        case Alphabet:
        case PrintOp:
            stmt = parseStatement(source, token);
            stmts = parseStatements(source);
            return makeStatementTree(stmt , stmts);
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect statements %s\n", token.tok);
            exit(1);
    }
}


/*********************************************************************
  Build AST
 **********************************************************************/
Declaration makeDeclarationNode( Token declare_type, Token identifier )
{
    Declaration tree_node;

    switch(declare_type.type){
        case FloatDeclaration:
            tree_node.type = Float;
            break;
        case IntegerDeclaration:
            tree_node.type = Int;
            break;
        default:
            break;
    }
    strcpy(tree_node.name, identifier.tok);

    return tree_node;
}

Declarations *makeDeclarationTree( Declaration decl, Declarations *decls )
{
    Declarations *new_tree = (Declarations *)malloc( sizeof(Declarations) );
    new_tree->first = decl;
    new_tree->rest = decls;

    return new_tree;
}


Statement makeAssignmentNode( char *id, Expression *term )
{
    Statement stmt;
    AssignmentStatement assign;

    stmt.type = Assignment;
    strcpy(assign.id, id);
    assign.expr = term;
    stmt.stmt.assign = assign;

    return stmt;
}

Statement makePrintNode( char *id )
{
    Statement stmt;
    stmt.type = Print;
    strcpy(stmt.stmt.variable, id);

    return stmt;
}

Statements *makeStatementTree( Statement stmt, Statements *stmts )
{
    Statements *new_tree = (Statements *)malloc( sizeof(Statements) );
    new_tree->first = stmt;
    new_tree->rest = stmts;

    return new_tree;
}

/* parser */
Program parser( FILE *source )
{
    Program program;

    program.declarations = parseDeclarations(source);
    program.statements = parseStatements(source);

    return program;
}


/********************************************************
  Build symbol table
 *********************************************************/
void InitializeTable( SymbolTable *table )
{
    table->count = 0;
}

void add_table( SymbolTable *table, char *name, DataType t )
{
    int index = lookup_index(table, name);
    if(index >= 0) {
        printf("Error : id %s has been declared\n", name);//error
        exit(1);
    }
    else if(table->count >= 23) {
        printf("Error : Exceeds maximum number (23) of variables\n");//error
        exit(1);
    }
    else{
        Symbol *sym = &(table->table[table->count++]);
        strcpy(sym->name, name);
        sym->type = t;
    }
}

SymbolTable build( Program program )
{
    SymbolTable table;
    Declarations *decls = program.declarations;
    Declaration current;

    InitializeTable(&table);

    while(decls !=NULL){
        current = decls->first;
        add_table(&table, current.name, current.type);
        decls = decls->rest;
    }

    return table;
}


/********************************************************************
  Type checking
 *********************************************************************/

void convertType( Expression * old, DataType type )
{
    if(old->type == Float && type == Int){
        printf("error : can't convert float to integer\n");
        exit(1);
    }
    if(old->type == Int && type == Float){
        Expression *tmp = (Expression *)malloc( sizeof(Expression) );
        if(old->v.type == Identifier)
            printf("convert to float %s \n",old->v.val.id);
        else
            printf("convert to float %d \n", old->v.val.ivalue);
        if (old->v.type == IntConst) { // constant folding
            old->v.val.fvalue = old->v.val.ivalue;
            old->v.type = FloatConst;
        } else {
            tmp->v = old->v;
            tmp->leftOperand = old->leftOperand;
            tmp->rightOperand = old->rightOperand;
            tmp->type = old->type;

            Value v;
            v.type = IntToFloatConvertNode;
            v.val.op = IntToFloatConvert;
            old->v = v;
            old->type = Int;
            old->leftOperand = tmp;
            old->rightOperand = NULL;
        }
    }
}

DataType generalize( Expression *left, Expression *right )
{
    if(left->type == Float || right->type == Float){
        printf("generalize : float\n");
        return Float;
    }
    printf("generalize : int\n");
    return Int;
}

int lookup_index( SymbolTable *table, char *c )
{
    int id = -1;
    for(int i = 0; i < table->count; i++){
        if (strcmp(table->table[i].name, c) == 0){
            id = i;
            break;
        }
    }
    return id;
}

DataType lookup_type( SymbolTable *table, char *c )
{
    int id = lookup_index(table, c);
    if( id < 0 ) {
        printf("Error : identifier %s is not declared\n", c);//error
        exit(1);
    }
    return table->table[id].type;
}

void checkexpression( Expression * expr, SymbolTable * table )
{
    assert(expr);
    char c[MAX_LEN];
    if(expr->leftOperand == NULL && expr->rightOperand == NULL){
        switch(expr->v.type){
            case Identifier:
                strcpy(c, expr->v.val.id);
                printf("identifier : %s\n",c);
                expr->type = lookup_type(table, c);
                break;
            case IntConst:
                printf("constant : int\n");
                expr->type = Int;
                break;
            case FloatConst:
                printf("constant : float\n");
                expr->type = Float;
                break;
            default:
                printf("Internal Error: invalid expression tree\n");
                exit(3);
        }
    }
    else{
        Expression *left = expr->leftOperand;
        Expression *right = expr->rightOperand;

        checkexpression(left, table);
        checkexpression(right, table);

        DataType type = generalize(left, right);
        convertType(left, type);
        convertType(right, type);
        expr->type = type;
        // constant folding
        if (left->v.type == IntConst && right->v.type == IntConst) {
            int64_t res;
            switch (expr->v.type) {
                case PlusNode:
                    res = (int64_t)left->v.val.ivalue + (int64_t)right->v.val.ivalue;
                    if (res < INT_MIN || res > INT_MAX) return;
                    expr->v.val.ivalue = left->v.val.ivalue + right->v.val.ivalue;
                    break;
                case MinusNode:
                    res = (int64_t)left->v.val.ivalue - (int64_t)right->v.val.ivalue;
                    if (res < INT_MIN || res > INT_MAX) return;
                    expr->v.val.ivalue = left->v.val.ivalue - right->v.val.ivalue;
                    break;
                case MulNode:
                    res = (int64_t)left->v.val.ivalue * (int64_t)right->v.val.ivalue;
                    if (res < INT_MIN || res > INT_MAX) return;
                    expr->v.val.ivalue = left->v.val.ivalue * right->v.val.ivalue;
                    break;
                case DivNode:
                    if (right->v.val.ivalue == 0) return;
                    res = (int64_t)left->v.val.ivalue / (int64_t)right->v.val.ivalue;
                    if (res < INT_MIN || res > INT_MAX) return;
                    expr->v.val.ivalue = left->v.val.ivalue / right->v.val.ivalue;
                    break;
                default:
                    printf("Internal Error: invalid expression tree\n");
                    exit(3);
            }
            expr->v.type = IntConst;
            free(left);
            free(right);
            expr->leftOperand = expr->rightOperand = NULL;
        }
    }
}

void checkstmt( Statement *stmt, SymbolTable * table )
{
    if(stmt->type == Assignment){
        AssignmentStatement assign = stmt->stmt.assign;
        printf("assignment : %s \n",assign.id);
        checkexpression(assign.expr, table);
        stmt->stmt.assign.type = lookup_type(table, assign.id);
        if (assign.expr->type == Float && stmt->stmt.assign.type == Int) {
            printf("error : can't convert float to integer\n");
            exit(1);
        } else {
            convertType(assign.expr, stmt->stmt.assign.type);
        }
    }
    else if (stmt->type == Print){
        printf("print : %s \n",stmt->stmt.variable);
        lookup_type(table, stmt->stmt.variable);
    }
    else {
        printf("error : statement error\n");//error
        exit(1);
    }
}

void check( Program *program, SymbolTable * table )
{
    Statements *stmts = program->statements;
    while(stmts != NULL){
        checkstmt(&stmts->first,table);
        stmts = stmts->rest;
    }
}


/***********************************************************************
  Code generation
 ************************************************************************/
void fprint_op( FILE *target, ValueType op )
{
    switch(op){
        case MinusNode:
            fprintf(target,"-\n");
            break;
        case PlusNode:
            fprintf(target,"+\n");
            break;
        case MulNode:
            fprintf(target,"*\n");
            break;
        case DivNode:
            fprintf(target,"/\n");
            break;
        default:
            fprintf(target,"Error in fprintf_op ValueType = %d\n",op);
            exit(1);
    }
}

void fprint_expr( FILE *target, Expression *expr, SymbolTable *table, int * precision )
{

    if(expr->leftOperand == NULL){
        switch( (expr->v).type ){
            case Identifier:
                ;int id = lookup_index(table, (expr->v).val.id);
                char reg = 'a' + id;
                fprintf(target,"l%c\n",reg);
                break;
            case IntConst:
                fprintf(target,"%d\n",(expr->v).val.ivalue);
                break;
            case FloatConst:
                fprintf(target,"%.5f\n", (expr->v).val.fvalue);
                break;
            default:
                fprintf(target,"Error In fprint_left_expr. (expr->v).type=%d\n",(expr->v).type);
                exit(1);
        }
    }
    else{
        fprint_expr(target, expr->leftOperand, table, precision);
        if (expr->type == Float && *precision != 5) {
            fprintf(target,"5k\n");
            *precision = 5;
        }
        if (expr->type == Int && *precision != 0) {
            fprintf(target,"0k\n");
            *precision = 0;
        }
        if(expr->rightOperand == NULL) { // IntToFloatConvertNode
            if (*precision != 5) {
                fprintf(target,"5k\n");
                *precision = 5;
            }
        } else { // arithmetic nodes
            fprint_expr(target, expr->rightOperand, table, precision);
            fprint_op(target, (expr->v).type);
        }
    }
}

void gencode(Program prog, FILE * target, SymbolTable *table)
{
    Statements *stmts = prog.statements;
    Statement stmt;
    int precision = 0;

    while(stmts != NULL){
        stmt = stmts->first;
        switch(stmt.type){
            case Print:
                ;int id = lookup_index(table, stmt.stmt.variable);
                char reg = 'a' + id;
                fprintf(target,"l%c\n",reg);
                fprintf(target,"p\n");
                break;
            case Assignment:
                fprint_expr(target, stmt.stmt.assign.expr, table, &precision);
                id = lookup_index(table, stmt.stmt.assign.id);
                reg = 'a' + id;
                fprintf(target,"s%c\n",reg);
                break;
        }
        stmts=stmts->rest;
    }

}


/***************************************
  For our debug,
  you can omit them.
 ****************************************/
void print_expr(Expression *expr)
{
    if(expr == NULL)
        return;
    else{
        print_expr(expr->leftOperand);
        switch((expr->v).type){
            case Identifier:
                printf("%s ", (expr->v).val.id);
                break;
            case IntConst:
                printf("%d ", (expr->v).val.ivalue);
                break;
            case FloatConst:
                printf("%f ", (expr->v).val.fvalue);
                break;
            case PlusNode:
                printf("+ ");
                break;
            case MinusNode:
                printf("- ");
                break;
            case MulNode:
                printf("* ");
                break;
            case DivNode:
                printf("/ ");
                break;
            case IntToFloatConvertNode:
                printf("(float) ");
                break;
            default:
                printf("error ");
                exit(1);
        }
        print_expr(expr->rightOperand);
    }
}

void test_parser( FILE *source )
{
    Declarations *decls;
    Statements *stmts;
    Declaration decl;
    Statement stmt;
    Program program = parser(source);

    decls = program.declarations;

    while(decls != NULL){
        decl = decls->first;
        if(decl.type == Int)
            printf("i ");
        if(decl.type == Float)
            printf("f ");
        printf("%s ",decl.name);
        decls = decls->rest;
    }

    stmts = program.statements;

    while(stmts != NULL){
        stmt = stmts->first;
        if(stmt.type == Print){
            printf("p %s ", stmt.stmt.variable);
        }

        if(stmt.type == Assignment){
            printf("%s = ", stmt.stmt.assign.id);
            print_expr(stmt.stmt.assign.expr);
        }
        stmts = stmts->rest;
    }

}
