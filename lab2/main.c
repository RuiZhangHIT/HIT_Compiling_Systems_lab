#include "node.h"
#include "syntax.tab.h"
#include "semantic.h"
#include <stdio.h>

extern int yyparse();
extern void yyrestart(FILE*);
extern pNode root;

int lexError = 0;
int synError = 0;

int main(int argc, char** argv) 
{
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) 
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if (!lexError && !synError) {
        table = initTable();
        traverseTree(root);
        deleteTable(table);
    }
    delNode(root);
    return 0;
}
