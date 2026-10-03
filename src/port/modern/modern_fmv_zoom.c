#include "modern_fmv_zoom.h"

#include <psyz/video.h>

#include "../fmv_zoom.h"
#include "../runtime_config.h"

/* Movie frames are at most 320 wide. */
enum { FMV_ZOOM_MAX_WIDTH = 320 };

/* Exactly the size of the crop: the presenter samples the whole texture. */
static SDL_GPUTexture *s_texture;
static int s_textureWidth, s_textureHeight;

int ModernFmvZoomPresent(SDL_GPUDevice *device, SDL_Window *window,
                         PsyzPresentSourceInfo *info) {
    static int enabled = -1;
    PsyzVideoCaptureState state;
    FmvZoomRect crop;
    int frameWidth, frameHeight, windowWidth, windowHeight;
    SDL_GPUTexture *vram;
    SDL_GPUCommandBuffer *cmd;

    if (enabled < 0) enabled = RuntimeConfigEnabled("video.fmv_zoom");
    if (!enabled || device == NULL || window == NULL || info == NULL) return 0;
    if (!HostFmvFrameSize(&frameWidth, &frameHeight) ||
        frameWidth > FMV_ZOOM_MAX_WIDTH ||
        !SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight) ||
        Psyz_VideoGetCaptureState(&state) != 0 ||
        !FmvZoomCrop(frameWidth, frameHeight, state.display_y,
                     windowWidth, windowHeight, &crop))
        return 0;
    if (s_texture != NULL && (s_textureWidth != crop.w || s_textureHeight != crop.h))
        ModernFmvZoomRelease(device);
    if (s_texture == NULL) {
        const SDL_GPUTextureCreateInfo create = {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER |
                     SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
            .width = (Uint32)crop.w,
            .height = (Uint32)crop.h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
        };
        s_texture = SDL_CreateGPUTexture(device, &create);
        if (s_texture == NULL) return 0;
        s_textureWidth = crop.w;
        s_textureHeight = crop.h;
    }
    vram = Psyz_VideoSnapshotVramTexture_SDL3GPU();
    if (vram == NULL) return 0;
    cmd = SDL_AcquireGPUCommandBuffer(device);
    if (cmd == NULL) return 0;
    {
        const SDL_GPUBlitInfo blit = {
            .source = {.texture = vram,
                       .x = (Uint32)(state.display_x + crop.x),
                       .y = (Uint32)crop.y,
                       .w = (Uint32)crop.w,
                       .h = (Uint32)crop.h},
            .destination = {.texture = s_texture,
                            .w = (Uint32)crop.w,
                            .h = (Uint32)crop.h},
            .load_op = SDL_GPU_LOADOP_DONT_CARE,
            .filter = SDL_GPU_FILTER_NEAREST,
        };
        SDL_BlitGPUTexture(cmd, &blit);
    }
    if (!SDL_SubmitGPUCommandBuffer(cmd)) return 0;
    info->texture = s_texture;
    info->w = (Uint32)crop.w;
    info->h = (Uint32)crop.h;
    info->aspect = (float)crop.w / (float)crop.h;
    return 1;
}

void ModernFmvZoomRelease(SDL_GPUDevice *device) {
    if (s_texture != NULL && device != NULL)
        SDL_ReleaseGPUTexture(device, s_texture);
    s_texture = NULL;
}
