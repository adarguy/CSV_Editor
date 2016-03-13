/* File: csvfunctions.c

Author: Adar Guy
contact: adarguy10@gmail.com

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csvfunctions.h"
#include <ctype.h>

#define MAXINPUTLINELEN     256
#define MAXITEMSPERROW		128

#define CHECKMALLOC(p)	if ((p)==NULL) { fprintf(stderr,"out of memory!"); exit(1); } else { }

static int debug = 0;

static int extractItems(char *line, char *row[]);
char *mystrdup(char *s);
void SS_SetDebug(int dbg) {
    debug = dbg;
}

SPREADSHEET *SS_ReadCSV(char *fileName) {
    char line[MAXINPUTLINELEN];
    char *tempRow[MAXITEMSPERROW];
    SPREADSHEET *result;
    struct OneRow *lastRow = NULL;
    int i;

	result = malloc(sizeof(SPREADSHEET));
	CHECKMALLOC(result);
    result->fileName = mystrdup(fileName);
    result->firstRow = NULL;
    result->numRows = result->numCols = 0;

    FILE *f = fopen(fileName, "r");
    if (f == NULL) {
        fprintf(stderr, "Unable to read from file %s\n", fileName);
        perror(fileName);
        return NULL;
    }
    for( i = 0; ; i++) {
        if (fgets(line, MAXINPUTLINELEN, f) == NULL)
            break;
        int k = extractItems(line, tempRow);
        if (result->numCols == 0) {
            result->numCols = k;
        } else
        if (result->numCols != k) {
            fprintf(stderr, "Row %d has different number of columns from first row\n", i);
            continue;	// ignore this row
        }
        char **rc = calloc(k, sizeof(char *));
        CHECKMALLOC(rc);
        struct OneRow *newrow = malloc( sizeof(struct OneRow));
        CHECKMALLOC(newrow);
        newrow->row = rc;
        newrow->nextRow = NULL;
        int ix;
        for( ix = 0; ix < k; ix++ ) {
            rc[ix] = tempRow[ix];
        }
        if (lastRow == NULL) {
            result->firstRow = newrow;
        } else {
            lastRow->nextRow = newrow;
        }
        lastRow = newrow;

    }
    result->numRows = i;
    fclose(f);
    return result;
}
void SS_SaveCSV(SPREADSHEET *ss, char *fileName) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_SaveCSV(--)\n");
    FILE *file;
    file = fopen(ss->fileName, "w");
    struct OneRow *currentRow = ss->firstRow;
    int i, j;

    for (j=0; j<ss->numRows; j++){
        for (i=0; i<ss->numCols; i++){
	        fprintf(file, "\"%s\",", currentRow->row[i]);     	
        }
	    fprintf(file, "\n");
	    currentRow = currentRow->nextRow;      
    }
    fclose(file);
    i = rename(ss->fileName, fileName);
    return;
}

extern void SS_Unload(SPREADSHEET *ss) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_Unload(--)\n");
    while (ss->numRows > 0)
        SS_DeleteRow(ss, 0);
    free(ss);
}

static char *getOneItem(char *line, char *tok) {
    char *tokSaved = tok;
    char c;
    c = *line++;
S1: if (c == '\"') {
        c = *line++;
        goto S2;
    }
    if (c == ',' || c == '\0' || c == '\n' || c == '\r') {
        goto S4;
    }
    *tok++ = c;
    c = *line++;
    goto S1;
S2: if (c == '\"') {
        c = *line++;
        goto S3;
    }
    if (c == '\0' || c == '\n' || c == '\r') {
        // unexpected end of input line
        fprintf(stderr, "mismatched doublequote found");
        goto S4;
    }
    *tok++ = c;
    c = *line++;
    goto S2;
S3: if (c == '\"') {
        *tok++ = '\"';
        c = *line++;
        goto S2;
    }
    if (c == ',' || c == '\0' || c == '\n' || c == '\r') {
        goto S4;
    }
    *tok++ = c;
    c = *line++;
    goto S1;
S4: if (c == '\0' || c == '\n' || c == '\r') {
        if (tokSaved == tok)
            return NULL;  // nothing was read
        line--;
    }
    *tok = '\0';
    return line;
}

static int extractItems(char *line, char *row[]) {
    char t[MAXINPUTLINELEN];
    int col = 0;
    for( ; ; ) {
        line = getOneItem(line,t);
        if (line == NULL) break;
        char *s = mystrdup(t);
        row[col++] = s;        
    }
    return col;
}

void SS_PrintStats(SPREADSHEET *ss) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_PrintStats(--)\n");
    printf("File: %s\nRows: %d\nColumns: %d\n", ss->fileName, ss->numRows, ss->numCols);   
}

void SS_MergeCSV(SPREADSHEET *ss1, SPREADSHEET *ss2) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_MergeCSV(--, --)\n");
    if (ss1->numCols != ss2->numCols) {
        printf("The 2 spreadsheets do not have equal columns");
        free(ss2);
        return;
    }
	struct OneRow *ss1rp, *ss2rp, *temp; 
    ss1rp = ss1->firstRow;    
    ss2rp = ss2->firstRow;   
	while (ss1rp->nextRow != NULL) {
		ss1rp = ss1rp->nextRow;
	}	
	ss1rp->nextRow = ss2rp;
    ss1->numRows += ss2->numRows;
    free(ss2);
}

void SS_DeleteRow(SPREADSHEET *ss, int rowNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_DeleteRow(--,%d)\n", rowNum);
	if (rowNum >=ss->numRows || rowNum<0) {
        printf("Row number (%d) is out of range\n", rowNum);
        return;
    }
    struct OneRow *rp1, *rp2;    
    int i=0;    
    rp1 = ss->firstRow;
    
    while (rp1 != NULL) {   
        if (i == rowNum) {      
            if(rp1 == ss->firstRow) {     
                ss->firstRow = rp1->nextRow;            
                free(rp1);          
                ss->numRows = ss->numRows - 1;          
                return;     
            }else{          
                rp2->nextRow = rp1->nextRow;            
                free(rp1);          
                ss->numRows = ss->numRows-1;            
                return;     
            }   
        }   
        rp2 = rp1;  
        rp1 = rp1->nextRow; 
        i++;    
    }
}
int cNum;
int cmpfn(const void *a, const void *b) {
        struct OneRow *aptr = *(struct OneRow **)a;
        struct OneRow *bptr = *(struct OneRow **)b;
        if (strcmp(aptr->row[cNum], bptr->row[cNum])<0)
            return -1;
        else if (strcmp(aptr->row[cNum], bptr->row[cNum])>0)
            return +1;
        else
            return 0;
    }
extern void SS_Sort(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_Sort(--,%d)\n", colNum);    
    if(colNum >=ss->numCols || colNum<0) {
        printf("Column number (%d) is out of range\n", colNum);
        return;
    }
    
    int numItems=0;
    struct OneRow *p;
    struct OneRow **arrayVersion;
    arrayVersion = calloc((ss->numRows), sizeof(struct OneRow *));
    numItems=0;
    for(p= ss->firstRow; p!=NULL; p=p->nextRow) {
        if (numItems >= ss->numRows) {
            ss->numRows *= 2;
            arrayVersion = realloc(arrayVersion, ss->numRows*sizeof(struct OneRow *));
        }
        arrayVersion[numItems++] = p;
    }
    qsort(arrayVersion, numItems, sizeof(struct OneRow *), cmpfn);   
    struct OneRow *nextRow=NULL;
    while(--numItems >= 0) {
        arrayVersion[numItems]->nextRow = nextRow;
        nextRow = arrayVersion[numItems];
    }
    free(arrayVersion);
    ss->firstRow=nextRow;
}

extern void SS_SortNumeric(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_SortNumeric(--,%d)\n", colNum);   
    if(colNum >=ss->numCols || colNum<0) {
        printf("Column number (%d) is out of range\n", colNum);
        return;
    }
    struct OneRow *rp = ss->firstRow;
    char *p;
    int i;
    int j,k,t;    
    while (rp != NULL){
        p=rp->row[colNum];
        t=strlen(p);
        for (j=0; j<t ;j++){       
            if(isdigit(*p)==0 && *p!= '.'){
                printf("Contents of spreasheet contain non-integer values\n");
                return;
            }
            p++;            
        }
        rp = rp->nextRow;
    }
    SS_Sort(ss,colNum);
    return;
}

int SS_FindRow(SPREADSHEET *ss, int colNum, char *text, int startNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_FindRow(--,%d,%s,%d)\n", colNum, text, startNum);
    if ((colNum >= ss->numCols || colNum < 0)) {
        printf("Column Number (%d) is out of range\n", colNum);
        return -1;  
    }
    if ((startNum >= ss->numRows || startNum < 0)) {
        printf("Row Number (%d)  is out of range\n", startNum);
        return -1;  
    }
    struct OneRow *cols = ss->firstRow;
    int i;    
    for( i=startNum; i<ss->numRows; i++ ) {
        if (strcmp(text, cols->row[colNum]) == 0)
            return i;
        cols = cols->nextRow; 
    }
    return -1;
}

void SS_PrintRow(SPREADSHEET *ss, int rowNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_PrintRow(--,%d)\n", rowNum);
    if (rowNum >= ss->numRows || rowNum < 0) {
        printf("Row number (%d) is out of range\n", rowNum);
        return;
    }
    struct OneRow *rp = ss->firstRow;
    while(rowNum > 0 && rp != NULL) {
        rp = rp->nextRow;
        rowNum--;
    }
    if (rp == NULL) {
        printf("Row number (%d) is out of range??\n", rowNum);
        return;        
    }
    
    int k;
    for( k = 0 ; k<ss->numCols; k++ ) {
        if (k>0)
            printf(", ");
        printf("%s", rp->row[k]);
    }
    putchar('\n');
    return;
}

double SS_ColumnSum(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_ColumnSum(--,%d)\n", colNum);double sum = 0.0;
    if (colNum >= ss->numCols || colNum<0) {
        printf("Column Number (%d) is out of range\n", colNum);
        return 0;
    }
    struct OneRow *currentRow = ss->firstRow;
    int a;
         
    for ( a=0; a<ss->numRows; a++ ) {
        if (atof(currentRow->row[colNum])==0) {
            printf("Column Number (%d) Contains Values That Cannot Be Operated On\n", colNum);
            return 0;
        }  
        sum += atof(currentRow->row[colNum]);
        currentRow = currentRow->nextRow;           
    }
    return sum;
}

double SS_ColumnAvg(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_ColumnAvg(--,%d)\n", colNum);
    if (colNum >= ss->numCols || colNum<0) {
        printf("Column Number (%d) is out of range\n", colNum);
        return 0;
    }
    double sum = SS_ColumnSum(ss,colNum);
    return sum/ss->numRows;
}

char *mystrdup(char *s) {
	int len = strlen(s);
	char *result = malloc(len+1);
	CHECKMALLOC(result);
	return strcpy(result, s);
}