#include <math.h>

#include "color.h"

static Color getColor(float hue, float saturation, float brightness) {
    Color c;
    c.a = 255;

    if (saturation == 0) {
        c.r = c.g = c.b = floor(brightness * 255.0f + 0.5f);
    } else {
        float h = (hue - floor(hue)) * 6.0f;
        float f = h - floor(h);
        float p = brightness * (1.0f - saturation);
        float q = brightness * (1.0f - saturation * f);
        float t = brightness * (1.0f - (saturation * (1.0f - f)));
        switch ((int) h) {
            case 0:
                c.r = floor(brightness * 255.0f + 0.5f);
                c.g = floor(t * 255.0f + 0.5f);
                c.b = floor(p * 255.0f + 0.5f);
                break;
            case 1:
                c.r = floor(q * 255.0f + 0.5f);
                c.g = floor(brightness * 255.0f + 0.5f);
                c.b = floor(p * 255.0f + 0.5f);
                break;
            case 2:
                c.r = floor(p * 255.0f + 0.5f);
                c.g = floor(brightness * 255.0f + 0.5f);
                c.b = floor(t * 255.0f + 0.5f);
                break;
            case 3:
                c.r = floor(p * 255.0f + 0.5f);
                c.g = floor(q * 255.0f + 0.5f);
                c.b = floor(brightness * 255.0f + 0.5f);
                break;
            case 4:
                c.r = floor(t * 255.0f + 0.5f);
                c.g = floor(p * 255.0f + 0.5f);
                c.b = floor(brightness * 255.0f + 0.5f);
                break;
            case 5:
                c.r = floor(brightness * 255.0f + 0.5f);
                c.g = floor(p * 255.0f + 0.5f);
                c.b = floor(q * 255.0f + 0.5f);
                break;
        }
    }

    return c;
}

Color getCoordinateColor(float x, float y) {
    float v = x * 2;

    if (y <= 0.95) {
        return getColor(y / 0.95, v < 1 ? v : 1.0, v > 1 ? 2.0-v : 1.0);
    } else {
        return getColor(0, 0, 1.0 - x);
    }
}
