#ifndef HOMEWORK_H1
#define HOMEWORK_H1

#define FORMAT '5'
#define MINVAL 0
#define MAXVAL 255

typedef struct {
    unsigned int width;
    unsigned char **matrix;
}image;

void initialize(image *im);
void render(image *im);
void writeData(const char * fileName, image *img);

#endif /* HOMEWORK_H1 */