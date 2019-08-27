// TODO:
// - Free shit

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
    int zeroes;
};


// Function prototypes ...

double **Hungarian (int n, double **costMatrix);

Lines countZeroLines (double **costMatrix, int n);
Lines newLines (int n);
void minLinesRow (Lines *Lines, int **Matrix, int n);
void minLinesCol (Lines *Lines, int **Matrix, int n);


double **getMatrix (char *fileName, int n);
double **copyMatrix (double **Matrix, int size);
int **copyMatrixToInt (double **Matrix, int size);
void printMatrix (double **matrix, int n);
void printIntMatrix (int **matrix, int n);


// Test driver for Hungarian();
int main (int argc, char **argv) {
    
    if (argc != 3) {
        fprintf (stderr, "usage: ./a.exe filename matrix-size\n");
        exit (1);
    }

    int n = atoi (argv[2]);

    double **costMatrix = getMatrix (argv[1], n);

    printf("Original matrix:\n");
    printMatrix (costMatrix, n);

    double **copyMatrix = Hungarian (n, costMatrix);

    printf("After Hungarian:\n");
    printMatrix (copyMatrix, n);

    return 0;
}

double **Hungarian (int n, double **costMatrix) {
    double **matrixCopy = copyMatrix (costMatrix, n);

    // Step 1. Subtract the smallest entry in each row from all the entries of its row.
    // Stores smallest value of each row in the matrix ...
    double *smallestRow = malloc (n * sizeof (double));
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n;  col++) {
            // Set to smallest row and cols to first value ...
            if (col == 0)
                smallestRow[row] = matrixCopy[row][col];

            // Check if value is lowest in row ...
            if (matrixCopy[row][col] < smallestRow[row])
                smallestRow[row] = matrixCopy[row][col];
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
        printf ("Pre step 5\n");
        printMatrix (matrixCopy, n);

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

        printf ("POST step 5\n");
        printMatrix (matrixCopy, n);

        // Return to Step 3.
        Lines = countZeroLines (matrixCopy, n);
    }
    return matrixCopy;
    
}

// TODO: Prioritise lines that have the most 0s in them first. Create queue??

Lines countZeroLines (double **costMatrix, int n) {
    Lines ret;
    Lines LinesR = newLines (n);
    Lines LinesC = newLines (n);

    int **MatrixC = copyMatrixToInt (costMatrix, n);
    int **MatrixR = copyMatrixToInt (costMatrix, n);
    printIntMatrix (MatrixC, n);
    
    minLinesRow (&LinesR, MatrixR, n);
    minLinesCol (&LinesC, MatrixC, n);

    if (LinesR.nLines == LinesC.nLines) {
        ret = (LinesR.zeroes >= LinesC.zeroes) ? LinesR : LinesC;
    } else {
        ret = (LinesR.nLines >= LinesC.nLines) ? LinesR : LinesC;
    }
    
    for (int i = 0; i < n; i++) {
        if (ret.lineCol[i] != 0) {
            printf ("Line at col %d\n", i);
            ret.nLines++;
        }
        if (ret.lineRow[i] != 0) {
            printf ("Line at row %d\n", i);
            ret.nLines++;
        }
    }
    printf ("row-priority has %d zeroes\n", LinesR.zeroes);
    printf ("col-priority has %d zeroes\n", LinesC.zeroes);
    return ret;

}

Lines newLines (int n) {
    Lines Lines;
    Lines.nLines = 0;
    // Use > 0 to represent line, 0 to represent no line ...
    Lines.lineRow = calloc (n, sizeof (int));
    Lines.lineCol = calloc (n, sizeof (int));
    Lines.zeroes = 0;

    return Lines;
}

void minLinesRow (Lines *Lines, int **Matrix, int n) {
    // Loop until minimum lines have been found ...
    while (1) {
        int *zeroCountRow = calloc (n, sizeof (int));
        int *zeroCountCol = calloc (n, sizeof (int));
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

        // TODO: LOOK AT THIS SECTION ...

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

            Lines->lineRow[maxZeroRow] = 1;
            Lines->zeroes += maxZeroR;
        } else {
            // Check to see if zero has already been drawn through ...
            for (int row = 0; row < n; row++) {
                Matrix[row][maxZeroCol] = -1;
            }
            Lines->lineCol[maxZeroCol] = 1;
            Lines->zeroes += maxZeroC;
        } 

    }
}

void minLinesCol (Lines *Lines, int **Matrix, int n) {
    // Loop until minimum lines have been found ...
    while (1) {
        int *zeroCountRow = calloc (n, sizeof (int));
        int *zeroCountCol = calloc (n, sizeof (int));

        int *altZeroCountRow = calloc (n, sizeof (int));
        int *altZeroCountCol = calloc (n, sizeof (int));

        int zeroes = 0;
        // Count amount of zeroes in each row and column for hungarian algo ...
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (Matrix[row][col] == 0) {
                    zeroCountRow[row]++;
                    zeroCountCol[col]++;
                    zeroes++;
                }

                if (Matrix[row][col] <= 0) {
                    altZeroCountRow[row]++;
                    altZeroCountCol[col]++;
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

        int altMaxZeroR = 0;
        int altMaxZeroC = 0;

        int maxZeroRow;
        int maxZeroCol;

        for (int i = 0; i < n; i++) {
            if (zeroCountRow[i] >= maxZeroR) {
                maxZeroR = zeroCountRow[i];
                altMaxZeroR = altZeroCountRow[i];
                maxZeroRow = i;
            }
            if (zeroCountCol[i] >= maxZeroC) {
                maxZeroC = zeroCountCol[i];
                altMaxZeroC = altZeroCountCol[i];
                maxZeroCol = i;
            }
        }

        // Set zeroes to -1 to not consider them anymore ...
        if (maxZeroC >= maxZeroR) {
            for (int row = 0; row < n; row++)
                Matrix[row][maxZeroCol] = -1;

            Lines->lineCol[maxZeroCol] = 1;
            printf ("col%d +%d\n",maxZeroCol, altMaxZeroC);
            Lines->zeroes += altMaxZeroC;
        } else {
            // Check to see if zero has already been drawn through ...
            for (int col = 0; col < n; col++)
                Matrix[maxZeroRow][col] = -1;

            Lines->lineRow[maxZeroRow] = 1;
            printf ("row%d +%d\n", maxZeroRow, altMaxZeroR);
            Lines->zeroes += altMaxZeroR;
        } 
    }
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

void printMatrix (double **matrix, int n) {
    printf ("============================\n");
    for (int row = 0; row < n; row++){
        for (int col = 0; col < n; col++)
            printf("%5.1f ", matrix[row][col]);
        printf ("\n");
    }
    printf ("============================\n");
}

void printIntMatrix (int **matrix, int n) {
    printf ("============================\n");
    for (int row = 0; row < n; row++){
        for (int col = 0; col < n; col++)
            printf("%5d ", matrix[row][col]);
        printf ("\n");
    }
    printf ("============================\n");
}