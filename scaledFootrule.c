#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef struct LinesRep Lines;
struct LinesRep {
    int nLines;
    int *lineRow;
    int *lineCol;
};


// Function prototypes ...

double **Hungarian (int n, double **costMatrix);
Lines countZeroLines (double **costMatrix, int n);
double **getMatrix (char *fileName, int n);
double **copyMatrix (double **Matrix, int size);
int **copyMatrixToInt (double **Matrix, int size);


// Test driver for Hungarian();
int main (int argc, char **argv) {
    
    if (argc != 3) {
        fprintf (stderr, "usage: %s filename matrix-size\n", argv[0]);
        exit (1);
    }
    printf ("segfault\n");
    double **costMatrix = getMatrix (argv[1], atoi(argv[2]));

    printf("Original matrix:\n");
    for (int row = 0; row < 3; row++){
        for (int col = 0; col < 3; col++)
            printf("%5.1f ", costMatrix[row][col]);
        printf ("\n");
    }

    double **copyMatrix = Hungarian (3, costMatrix);

    printf("After Hungarian:\n");
    for (int row = 0; row < 3; row++){
        for (int col = 0; col < 3; col++)
            printf("%5.1f ", copyMatrix[row][col]);
        printf ("\n");
    }
    return 0;
}

double **Hungarian (int n, double **costMatrix) {
    double **matrixCopy = copyMatrix (costMatrix, n);

    // Step 1. Subtract the smallest entry in each row from all the entries of its row.
    // Stores smallest value of each row in the matrix ...
    double *smallestRow = malloc (n * sizeof (double));
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            // Set to smallest row and cols to first value ...
            if (col == 0) smallestRow[row] = matrixCopy[row][col];

            // Check if value is lowest in row ...
            if (matrixCopy[row][col] < smallestRow[row]) smallestRow[row] = matrixCopy[row][col];
        }
    }

    // Now subtract ...
    for (int row = 0; row < n; row++)
        for (int col = 0; col < n; col++) 
            matrixCopy[row][col] -= smallestRow[row];


    // Step 2. Subtract the smallest entry in each column from all the entries
    // of its column.
    
    // Stores smallest value of each column in the matrix ...
    double *smallestCol = malloc (n * sizeof (double));

    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            // Set to smallest row and cols to first value ...
            if (row == 0) smallestCol[col] = matrixCopy[row][col];

            // Check if value is lowest in col ...
            if (matrixCopy[row][col] < smallestCol[col]) smallestCol[col] = matrixCopy[row][col];
        }
    }

    // Now subtract ...
    for (int row = 0; row < n; row++)
        for (int col = 0; col < n; col++)
            matrixCopy[row][col] -= smallestCol[col];


    // Step 3. Draw lines through appropriate rows and columns so that all the
    // zero entries of the cost matrix are covered and the minimum number of
    // such lines is used.

    Lines Lines = countZeroLines (matrixCopy, n);

    // Step 4. Test for Optimality: (i) If the minimum number of covering lines
    // is n, an optimal assignment of zeros is possible and we are finished. (ii) If
    // the minimum number of covering lines is less than n, an optimal
    // assignment of zeros is not yet possible. In that case, proceed to Step 5.

    while (Lines.nLines < n) {

        // Step 5. Determine the smallest entry not covered by any line ...
        double smallest;
        int first = 0;

        for (int row = 0; row < n; row++) {
            // Only consider number if it's not covered by a line ...
            if (Lines.lineRow[row] != 0) continue;
            for (int col = 0; col < n; col++) {
                // Only consider number if it's not covered by a line ...
                if (Lines.lineCol[col] != 0) continue; 
                if (first == 0) {
                    smallest = matrixCopy[row][col];
                    first = 1;
                    continue;
                }
                if (matrixCopy[row][col] < smallest) smallest = matrixCopy[row][col];
            }
        }
        
        // Subtract this entry from each uncovered row, and then add it to each covered
        // column.

        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                // If row is uncovered ...
                if (Lines.lineRow[row] == 0) matrixCopy[row][col] -= smallest;

                // If column is covered ...
                if (Lines.lineCol[col] == 1) matrixCopy[row][col] += smallest;
            }
        }

        // Return to Step 3.
        Lines = countZeroLines (matrixCopy, n);
    }
    return matrixCopy;
    
}

Lines countZeroLines (double **costMatrix, int n) {

    Lines Lines;

    int **Matrix = copyMatrixToInt (costMatrix, n);

    Lines.nLines = 0;
    // Use > 0 to represent line, 0 to represent no line ...

    Lines.lineRow = calloc (n, n * sizeof (int));
    Lines.lineCol = calloc (n, n * sizeof (int));

    // Loop until minimum lines have been found ...
    while (1) {
        int *zeroCountRow = calloc (n, n * sizeof (int));
        int *zeroCountCol = calloc (n, n * sizeof (int));
        int zeroes = 0;
        // Count amount of zeroes in each row and column for hungarian algo ...
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (Matrix[row][col] == 0) {
                    zeroCountRow[row]++;
                    zeroCountCol[col]++;
                    zeroes++;
                }
            }
        }
        
        // If we've drawn a line through all the zeroes in the matrix, break
        if (zeroes == 0) break;

        ///////////////////////////////
        // Draw lines (the sauce)... //
        ///////////////////////////////
        int maxZeroR = 0;
        int maxZeroC = 0;
        int maxZeroRow;
        int maxZeroCol;
        for (int row = 0; row < n; row++) {
            if (zeroCountRow[row] >= maxZeroR) {
                maxZeroR = zeroCountRow[row];
                maxZeroRow = row;
            }
        }

        // Reset max count ...

        for (int col = 0; col < n; col++) {
            if (zeroCountCol[col] >= maxZeroC) {
                maxZeroC = zeroCountCol[col];
                maxZeroCol = col;
            }
        }
        // Set zeroes to -1 to not consider them anymore ...

        if (maxZeroR >= maxZeroC) {

            for (int col = 0; col < n; col++)
                Matrix[maxZeroRow][col] = -1;

            Lines.lineRow[maxZeroRow] = 1;

        } else {

            // Check to see if zero has already been drawn through ...
            for (int row = 0; row < n; row++) {
                Matrix[row][maxZeroCol] = -1;
            }
            Lines.lineCol[maxZeroCol] = 1;

        } 

    }

    for (int i = 0; i < n; i++) {
        if (Lines.lineCol[i] != 0)
            Lines.nLines++;

        if (Lines.lineRow[i] != 0)
            Lines.nLines++;        
    }

    return Lines;

}

double **getMatrix (char *fileName, int n) {
    FILE *fp = fopen (fileName, "r");
    
    double **matrix = malloc (n * sizeof (*matrix));

    for (int row = 0; row < n; row++) {
        matrix[row] = malloc (n * sizeof (double));
        for (int col = 0; col < n; col++) {
            fscanf(fp, "%lf", &matrix[row][col]);
        }
    }

    fclose (fp);

    return matrix;
}

double **copyMatrix (double **Matrix, int size) {
    double **copyMatrix = malloc (size * sizeof (double*));

    for (int row = 0; row < size; row++) {
        copyMatrix[row] = malloc (size * sizeof (double));
        for (int col = 0; col < size; col++) {
            copyMatrix[row][col] = Matrix[row][col];
        }
    }
    return copyMatrix;
}

int **copyMatrixToInt (double **Matrix, int size) {
    int **copyMatrix = malloc (size * sizeof (int*));

    for (int row = 0; row < size; row++) {
        copyMatrix[row] = malloc (size * sizeof (int));
        for (int col = 0; col < size; col++) {
            if (Matrix[row][col] != 0)
                copyMatrix[row][col] = 1;
            else
                copyMatrix[row][col] = 0;
        }
    }
    return copyMatrix;
}