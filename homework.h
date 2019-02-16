#ifndef HOMEWORK_H
#define HOMEWORK_H

#define CHANNELS_NO 3
#define GRAYSCALE '5'
#define COLOR '6'
#define KERNEL_HEIGHT 3
#define KERNEL_WIDTH 3
#define KERNEL_MEAN 16
extern const unsigned char GAUSSIAN_KERNEL[KERNEL_HEIGHT][KERNEL_WIDTH];

// Pixel color - color[0] = rosu, color[1] = verde, color[2] = albastru
// Pixel gri   - color[0] = gri, color[1], color[2] = 0
typedef struct {
    unsigned char color[CHANNELS_NO];
}pixel;

typedef struct {
    // '5' pentru poza alb-negru, '6' pentru poza color
    char format;
    unsigned int width;
    unsigned int height;
    unsigned char maxval;
    pixel **matrix;
}image;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

#endif /* HOMEWORK_H */