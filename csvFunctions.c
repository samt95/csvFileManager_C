/* File: csvfunctions2.c

Sam Taylor
V00811400

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csvFunctions.h"

#define MAXINPUTLINELEN     256
#define MAXITEMSPERROW		128

#define CHECKMALLOC(p)	if ((p)==NULL) { fprintf(stderr,"out of memory!"); exit(1); } else { }

static int debug = 0;
static int sortcol = 0;


// forward declarations
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
    for(i = 0; ; i++) {
        if (fgets(line, MAXINPUTLINELEN, f) == NULL)
            break;
        int k = extractItems(line, tempRow);
        if (i == 0) { // set numCols equals the the number of columns in the first row
            result->numCols = k;
        } 
        else if (result->numCols != k) {
            fprintf(stderr, "Row %d has different number of columns from first row and was ignored\n", i);
            continue;	// ignore this row
        }
        result->numRows++;
        // instantiate the storage for the new row and copy k cells into it
        char **rc = calloc(k, sizeof(char *));
        CHECKMALLOC(rc);
        struct OneRow *newrow = malloc(sizeof(struct OneRow));
        CHECKMALLOC(newrow);
        newrow->row = rc;
        newrow->nextRow = NULL;
        int ix;
        for( ix = 0; ix < k; ix++ ) {
            rc[ix] = tempRow[ix];
        }
        
        // add the new row as the last row in the spreadsheet
        if (lastRow == NULL) {
            result->firstRow = newrow;
        } else {
            lastRow->nextRow = newrow;
        }
        lastRow = newrow;

    }
    fclose(f);
    return result;
}

// Write the spreadsheet in CSV format to the specified file 
void SS_SaveCSV(SPREADSHEET *ss, char *fileName) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_SaveCSV(--)\n");
    struct OneRow *rp = ss->firstRow;
    FILE *f;
    f =fopen(fileName, "w");
    for(int i = 0; i < ss->numRows; i++) {
        for(int k = 0; k < ss->numCols; k++) {
            fprintf(f, "\"%s\"", rp->row[k]);
            if(k < ss->numCols-1)
                fprintf(f, ",");
        }
        fprintf(f, "\n");
        rp = rp->nextRow;
    }
    fclose(f);
    return;
}

// Free all storage being use by the spreadsheet instance.
extern void SS_Unload(SPREADSHEET *ss) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_Unload(--)\n");
    for(int i = 0; i < ss->numRows; i++) {
        SS_DeleteRow(ss, 0);        
    }
    return;
}

// Reads one item from the csv row.
// line is a string where reading should begin; tok reference a char array into
// which the characters for the item are copied.
// On return, the result references the remainder of the original string which has
// not been read, or is NULL if no item could be read before the end of the line.
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

// Extracts items one by one from line, copies them into heap storage,
// and stores references to them in the row array.
// The function result is the number of items copied.
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

// prints filename, number of rows and number of columns for
// this spreadsheet
void SS_PrintStats(SPREADSHEET *ss) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_PrintStats(--)\n");
	printf("File: %s\n", ss->fileName);
    printf("Rows: %d\n", ss->numRows);
    printf("Columns: %d\n", ss->numCols);
}


// Transfers rows from spreadsheet ss2 to the end of spreadsheet ss1
// then releases any storage for ss2 which is no longer needed
void SS_MergeCSV(SPREADSHEET *ss1, SPREADSHEET *ss2) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_MergeCSV(--, --)\n");

    if(ss1->numCols != ss2->numCols) {
        printf("Can not merge -- different number of columns in each file");
        SS_Unload(ss2);
        return;
    }
    struct OneRow *temp = ss1->firstRow;
    for(int i = 0; i < ss1->numRows-1; i++){
        temp = temp->nextRow;
    }
    temp->nextRow = ss2->firstRow;
    ss1->numRows += ss2->numRows;
}

// Deletes the specified row from the spreadsheet.
// Any storage used by the row is freed.
void SS_DeleteRow(SPREADSHEET *ss, int rowNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_DeleteRow(--,%d)\n", rowNum);
    if(rowNum < 0 || rowNum > ss->numRows) {
        printf("-- bad row specification\n");
        return;
    }
    if(ss->numRows == 0){
        printf("Spreadsheet is empty\n");
        return;
    }
    if(ss->numRows == 1) {
        free(ss->firstRow->row);
        free(ss->firstRow);
        ss->numRows--;
        return;        
    }   
    struct OneRow *rp = ss->firstRow;
    if(rowNum == 0) {
        ss->firstRow = ss->firstRow->nextRow;   
        free(rp->row);
        rp->nextRow = NULL;
        free(rp);
        ss->numRows--;
        return;
    }
    for(int i = 0; i < rowNum-1; i++) {
        rp = rp->nextRow;
    }   
    if(rowNum == ss->numRows-1) {
        free(rp->nextRow->row);
        free(rp->nextRow);
        ss->numRows--;
        return;
    }
    struct OneRow *rp2 = ss->firstRow;
    for(int i = 0; i < rowNum; i++) {
        rp2 = rp2->nextRow;
    } 
    free(rp->nextRow->row);
    rp->nextRow = rp2->nextRow;
    rp2->nextRow = NULL;
    free(rp2);
    rp->nextRow = NULL;
    ss->numRows--;
    return;
}
int stringcmp(const void *a, const void *b) {
    struct OneRow *aptr=  *(struct OneRow **)a;
    struct OneRow *bptr=  *(struct OneRow **)b;
    if(strcmp(aptr->row[sortcol], bptr->row[sortcol]) < 0) 
        return -1;
    else if(strcmp(aptr->row[sortcol], bptr->row[sortcol]) > 0)
        return +1;
    else
        return 0;
}

// Sorts the rows of the spreadsheet into ascending order, based on
// the strings in the specified column
extern void SS_Sort(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_Sort(--,%d)\n", colNum);
    sortcol = colNum;
    struct OneRow *temp[ss->numRows];
    struct OneRow *rp = ss->firstRow;
    for(int i = 0; i < ss->numRows; i++) {
        temp[i] = rp;
        rp = rp->nextRow;
    }

    qsort(temp, ss->numRows, sizeof(struct OneRow*), stringcmp);
    rp = NULL;
    for(int i = ss->numRows-1; i >= 0; i--) {
        temp[i]->nextRow = rp;
        rp = temp[i];
    }
    ss->firstRow = temp[0];
    return;
	
}

int floatcmp(const void *a, const void *b) {
    struct OneRow *aptr=  *(struct OneRow **)a;
    struct OneRow *bptr=  *(struct OneRow **)b;
    float x = atof(aptr->row[sortcol]);
    float y = atof(bptr->row[sortcol]);
    if(x < y) 
        return -1;
    else if(x > y)
        return +1;
    else
        return 0;
}

// Sorts the rows of the spreadsheet into ascending order, based on
// the values of the floating point numbers in the specified column
extern void SS_SortNumeric(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_SortNumeric(--,%d)\n", colNum);
    sortcol = colNum;
    struct OneRow *temp[ss->numRows];
    struct OneRow *rp = ss->firstRow;
    for(int i = 0; i < ss->numRows; i++) {
        temp[i] = rp;
        rp = rp->nextRow;
    }

    qsort(temp, ss->numRows, sizeof(struct OneRow*), floatcmp);
    rp = NULL;
    for(int i = ss->numRows-1; i >= 0; i--) {
        temp[i]->nextRow = rp;
        rp = temp[i];
    }
    ss->firstRow = temp[0];
    return;
}

// Searches down the specified column for a row which contains text.
// The search starts at row number startNum;
// The result is the row number (where the first row is numbered 0).
// If the text is not found, the result is -1.
int SS_FindRow(SPREADSHEET *ss, int colNum, char *text, int startNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_FindRow(--,%d,%s,%d)\n",
            colNum, text, startNum);
    struct OneRow *rp = ss->firstRow;
    for( int k = 0; k  < startNum; k++)
        rp = rp->nextRow;
    for( int i = startNum; i < ss->numRows; i++) {
        if(strcmp(rp->row[colNum], text) == 0)
            return i;
        rp = rp->nextRow;
    }
    return -1;
}

// Outputs the specified row of the spreadsheet.
// It is printed as one line on standard output.
void SS_PrintRow(SPREADSHEET *ss, int rowNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_PrintRow(--,%d)\n", rowNum);
    if (rowNum >= ss->numRows || rowNum < 0) {
        printf("Row number (%d) is out of range.\n", rowNum);
        return;
    }

    struct OneRow *rp = ss->firstRow;
    while(rowNum > 0 && rp != NULL) {
        rp = rp->nextRow;
        rowNum--;
    }

    for(int k = 0 ; k<ss->numCols; k++) {
        if (k>0)
            printf(", ");
        printf("%s", rp->row[k]);
    }
    putchar('\n');
    return;
}

// The specified column must contain the textual representations of numbers
// (either integer or floating point). The sum of the numbers in the column
// is returned as a floating point number.
double SS_ColumnSum(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_ColumnSum(--,%d)\n", colNum);
    double sum = 0.0;
    struct OneRow *rp = ss->firstRow;
    for( int i = 0; i < ss->numRows; i++) {
        double val = atof(rp->row[colNum]);
        sum += val;
        rp = rp->nextRow;
    }
    return sum;
}

double SS_ColumnAvg(SPREADSHEET *ss, int colNum) {
    if (debug)
        fprintf(stderr, "DEBUG: Call to SS_ColumnAvg(--,%d)\n", colNum);
    double sum = SS_ColumnSum(ss,colNum);
    return sum/ss->numRows;
}

// The strdup function is provided in many but not all variants of the
// C library. Here it is, renamed as mystrdup, just in case.
char *mystrdup(char *s) {
	int len = strlen(s);
	char *result = malloc(len+1);
	CHECKMALLOC(result);
	return strcpy(result, s);
}
