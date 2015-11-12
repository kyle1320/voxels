#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

GLuint loadTextureBMP(const char * texture_file_path) {
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int width, height;
    unsigned int imageSize;
    unsigned char *data;

    GLuint textureID;

    FILE * file = fopen(texture_file_path, "rb");

    if (!file) {
        printf("Error loading %s: Image could not be opened\n", texture_file_path);
        return 0;
    }

    if (fread(header, sizeof(char), 54, file) != 54 ||
        header[0] != 'B' || header[1] != 'M' ) { // make sure the header is correct

        printf("Error loading %s: Not a correct BMP file\n", texture_file_path);
        return 0;
    }

    dataPos    =  *(int*)&(header[0x0A]);
    imageSize  =  *(int*)&(header[0x22]);
    width      =  *(int*)&(header[0x12]);
    height     = -*(int*)&(header[0x16]);

    if (imageSize == 0)
        imageSize = width * height * 4;

    if (dataPos == 0)
        dataPos = 54;

    data = malloc(imageSize * sizeof(unsigned char));
    fread(data, sizeof(unsigned char), imageSize, file);

    fclose(file);

    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

    free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return textureID;
}
