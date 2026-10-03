#ifndef RAGE_MODERN_FMV_ZOOM_H
#define RAGE_MODERN_FMV_ZOOM_H

#include <SDL3/SDL.h>
#include <psyz/present_sdl3_gpu.h>

/* With video.fmv_zoom, presents only the part of a playing movie that fills
 * the window (../fmv_zoom.h). Returns 0, leaving `info` alone, when no movie
 * plays, the setting is off or the crop cannot be made. */
int ModernFmvZoomPresent(SDL_GPUDevice *device, SDL_Window *window,
                         PsyzPresentSourceInfo *info);
void ModernFmvZoomRelease(SDL_GPUDevice *device);

#endif
