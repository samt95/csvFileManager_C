/*  File: csvManager.c

    Developer: Sam Taylor

    CSV File Manager

    Reads in a csv file and performs operations
    to it such as printing or deleting rows, saving
    the file, and merging files.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csvFunctions.h"

char *programName;
static char *delims = " \t\n\r\f";

// function headers to allow forward use
static SPREADSHEET *load(SPREADSHEET *ss);
static void unload(SPREADSHEET *ss);
static void save(SPREADSHEET *ss);
static void merge(SPREADSHEET *ss);
static void stats(SPREADSHEET *ss);
static void sort(SPREADSHEET *ss);
static void sortnumeric(SPREADSHEET *ss);
static void deleterow(SPREADSHEET *ss);
static void findrow(SPREADSHEET *ss);
static void printrow(SPREADSHEET *ss);
static void evalsum(SPREADSHEET *ss);
static void evalavg(SPREADSHEET *ss);
static void help();
static int convertColId( char *text );
static char *getStringArg(void);
static int getColNumberArg(void);

// Main function where execution begins
//
// Usage:
// csvprog [-d] filename.csv
// where the -d flag enables debugging output
// Once started, the program repeatedly prompts for
// subcommands to be entered.
int main( int argc, char *argv[] ) {
    int i;
    int debug = 0;

    programName = argv[0];
    for( i=1; i<argc; i++ ) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            if (strcmp(arg,"-d") == 0) {
                debug = 1;
            } else {
                fprintf(stderr, "%s: unknown option %s\n", programName, arg);
            }
        } else {
            fprintf(stderr, "%s: unrecognized command line argument %s\n", programName, arg);
        }
    }

    SPREADSHEET *ss = NULL;	// the current spreadsheet

    SS_SetDebug(debug);

    char lineBuffer[80];
    char *command;

    // now process queries from standard input
    for( ; ; ) {
        printf("\nEnter a subcommand ==> ");
        fflush(stdout);

        // read one line
        if (fgets(lineBuffer, 80, stdin) == NULL)
            break;

        command = strtok(lineBuffer, delims);
        if (command == NULL || strcmp(command,"quit") == 0)
            break;
        if (strcmp(command,"load") == 0)
            ss = load(ss);
        else
        if (strcmp(command,"save") == 0)
            save(ss);
        else
        if (strcmp(command,"merge") == 0)
            merge(ss);
        else
        if (strcmp(command,"stats") == 0)
            stats(ss);
        else
        if (strcmp(command,"sort") == 0)
            sort(ss);
        else
        if (strcmp(command,"sortnumeric") == 0)
            sortnumeric(ss);
        else
        if (strcmp(command,"findrow") == 0)
            findrow(ss);
        else
        if (strcmp(command,"printrow") == 0)
            printrow(ss);
        else
       if (strcmp(command,"deleterow") == 0)
            deleterow(ss);
        else
        if (strcmp(command,"evalsum") == 0)
            evalsum(ss);
        else
        if (strcmp(command,"evalavg") == 0)
            evalavg(ss);
        else
        if (strcmp(command,"help") == 0)
            help();
        else
            printf("-- unrecognized command (%s), type help for a list of commands\n", command); //Error fixed!
    }
    printf("\nProgram exited\n");
    return 0;
}

static int isFileLoaded(SPREADSHEET *ss) {
    if (ss != NULL)
        return 1;
    printf("command ignored, no spreadsheet loaded\n");
    return 0;
}

static void unload(SPREADSHEET *ss) {
	if (ss == NULL) return;
	SS_Unload(ss);
}

static SPREADSHEET *load(SPREADSHEET *ss) {
    unload(ss);
	char *fileName = getStringArg();
    if(fileName == NULL) {
        printf("file name not specified\n");
        printf("valid command: load <filename>\n");
        return NULL;
    }
	return SS_ReadCSV(fileName);
}

static void save(SPREADSHEET *ss) {
	if (!isFileLoaded(ss)) return;
	char *fileName = getStringArg();
    if(fileName == NULL) {
        fileName = ss->fileName;
    }
	SS_SaveCSV(ss, fileName);
}

static void merge(SPREADSHEET *ss) {
	if (!isFileLoaded(ss)) return;
	char *fileName = getStringArg();
	SPREADSHEET *ss2 = SS_ReadCSV(fileName);
    if (ss2 == NULL) return;
	SS_MergeCSV(ss, ss2);
}

static void stats(SPREADSHEET *ss) {
	if (!isFileLoaded(ss)) return;
	SS_PrintStats(ss);
}

static void sort(SPREADSHEET *ss) {
	if (!isFileLoaded(ss)) return;
    int colNum = getColNumberArg();
    if (colNum < 0)
        return;
    SS_Sort(ss, colNum);
}

static void sortnumeric(SPREADSHEET *ss) {
	if (!isFileLoaded(ss)) return;
    int colNum = getColNumberArg();
    if (colNum < 0)
        return;
    SS_SortNumeric(ss, colNum);
}

static void deleterow(SPREADSHEET *ss) {
    char *row;
    int rowNum = 0;
	if (!isFileLoaded(ss)) return;
    row = strtok(NULL, delims);
    if (row == NULL) {
        printf("missing row number\n");
        return;
    }
    rowNum = atoi(row);
    SS_DeleteRow(ss, rowNum);
}

// Finds and outputs the number of the row which contains the
// specified text in the specified column, optionally starting at
// a specified row number
static void findrow(SPREADSHEET *ss) {
    char *text, *startPos;
    int startNum = 0;
    if (ss == NULL) return;
    int colNum = getColNumberArg();
    if (colNum < 0)
        return;
    if (colNum >= ss->numCols) {
        printf("-- bad column specification\n");
        return;
    }
    text = getStringArg();
    if (text == NULL) {
        printf("missing search text\n");
        return;
    }
    startPos = getStringArg();
    if (startPos != NULL)
    	startNum = atoi(startPos);
    if (startNum >= ss->numRows) {
        printf("-- bad start position specification\n");
        return;
    }
    int r = SS_FindRow(ss, colNum, text, startNum);
    if (r >= 0)
        printf("-- found in row %d\n", r);
    else
        printf("-- no matching row was found\n");
}

// Prints the row specified by its row number
static void printrow(SPREADSHEET *ss) {
    char *startRow, *endRow;
    int rowNum = 0;
    int endRowNum = 0;
    if (!isFileLoaded(ss)) return;
    startRow = strtok(NULL, delims);
    if (startRow == NULL) {
        printf("missing row number\n");
        return;
    }
    rowNum = atoi(startRow);
    endRow = strtok(NULL, delims);

    if(rowNum < 0){
        rowNum = 0;
    }
    if(endRow == NULL) {
        endRowNum = rowNum;
    } else {
        endRowNum = atoi(endRow);
    }    
    if (endRowNum >= ss->numRows){
        endRowNum = ss->numRows-1;
    }    
    do {
        SS_PrintRow(ss, rowNum);
    } while(++rowNum <= endRowNum);
}

static void evalsum(SPREADSHEET *ss) {
    int colNum = 0;
    if (!isFileLoaded(ss)) return;
    colNum = getColNumberArg();
    if (colNum < 0) {
        return;        
    }
    if (colNum >= ss->numCols) {
        printf("-- bad column specification\n");
        return;       
    }
    double sum = SS_ColumnSum(ss, colNum);
    printf("-- sum = %f\n", sum);
}

static void evalavg(SPREADSHEET *ss) {
    int colNum = 0;
    if (!isFileLoaded(ss)) return;
    colNum = getColNumberArg();
    if (colNum < 0) {
        return;        
    }
    if (colNum >= ss->numCols) {
        printf("-- bad column specification\n");
        return;       
    }
    double avg = SS_ColumnAvg(ss, colNum);
    printf("-- average = %f\n", avg);
}

static void help() {
    char *helptext[] = {
        "\nThe valid commands are:",
        "quit                     -- to exit the program",
        "help                     -- to display this help message",
        "load <filename>          -- to read in a spreadsheet",
        "save                     -- to save the spreadsheet back",
        "save <filename>          -- to save back to a different file",
        "merge <filename>         -- to read and append rows from another file",
        "stats                    -- to report on the spreadsheet size",
        "sort <col>               -- sort rows based on text data in column <col>",
        "sortnumeric <col>        -- sort rows based on numbers in column <col>",
        "deleterow <n>            -- delete row <n> from spreadsheet",
        "findrow <col> <text>     -- print the number of the first row",
        "                            such that column <col> holds <text>",
        "findrow <col> <text> <n> -- similarly, except search starts at",
        "                            row number n",
        "printrow <n>             -- print row number <n>",
        "printrow <n> <m>         -- print rows numbered <n> through <m>",
        "evalsum <col>            -- print the sum of the numbers that",
        "                            are in column <col>",
        "evalavg <col>            -- print the average of the numbers that",
        "                            are in column <col>",
        NULL
        };
    char **line;
    for(line = helptext; *line != NULL; line++) {
        printf("%s\n", *line);
    }
    putchar('\n');
}

// Columns can be identified by number (e.g. 17) or by the letter code
// used in Excel. Only single letters are supported, but numbers can
// be as large as desired.
//    convertColId("A"), convertColId("0") return 0
//    ...
//    convertColId("Z"), convertColId("25") return 25
static int convertColId( char *text ) {
    char c;
    int len = strlen(text);
    c = text[0];
    if (c >= 'A' && c <= 'Z' && len == 1) {
        return c-'A';
    }
    if (c >= '0' && c <= '9') {
        return atoi(text);
    }
    printf("-- bad column specification\n");
    return -1;
}

static char *getStringArg(void) {
	char *s;
	s = strtok(NULL, delims);
	if (s != NULL && s[0]=='\"') {
		char *closeQuotePos = index(s+1, '\"');
		if (closeQuotePos != NULL)
			*closeQuotePos = '\0';
		return s+1;
	}
	return s;
}

static int getColNumberArg(void) {
    char *col;
    int colNum = 0;
    col = strtok(NULL, delims);
    if (col == NULL) {
        printf("missing column specification\n");
        return -1;
    }
    colNum = convertColId(col);
    return colNum;;
}