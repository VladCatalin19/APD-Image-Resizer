#include "homework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define BUFFLEN 256

#define MIN(a,b) (((a)<(b))?(a):(b))

int num_threads;
int resize_factor;

// Structura care incapsuleaza argumentele care vor fi trimise unui fir de
// executie
struct args{
    int thread_id;
    void (*pixel_mean)(image*, int, int, pixel*);
    image *in;
    image *out;
};

// Matrice pentru resize_factor 3
const unsigned char GAUSSIAN_KERNEL[KERNEL_HEIGHT][KERNEL_WIDTH] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};

// Aloca memorie pentru o matrice de pixeli de dimensiune height x width
void alloc_matrix(pixel ***matrix, int height, int width) {
    *matrix = (pixel **) malloc(height * sizeof(pixel *));

    for (int i = 0; i < height; ++i) {
        (*matrix)[i] = (pixel *) calloc(width, sizeof(pixel));
    }
}

// Dezaloca memoria unei matrice
void free_matrix(pixel** matrix, int height, int width) {
    for (int i = 0; i < height; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

// Citeste matricea de pixeli pentru o imagine alb-negru din fisierul de intrare
void read_gray_matrix(image *img, FILE* input) {
    unsigned char buffer[img->width];

    for (int i = 0; i < img->height; ++i) {
        fread(buffer, sizeof(unsigned char), img->width, input);

        for (int j = 0; j < img->width; ++j) {
            img->matrix[i][j].color[0] = buffer[j];
        }
    }
}

// Citeste matricea de pixeli pentru o imagine color din fisierul de intrare
void read_color_matrix(image *img, FILE *input) {
    unsigned char buffer[img->width* CHANNELS_NO];

    for (int i = 0; i < img->height; ++i) {
        fread(buffer, sizeof(unsigned char), img->width * CHANNELS_NO, input);
        for (int j = 0; j < img->width; ++j) {
            for (int k = 0; k < CHANNELS_NO; ++k) {
                img->matrix[i][j].color[k] = buffer[j * CHANNELS_NO + k];
            }
        }
    }
}

void readInput(const char * fileName, image *img) {
    FILE* input = fopen(fileName, "rb");

    fscanf(input, "P%c\n", &(img->format));
    fscanf(input, "%d %d\n", &(img->width), &(img->height));
    fscanf(input, "%hhu\n", &(img->maxval));

    alloc_matrix(&(img->matrix), img->height, img->width);

    if (img->format == GRAYSCALE) {
        read_gray_matrix(img, input);
    } else {
        read_color_matrix(img, input);
    }

    fclose(input);
}


// Scrie matricea de pixeli pentru o imagine alb-negru in fisierul de intrare
void write_gray_matrix(image *img, FILE* output) {
    unsigned char buffer[img->width];

    for (int i = 0; i < img->height; ++i) {
        for (int j = 0; j < img->width; ++j) {
            buffer[j] = img->matrix[i][j].color[0];
        }

        fwrite(buffer, sizeof(unsigned char), img->width, output);
    }
}

// Scrie matricea de pixeli pentru o imagine color in fisierul de intrare
void write_color_matrix(image *img, FILE* output) {
    unsigned char buffer[img->width * CHANNELS_NO];

    for (int i = 0; i < img->height; ++i) {
        for (int j = 0; j < img->width; ++j) {
            for (int k = 0; k < CHANNELS_NO; ++k) {
                buffer[j * CHANNELS_NO + k] = img->matrix[i][j].color[k];
            }
        }

        fwrite(buffer, sizeof(unsigned char), img->width * CHANNELS_NO, output);
    }
}

void writeData(const char * fileName, image *img) {
    FILE *output = fopen(fileName, "wb");

    fprintf(output, "P%c\n", img->format);
    fprintf(output, "%d %d\n", img->width, img->height);
    fprintf(output, "%hhu\n", img->maxval);

    if (img->format == GRAYSCALE) {
        write_gray_matrix(img, output);
    } else {
        write_color_matrix(img, output);
    }

    free_matrix(img->matrix, img->height, img->width);
    fclose(output);
}


// Calculeaza culoarea pixelului de pe pozitia (x, y) din imaginea alb-negru
// ce va rezulta dupa scalare
void pixel_mean_gray(image *in, int x, int y, pixel *px) {
    unsigned short mean = 0;

    for (int i = 0; i < resize_factor; ++i) {
        for(int j = 0; j < resize_factor; ++j) {
            mean += in->matrix[x * resize_factor + i][y * resize_factor + j]
                    .color[0];
        }
    }

    mean /= resize_factor * resize_factor;

    memset(px, 0, sizeof(pixel));
    px->color[0] = (unsigned char)mean;

}

// Calculeaza culoarea pixelului de pe pozitia (x, y) din imaginea color
// ce va rezulta dupa scalare
void pixel_mean_color(image *in, int x, int y, pixel *px) {
    unsigned short mean[CHANNELS_NO];
    memset(mean, 0, CHANNELS_NO * sizeof(unsigned short));

    for (int i = 0; i < resize_factor; ++i) {
        for(int j = 0; j < resize_factor; ++j) {
            for (int k = 0; k < CHANNELS_NO; ++k) {
                mean[k] += in->matrix[x * resize_factor + i][y * resize_factor + j]
                           .color[k];
            }
        }
    }

    for (int k = 0; k < CHANNELS_NO; ++k) {
        mean[k] /= resize_factor * resize_factor;
    }

    memset(px, 0, sizeof(pixel));
    for (int k = 0; k < CHANNELS_NO; ++k) {
        px->color[k] = (unsigned char)mean[k];
    }
}

// Calculeaza culoarea pixelului de pe pozitia (x, y) din imaginea alb-negru
// ce va rezulta dupa aplicarea filtrului Gaussian
void pixel_gauss_gray(image *in, int x, int y, pixel *px) {
    unsigned short mean = 0;

    for (int i = 0; i < resize_factor; ++i) {
        for(int j = 0; j < resize_factor; ++j) {
            mean += in->matrix[x * resize_factor + i][y * resize_factor + j]
                    .color[0] * GAUSSIAN_KERNEL[i][j];
        }
    }

    mean /= KERNEL_MEAN;

    memset(px, 0, sizeof(pixel));
    px->color[0] = (unsigned char)mean;
}

// Calculeaza culoarea pixelului de pe pozitia (x, y) din imaginea color
// ce va rezulta dupa aplicarea filtrului Gaussian
void pixel_gauss_color(image *in, int x, int y, pixel *px) {
    unsigned short mean[CHANNELS_NO];
    memset(mean, 0, CHANNELS_NO * sizeof(unsigned short));

    for (int i = 0; i < resize_factor; ++i) {
        for(int j = 0; j < resize_factor; ++j) {
            for (int k = 0; k < CHANNELS_NO; ++k) {
                mean[k] += in->matrix[x * resize_factor + i][y * resize_factor + j]
                           .color[k] * GAUSSIAN_KERNEL[i][j];
            }
        }
    }

    for (int k = 0; k < CHANNELS_NO; ++k) {
        mean[k] /= KERNEL_MEAN;
    }

    memset(px, 0, sizeof(pixel));
    for (int k = 0; k < CHANNELS_NO; ++k) {
        px->color[k] = (unsigned char)mean[k];
    }
}

// Aplica filtrul respectiv pe imaginea de intrare si creeaza matricea de
// pixeli a imaginii rezultate
void* thread_function(void *input) {
    struct args *argument =  (struct args*)input;
    int thread_id = argument->thread_id;
    void (*pixel_mean)(image*, int, int, pixel*) = argument->pixel_mean;
    image *in = argument->in;
    image *out = argument->out;

    int start_height;
    int end_height;
    int start_width;
    int end_width;
    int step;

    // Se paralelizeaza dupa dimensiunea cea mai mare
    if (out->height < out->width) {
        start_height = 0;
        end_height = out->height;

        step = ceil((float)(out->width) / (float)(num_threads));
        start_width = thread_id * step;
        end_width = MIN((thread_id + 1) * step, out->width);
    } else {
        step = ceil((float)(out->height) / (float)(num_threads));
        start_height = thread_id * step;
        end_height = MIN((thread_id + 1) * step, out->height);

        start_width = 0;
        end_width = out->width;
    }

    for (int i = start_height; i < end_height; ++i) {
        for (int j = start_width; j < end_width; ++j) {
            pixel px;
            (*pixel_mean)(in, i, j, &px);

            for (int k = 0; k < CHANNELS_NO; ++k) {
                out->matrix[i][j].color[k] = px.color[k];
            }
        }
    }

    return NULL;
}


void resize(image *in, image * out) { 

    // Creare headere imagine out
    out->format = in->format;
    out->width = in->width / resize_factor;
    out->height = in->height / resize_factor;
    out->maxval = in->maxval;
    alloc_matrix(&(out->matrix), out->height, out->width);

    pthread_t tid[num_threads];
    int thread_id[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        thread_id[i] = i;
    }

    void (*pixel_mean)(image*, int, int, pixel*) = NULL;
    if (resize_factor % 2 == 0) {
        if (out->format == GRAYSCALE) {
            pixel_mean = pixel_mean_gray;
        } else {
            pixel_mean = pixel_mean_color;
        }

    } else {
        if (out->format == GRAYSCALE) {
            pixel_mean = pixel_gauss_gray;
        } else {
            pixel_mean = pixel_gauss_color;
        }
    }

    struct args arguments[num_threads];

    // Calculare imagine out
    for (int i = 0; i < num_threads; ++i) {
        
        arguments[i].thread_id = thread_id[i];
        arguments[i].pixel_mean = pixel_mean;
        arguments[i].in = in;
        arguments[i].out = out;

        pthread_create(&(tid[i]), NULL, thread_function, (void *)(&arguments[i]));
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(tid[i], NULL);
    }

    free_matrix(in->matrix, in->height, in->width);
}