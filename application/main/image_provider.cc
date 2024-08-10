
/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "esp_log.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "spi_flash_mmap.h"
#include "app_camera_esp.h"
#include "esp_camera.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "image_provider.h"
#include "model_settings.h"
#include "model_utils.h"

static const char *TAG = "app_camera";

static uint16_t *display_buf;  // buffer to hold data to be sent to display

// Get the camera module ready
TfLiteStatus InitCamera() {
  int ret = app_camera_init();
  if (ret != 0) {
    MicroPrintf("Camera init failed\n");
    return kTfLiteError;
  }
  MicroPrintf("Camera Initialized\n");
  return kTfLiteOk;
}

void *image_provider_get_display_buf() { return (void *)display_buf; }

TfLiteStatus GetImage(int image_width, int image_height, int channels,
                      uint8_t *image_data) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    ESP_LOGE(TAG, "Camera capture failed");
    return kTfLiteError;
  }

  // Convert RGB565 to RGB888
  convert_rgb565_to_rgb888((uint8_t *)fb->buf, image_data, image_width,
                           image_height);

  /* here the esp camera can give you grayscale image directly */
  esp_camera_fb_return(fb);
  return kTfLiteOk;
}
