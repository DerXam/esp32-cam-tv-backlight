#ifndef TG_CAMERA_CAMERA_H
#define TG_CAMERA_CAMERA_H

#include "types.h"
#include "esp_camera.h"

// Forward declaration to avoid including state.h here
struct AppState;

bool cameraBegin();
camera_fb_t* cameraGrab();
void cameraRelease(camera_fb_t* fb);
RGB samplePixel(const camera_fb_t* fb, Point p, const AppState& s);
void applySensorFromState(const AppState& s);

#endif // TG_CAMERA_CAMERA_H
