#include <stdio.h>

#include "fmv_zoom.h"

static int s_failures;

static void Expect(const char *name, int ok) {
    if (!ok) {
        printf("%s\n", name);
        s_failures++;
    }
}

static int Is(const FmvZoomRect *rect, int x, int y, int w, int h) {
    return rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

int main(void) {
    FmvZoomRect rect;
    /* The 320x192 opening movie sits 24 lines down its page. A 16:9 window
     * keeps all 320 columns and the middle 180 lines. */
    Expect("opening movie on 16:9",
           FmvZoomCrop(320, 192, 0, 1920, 1080, &rect) && Is(&rect, 0, 30, 320, 180));
    Expect("second display page",
           FmvZoomCrop(320, 192, 240, 1920, 1080, &rect) && Is(&rect, 0, 270, 320, 180));
    /* 5:3 is the movie's own shape: nothing to trim beyond the bands. */
    Expect("opening movie on 5:3",
           FmvZoomCrop(320, 192, 0, 1000, 600, &rect) && Is(&rect, 0, 24, 320, 192));
    /* On 4:3 the bands go and the sides are trimmed instead. */
    Expect("opening movie on 4:3",
           FmvZoomCrop(320, 192, 0, 1024, 768, &rect) && Is(&rect, 32, 24, 256, 192));
    /* The full-page ending on 16:9 loses 30 lines at the top and bottom. */
    Expect("ending on 16:9",
           FmvZoomCrop(320, 240, 0, 1280, 720, &rect) && Is(&rect, 0, 30, 320, 180));
    /* An ultrawide window still leaves at least one line. */
    Expect("very wide window",
           FmvZoomCrop(320, 192, 0, 100000, 1, &rect) && rect.h == 1);
    Expect("rejects a taller frame than a page", !FmvZoomCrop(320, 256, 0, 1920, 1080, &rect));
    Expect("rejects an empty window", !FmvZoomCrop(320, 192, 0, 0, 1080, &rect));
    Expect("rejects no output", !FmvZoomCrop(320, 192, 0, 1920, 1080, NULL));
    return s_failures != 0;
}
