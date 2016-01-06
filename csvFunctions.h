/* File: csvfunctions.h

Sam Taylor
V00811400

*/

struct OneRow {
	char **row;   // a pointer to an array of strings
	struct OneRow *nextRow;  // pointer to next row
};

struct SpreadSheet {
    char   *fileName;
    int     numRows;
    int     numCols;
    struct OneRow *firstRow;
};

typedef struct SpreadSheet SPREADSHEET;

// Prototypes for the module functions
// See comments in csvfunctions2.c for more details of how these functions
// are used
extern SPREADSHEET *SS_ReadCSV(char *fileName);
extern void SS_SaveCSV(SPREADSHEET *ss, char *fileName);
extern void SS_Unload(SPREADSHEET *ss);
extern void SS_PrintStats(SPREADSHEET *ss);

extern void SS_SetDebug(int debug);

extern int SS_FindRow(SPREADSHEET *ss, int colNum, char *text, int startNum);
extern void SS_PrintRow(SPREADSHEET *ss, int rowNum);
extern double SS_ColumnSum(SPREADSHEET *ss, int colNum);
extern double SS_ColumnAvg(SPREADSHEET *ss, int colNum);

extern void SS_MergeCSV(SPREADSHEET *ss1, SPREADSHEET *ss2);
extern void SS_DeleteRow(SPREADSHEET *ss, int rowNum);

extern void SS_Sort(SPREADSHEET *ss, int colNum);
extern void SS_SortNumeric(SPREADSHEET *ss, int colNum);
