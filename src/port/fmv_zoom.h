#ifndef RAGE_FMV_ZOOM_H
#define RAGE_FMV_ZOOM_H

/* Movies are decoded into both 240-line display pages, centred vertically:
 * the opening and class movies are 320x192, the ending fills 320x240. With
 * video.fmv_zoom the presenter shows only the part of the movie that fills
 * the window, instead of the whole 4:3 page with its black bands (#29). */

typedef struct FmvZoomRect {
    int x, y, w, h;
} FmvZoomRect;

/* The size of the movie frame on screen, while one is playing (fmv_host.c). */
int HostFmvFrameSize(int *width, int *height);

/* The part of a `frameWidth` x `frameHeight` movie, centred in a 240-line page
 * that starts at `pageY`, that fills a `windowWidth` x `windowHeight` window.
 * PS1 pixels at 320x240 on a 4:3 screen are square, so the crop keeps the
 * window's shape without stretching. Returns 0 for sizes it cannot use. */
int FmvZoomCrop(int frameWidth, int frameHeight, int pageY,
                int windowWidth, int windowHeight, FmvZoomRect *out);

#endif
