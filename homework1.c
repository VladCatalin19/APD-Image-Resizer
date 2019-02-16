#include "homework1.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

int num_threads;
int resolution;

struct args {
    int thread_id;
    image *im;
};

// Aloca memorie pentru matricea de pixeli
void alloc_matrix(unsigned char ***matrix, unsigned int width) {
    *matrix = (unsigned char **) malloc(width * sizeof(unsigned char *));

    for (int i = 0; i < width; ++i) {
        (*matrix)[i] = (unsigned char *) malloc (width * sizeof(unsigned char));
    }
}

// Dezaloca memoria matricei de pixeli
void free_matrix(unsigned char **matrix, unsigned int width) {
    for (int i = 0; i < width; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

void initialize(image *im) {
    im->width = (unsigned int) resolution;
    alloc_matrix(&(im->matrix), im->width);
}

// Calculeaza distanta de la centrul fiecarui pixel la dreapta si decide daca
// pixelul respectiv trebuie sa fie alb sau negru
void* thread_function(void *input) {

    struct args* arguments = (struct args*) input;
    int thread_id = arguments->thread_id;
    image *im = arguments->im;

    int step = ceil((float)(im->width) / (float)(num_threads));
    int start = thread_id * step ;
    int end = MIN((thread_id + 1) * step, im->width);

    double devisor = sqrt(5);
    double new_resolution = 100.0 / (double)(resolution);

    for (int i = start; i < end; ++i) {
        for (int j = 0; j < im->width; ++j) {
            double x = ((double)(i) + 0.5) * new_resolution;
            double y = ((double)(j) + 0.5) * new_resolution;
            double d = (double)(abs(-1.0 * x + 2.0 * y)) / devisor;

            if (d <= 3.0f) {
                im->matrix[j][i] = MINVAL;      // Negru
            } else {
                im->matrix[j][i] = MAXVAL;      // Alb
            }
        }
    }
    return NULL;
}

void render(image *im) {
    pthread_t tid[num_threads];
    int thread_id[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        thread_id[i] = i;
    }

    struct args arguments[num_threads];

    // Calculare imagine out
    for (int i = 0; i < num_threads; ++i) {
        
        arguments[i].thread_id = thread_id[i];
        arguments[i].im = im;

        pthread_create(&(tid[i]), NULL, thread_function, (void *)(&arguments[i]));
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(tid[i], NULL);
    }
}

void writeData(const char * fileName, image *img) {
    FILE *output = fopen(fileName, "wb");

    fprintf(output, "P%c\n", FORMAT);
    fprintf(output, "%d %d\n", img->width, img->width);
    fprintf(output, "%hhu\n", MAXVAL);

    unsigned char buffer[img->width];

    for (int i = img->width - 1; i >= 0; --i) {
        for (int j = 0; j < img->width; ++j) {
            buffer[j] = img->matrix[i][j];
        }

        fwrite(buffer, sizeof(unsigned char), img->width, output);
    }

    free_matrix(img->matrix, img->width);
    fclose(output);
}
