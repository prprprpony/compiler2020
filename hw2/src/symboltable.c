#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include"header.h"

#define TABLE_SIZE	1048576

symtab * hash_table[TABLE_SIZE];
extern int linenumber;

/* djb2 */
int HASH(char * str){
    size_t idx = 5381;
    while (*str)
        idx = idx * 33 + *str++;
    return (idx & (TABLE_SIZE-1));
}

/*returns the symbol table entry if found else NULL*/

symtab * lookup(char *name){
    int hash_key;
    symtab* symptr;
    if(!name)
        return NULL;
    hash_key=HASH(name);
    symptr=hash_table[hash_key];

    while(symptr){
        if(!(strcmp(name,symptr->lexeme)))
            return symptr;
        symptr=symptr->front;
    }
    return NULL;
}


void insertID(char *name){
    int hash_key;
    symtab* ptr;
    symtab* symptr=(symtab*)malloc(sizeof(symtab));

    hash_key=HASH(name);
    ptr=hash_table[hash_key];

    if(ptr==NULL){
        /*first entry for this hash_key*/
        hash_table[hash_key]=symptr;
        symptr->front=NULL;
        symptr->back=symptr;
    }
    else{
        symptr->front=ptr;
        ptr->back=symptr;
        symptr->back=symptr;
        hash_table[hash_key]=symptr;
    }

    strcpy(symptr->lexeme,name);
    symptr->line=linenumber;
    symptr->counter=1;
}

void printSym(symtab* ptr) 
{
    printf("%s\t\t%d\n", ptr->lexeme, ptr->counter);
}

int symcmp(const void* a, const void* b)
{
    char *sa = (*((symtab**)a))->lexeme;
    char *sb = (*((symtab**)b))->lexeme;
    return strcmp(sa, sb);
}

void printSymTab()
{
    size_t cap = 512;
    symtab** sorted = malloc(cap * sizeof(symtab*));
    size_t sortedi = 0;

    int i;
    for (i=0; i<TABLE_SIZE; i++)
    {
        symtab* symptr;
        symptr = hash_table[i];
        while (symptr != NULL)
        {
            if (sortedi == cap)
                sorted = realloc(sorted, (cap *= 2) * sizeof(symtab*));
            sorted[sortedi++] = symptr;
            symptr=symptr->front;
        }
    }
    qsort(sorted, sortedi, sizeof(symtab*), symcmp);
    printf("\nFrequency of identifiers:\n");
    for (i=0; i<sortedi; i++)
    {
        printSym(sorted[i]);
    }
    free(sorted);
}
