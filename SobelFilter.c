#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define HI(num)	(((num) & 0x0000FF00) << 8)
#define LO(num)	((num) & 0x000000FF)

typedef struct _PGMImage
{
	int height;
	int width;
	int maxGray;
	int** data;
}PGMImage;


PGMImage* readPGM(const char* file_name);
PGMImage* initSobelImage(PGMImage* pgm);
PGMImage* sobelEdgeDetection(PGMImage* pgm);
void paddingCorners(PGMImage* newPgm);
void normalizeColors(PGMImage* pgm, int max_x, int max_y);
int** allocateMatrix(int height, int width);
void freeMatrix(int** data, int height);
void printMatrix(PGMImage* pgm);
void SkipComments(FILE* fp);
void writePGM(const char* filename, const PGMImage* newPgm);

int main() {

    char filename[100];
    printf("Dosyanin adini giriniz. (Ornek : image.pgm) : ");
    scanf("%s", filename);
    PGMImage* pgm = readPGM(filename);
    PGMImage* newPgm = sobelEdgeDetection(pgm);
	return 0;
}

PGMImage* sobelEdgeDetection(PGMImage* pgm) {

    PGMImage* newPgm = initSobelImage(pgm);
    PGMImage* newPgmX = initSobelImage(pgm);
    PGMImage* newPgmY = initSobelImage(pgm);

    int Gx[3][3] = { {-1, 0, 1 },
                     {-2, 0, 2 },
                     {-1, 0, 1 } };
    int Gy[3][3] = { {1, 2, 1 },
                     {0, 0, 0 },
                     {-1, -2, -1 }};

    int op_x, op_y, new_value;
    int max_x = 255, max_y = 255, max_all = 255, min_x = 0, min_y = 0, min_all = 0; 

    for (int i = 1; i < pgm->height-1; i++)
    {
        for (int j = 1; j < pgm->width-1; j++)
        {
            op_x = applyMask(Gx, pgm, i, j);
            op_y = applyMask(Gy, pgm, i, j);
            newPgmX->data[i][j] = op_x;
            newPgmY->data[i][j] = op_y;
            new_value = round(sqrt(pow(op_x, 2) + pow(op_y, 2)));
            newPgm->data[i][j] = new_value;

            if (max_x < op_x) {
                max_x = op_x;
            }
            else if(min_x > op_x){
                min_x = op_x;
            }

            if (max_y < op_y) {
                max_y = op_y;
            }
            else if (min_y > op_y) {
                min_y = op_y;
            }
  
            if (max_all < new_value) {
                max_all = new_value;
            }
            else if (min_all > new_value) {
                min_all = new_value;
            }

        }
    }
    paddingCorners(newPgm);
    paddingCorners(newPgmX);
    paddingCorners(newPgmY);

    normalizeColors(newPgm, max_all, min_all);
    normalizeColors(newPgmX, max_x, min_x);
    normalizeColors(newPgmY, max_x, min_y);

    writePGM("output_x.pgm", newPgmX);
    writePGM("output_y.pgm", newPgmY);
    writePGM("output_all.pgm", newPgm);
    return newPgm;
}

PGMImage* initSobelImage(PGMImage* pgm) {

    PGMImage* newPgm = (PGMImage*)malloc(sizeof(PGMImage));
    newPgm->height = pgm->height;
    newPgm->width = pgm->width;
    newPgm->maxGray = pgm->maxGray;
    newPgm->data = allocateMatrix(newPgm->height, newPgm->width);
    return newPgm;
}

void normalizeColors(PGMImage* pgm, int max, int min) {

    int range = max - min, newValue;;

    for (int i = 0; i < pgm->height; i++)
    {
        for  (int j = 0; j< pgm->width; j++)
        {
            newValue = round(((pgm->data[i][j]) - min) * 255 / range);
            pgm->data[i][j] = newValue;
        }
    }
}

void paddingCorners(PGMImage* newPgm) {

    for (int i = 0; i < newPgm->width; i++)
    {
        newPgm->data[0][i] = 0;
        newPgm->data[(newPgm->height) - 1][i] = 0;
    }
    for (int i = 0; i < newPgm->height; i++)
    {
        newPgm->data[i][0] = 0;
        newPgm->data[i][(newPgm->width) - 1] = 0;
    }
}

void writePGM(const char* filename, const PGMImage* newPgm)
{
    FILE* pgmFile;
    int i, j;
    int hi, lo;

    pgmFile = fopen(filename, "wb");
    if (pgmFile == NULL) {
        perror("cannot open file to write");
        exit(EXIT_FAILURE);
    }

    fprintf(pgmFile, "P5 ");
    fprintf(pgmFile, "%d %d ", newPgm -> width, newPgm->height);
    fprintf(pgmFile, "%d ", newPgm->maxGray);

    if (newPgm->maxGray > 255) {
        for (i = 0; i < newPgm->height; ++i) {
            for (j = 0; j < newPgm->width; ++j) {
                hi = HI(newPgm->data[i][j]);
                lo = LO(newPgm->data[i][j]);
                fputc(hi, pgmFile);
                fputc(lo, pgmFile);
            }
        }
    }
    else {
        for (i = 0; i < newPgm->height; ++i) {
            for (j = 0; j < newPgm->width; ++j) {
                lo = LO(newPgm->data[i][j]);
                fputc(lo, pgmFile);
            }
        }
    }

    freeMatrix(newPgm->data, newPgm->height);
    free(newPgm);
    fclose(pgmFile);
}

int applyMask(int mask[3][3], PGMImage* pgm, int row, int col) {

    int total = 0;
    for (int i = row-1;  i <= row+1; i++)
    {
        for (int j = col-1; j <= col+1; j++)
        {
            total += (pgm->data[i][j]) * mask[i - row + 1][j - col + 1];
        }
    }
    return total;
}

PGMImage* readPGM(const char* file_name)
{
    FILE* pgmFile;
    PGMImage* image = (PGMImage*)malloc(sizeof(PGMImage));
    char version[3];
    int i, j;
    int lo, hi;
    pgmFile = fopen(file_name, "rb+");
    if (pgmFile == NULL) {
        perror("Cannot open file to read");
        exit(EXIT_FAILURE);
    }
    fgets(version, sizeof(version), pgmFile);
    
    SkipComments(pgmFile);
    fscanf(pgmFile, "%d", &image->width);
    SkipComments(pgmFile);
    fscanf(pgmFile, "%d", &image->height);
    SkipComments(pgmFile);
    fscanf(pgmFile, "%d", &image->maxGray);
    fgetc(pgmFile);
    printf("%d %d %d %s", image->width, image->height, image->maxGray, version);
    
    if (!strcmp(version, "P5")) {
        image->data = allocateMatrix(image->height, image->width);     
        if (image->maxGray > 255) {
            for (i = 0; i < image->height; ++i) {
                for (j = 0; j < image->width; ++j) {
                    hi = fgetc(pgmFile);
                    lo = fgetc(pgmFile);
                    image->data[i][j] = (hi << 8) + lo;
                }
            }
        }
        else {
            for (i = 0; i < image->height; ++i) {
                for (j = 0; j < image->width; ++j) {
                    lo = fgetc(pgmFile);
                    image->data[i][j] = lo;
                }
            }
        }             
    }
    else if (!strcmp(version, "P2")) {
        image->data = allocateMatrix(image->height, image->width);
        if (image->maxGray > 255) {
            for (i = 0; i < image->height; ++i) {
                for (j = 0; j < image->width; ++j) {
                    fscanf(pgmFile, "%d", &hi);
                    fscanf(pgmFile, "%d", &lo);
                    image->data[i][j] = (hi << 8) + lo;
                }
            }
        }
        else {
            for (i = 0; i < image->height; ++i) {
                for (j = 0; j < image->width; ++j) {
                    fscanf(pgmFile, "%d", &lo);
                    image->data[i][j] = lo;
                }
            }
        }

        fclose(pgmFile);
    }
    
    return image;
}

void printMatrix(PGMImage* pgm) {

    for (int i = 0; i < pgm->height; i++)
    {
        for (int j = 0; j < pgm->width; j++)
        {
            printf(" %d ", pgm->data[i][j]);
        }
        printf("\n");
    }
}
void SkipComments(FILE* fp)
{
    int ch;
    char line[100];
    while ((ch = fgetc(fp)) != EOF && isspace(ch)) {
        ;
    }

    if (ch == '#') {
        fgets(line, sizeof(line), fp);
        SkipComments(fp);
    }
    else {
        fseek(fp, -1, SEEK_CUR);
    }
}

int **allocateMatrix(int height, int width) 
{
	int** data = (int**)malloc(height * sizeof(int*));
	if (!data) {
		printf("Could not allocate.");
	}

	for (int i = 0; i <	height; i++)
	{
		data[i] = (int*)malloc(width * sizeof(int));
		if (!data[i]) {
			printf("Could not allocate.");
			return 0;
		}
	}
	return data;
}
void freeMatrix(int** data, int height)
{
	int i;

	for (i = 0; i < height; ++i) {
		free(data[i]);
	}
	free(data);
}

