#ifndef COLOR_H_
#define COLOR_H_

typedef union Color_U {
    struct {
        unsigned int r:8;
        unsigned int g:8;
        unsigned int b:8;
        unsigned int a:8;
    };

    unsigned int all;
} Color;

Color getCoordinateColor(float x, float y);

#endif
