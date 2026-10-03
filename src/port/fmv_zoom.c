#include "fmv_zoom.h"

enum { FMV_PAGE_HEIGHT = 240 };

int FmvZoomCrop(int frameWidth, int frameHeight, int pageY,
                int windowWidth, int windowHeight, FmvZoomRect *out) {
    long long wide, tall;
    if (out == 0 || frameWidth <= 0 || frameHeight <= 0 ||
        frameHeight > FMV_PAGE_HEIGHT || pageY < 0 ||
        windowWidth <= 0 || windowHeight <= 0)
        return 0;
    out->x = 0;
    out->y = pageY + (FMV_PAGE_HEIGHT - frameHeight) / 2;
    out->w = frameWidth;
    out->h = frameHeight;
    /* Compare frameWidth / frameHeight with windowWidth / windowHeight. */
    wide = (long long)frameWidth * windowHeight;
    tall = (long long)frameHeight * windowWidth;
    if (tall > wide) {
        /* A window wider than the movie: keep the width, trim top and bottom. */
        int height = (int)((wide + windowWidth / 2) / windowWidth);
        if (height < 1) height = 1;
        out->y += (frameHeight - height) / 2;
        out->h = height;
    } else if (wide > tall) {
        /* A narrower window: keep the height, trim the sides. */
        int width = (int)((tall + windowHeight / 2) / windowHeight);
        if (width < 1) width = 1;
        out->x = (frameWidth - width) / 2;
        out->w = width;
    }
    return 1;
}
