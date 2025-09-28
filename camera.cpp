#include "camera.h"
#include "config.h"
#include "state.h"

bool cameraBegin(){
  camera_config_t c;
  c.ledc_channel = LEDC_CHANNEL_0;
  c.ledc_timer   = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM; c.pin_d1 = Y3_GPIO_NUM; c.pin_d2 = Y4_GPIO_NUM; c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM; c.pin_d5 = Y7_GPIO_NUM; c.pin_d6 = Y8_GPIO_NUM; c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM; c.pin_pclk = PCLK_GPIO_NUM;
  c.pin_vsync = VSYNC_GPIO_NUM; c.pin_href = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM; c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM; c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 10'000'000;
  c.pixel_format = PIXFORMAT_YUV422;
  c.frame_size = FRAMESIZE_QQVGA; // 160x120
  c.fb_count = 1;
  return esp_camera_init(&c) == ESP_OK;
}

camera_fb_t* cameraGrab(){ return esp_camera_fb_get(); }
void cameraRelease(camera_fb_t* fb){ if (fb) esp_camera_fb_return(fb); }

static RGB correctedYuv(uint8_t Y,uint8_t U,uint8_t V,int px,int py,int W,int H,const AppState& s){
  float cx = (W - 1) * s.hotspotX, cy = (H - 1) * s.hotspotY;
  float dx = px - cx, dy = py - cy;
  float r = sqrtf(dx*dx + dy*dy) / W;
  float fac = 1.0f + s.vignetteStrength * powf(r, s.vignetteExponent);
  int Yc = constrain(int(Y * fac), 0, 255);

  int rt = Yc + 1.370705f * (V - 128);
  int gt = Yc - 0.698001f * (V - 128) - 0.337633f * (U - 128);
  int bt = Yc + 1.732446f * (U - 128);

  float rc = s.colorMatrix[0]*rt + s.colorMatrix[1]*gt + s.colorMatrix[2]*bt;
  float gc = s.colorMatrix[3]*rt + s.colorMatrix[4]*gt + s.colorMatrix[5]*bt;
  float bc = s.colorMatrix[6]*rt + s.colorMatrix[7]*gt + s.colorMatrix[8]*bt;
  return clampRGB(int(rc), int(gc), int(bc));
}

RGB samplePixel(const camera_fb_t* fb, Point xy, const AppState& s){
  int px = xy.x, py = xy.y;
  px = constrain(px, 0, (int)fb->width - 1);
  py = constrain(py, 0, (int)fb->height - 1);

  const uint8_t* buf = fb->buf;
  const int W = fb->width, H = fb->height;
  int pairIndex = (py * W + px) >> 1;
  int off = pairIndex * 4;
  bool second = px & 1;
  uint8_t Y = buf[off + (second ? 2 : 0)];
  uint8_t U = buf[off + 1];
  uint8_t V = buf[off + 3];
  return correctedYuv(Y, U, V, px, py, W, H, s);
}

void applySensorFromState(const AppState& s){
  sensor_t* sens = esp_camera_sensor_get();
  if (!sens) return;

  // Manual control
  sens->set_whitebal(sens, 0);
  sens->set_awb_gain(sens, 0);
  sens->set_exposure_ctrl(sens, 0);
  sens->set_aec2(sens, 0);
  sens->set_gain_ctrl(sens, 0);

  sens->set_agc_gain(sens, s.camGain);
  sens->set_aec_value(sens, s.camBrightness);
  sens->set_ae_level(sens, 0);
  sens->set_wb_mode(sens, 3);
  sens->set_gainceiling(sens, (gainceiling_t)0);
  sens->set_contrast(sens, s.camContrast);
}
