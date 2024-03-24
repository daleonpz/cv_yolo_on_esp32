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

#ifndef TENSORFLOW_LITE_MICRO_EXAMPLES_PERSON_DETECTION_ESP_APP_CAMERA_ESP_H_
#define TENSORFLOW_LITE_MICRO_EXAMPLES_PERSON_DETECTION_ESP_APP_CAMERA_ESP_H_

#include "sensor.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_system.h"
#include "tensorflow/lite/micro/micro_log.h"
// #include "tensorflow/lite/c/common.h"

#define CAMERA_FRAME_SIZE    FRAMESIZE_96X96
#define CAMERA_PIXEL_FORMAT PIXFORMAT_RGB565 // gives better quality images

#define CAMERA_MODULE_NAME "ESP-EYE"
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 4
#define CAMERA_PIN_SIOD 18
#define CAMERA_PIN_SIOC 23

#define CAMERA_PIN_D7 36
#define CAMERA_PIN_D6 37
#define CAMERA_PIN_D5 38
#define CAMERA_PIN_D4 39
#define CAMERA_PIN_D3 35
#define CAMERA_PIN_D2 14
#define CAMERA_PIN_D1 13
#define CAMERA_PIN_D0 34
#define CAMERA_PIN_VSYNC 5
#define CAMERA_PIN_HREF 27
#define CAMERA_PIN_PCLK 25

#define XCLK_FREQ_HZ 15000000

#ifdef __cplusplus
extern "C" {
#endif

int app_camera_init();

#ifdef __cplusplus
}
#endif

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_PERSON_DETECTION_ESP_APP_CAMERA_ESP_H_
