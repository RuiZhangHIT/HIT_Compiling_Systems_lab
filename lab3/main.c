#include "inter.h"
#include "syntax.tab.h"

extern int yyparse();
extern void yyrestart(FILE*);
extern pNode root;

int lexError = 0;
int synError = 0;

int main(int argc, char** argv) 
{
    if (argc <= 1) 
        return 1;
    
    FILE* fr = fopen(argv[1], "r");
    if (!fr) {
        perror(argv[1]);
        return 1;
    }

    FILE* fw = fopen(argv[2], "wt+");
    if (!fw) {
        perror(argv[2]);
        return 1;
    }

    yyrestart(fr);
    yyparse();
    if (!lexError && !synError) {
        table = initTable();
        traverseTree(root);

        interCodeList = newInterCodeList();  
        genInterCodes(root);
        if (!interError)
            printInterCode(fw, interCodeList);

        deleteTable(table);
    }
    
    delNode(root);
    return 0;
}
