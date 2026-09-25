#include "car_lights.h"

#include <math.h>

static float Unit(float value) {
    if (!isfinite(value)) return 1.0f;
    return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
}

void UpdateCarLights(CarLights *lights, float daylight, float shelter,
                     int braking, float seconds) {
    float brightness, step, target;
    if (!lights || !isfinite(seconds) || seconds <= 0.0f) return;
    brightness = Unit(daylight) * Unit(shelter);
    /* A bridge shadow must not flash the headlights. Require continuous
     * darkness; separate thresholds retain the latch around twilight. */
    if (brightness < 0.28f) {
        lights->darkSeconds += seconds;
        if (lights->darkSeconds >= 0.6f) {
            lights->darkSeconds = 0.6f;
            lights->automatic = 1;
        }
    } else {
        lights->darkSeconds = 0;
        if (brightness > 0.36f) lights->automatic = 0;
    }

    target = lights->automatic ? 1.0f : 0.0f;
    step = seconds * 5.0f;
    lights->headlights = Unit(lights->headlights);
    if (lights->headlights < target) {
        lights->headlights += step;
        if (lights->headlights > target) lights->headlights = target;
    } else {
        lights->headlights -= step;
        if (lights->headlights < target) lights->headlights = target;
    }
    lights->tail = 0.06f + lights->headlights * 0.14f;
    /* STOP responds immediately, including while stationary in daylight.
     * A shared tail/stop surface takes max(tail, stop); separate segments
     * use the two values independently. */
    lights->stop = braking ? 1.0f : 0.0f;
}
