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

#include "main_functions.h"

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "image_provider.h"
#include "model.h"
#include "model_settings.h"
#include "model_utils.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// In order to use optimized tensorflow lite kernels, a signed int8_t quantized
// model is preferred over the legacy unsigned model format. This means that
// throughout this project, input images must be converted from unisgned to
// signed format. The easiest and quickest way to convert from unsigned to
// signed 8-bit integers is to subtract 128 from the unsigned value to get a
// signed value.

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 81 * 1024;
static uint8_t* tensor_arena;  // Maybe we should move this to external

constexpr float kConfidenceThreshold = 0.5;
constexpr float kIoUThreshold = 0.3;
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf(
        "Model provided is schema version %d not equal to supported "
        "version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  if (tensor_arena == NULL) {
    printf("%s: free RAM size: %d, INTERNAL: %d, PSRAM: %d\n", "tag",
           heap_caps_get_free_size(MALLOC_CAP_8BIT),
           heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
           heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM));
    tensor_arena = (uint8_t*)heap_caps_malloc(
        kTensorArenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }

  if (tensor_arena == NULL) {
    printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
    return;
  }

  MicroPrintf("Memory arena allocated\n");
  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<10> micro_op_resolver;
  micro_op_resolver.AddQuantize();
  micro_op_resolver.AddPad();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddLogistic();
  micro_op_resolver.AddMul();
  micro_op_resolver.AddConcatenation();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddAdd();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddStridedSlice();

  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
  printTensorDimensions(input);

  // Initialize Camera
  TfLiteStatus init_status = InitCamera();
  if (init_status != kTfLiteOk) {
    MicroPrintf("InitCamera failed\n");
    return;
  }
}

// The name of this function is important for Arduino compatibility.
void loop() {
  MicroPrintf("Inference loop started\n");
  // Get image from provider.
  if (kTfLiteOk !=
      GetImage(kNumCols, kNumRows, kNumChannels, input->data.uint8)) {
    MicroPrintf("Image capture failed.");
  }

  unsigned detect_time;
  detect_time = esp_timer_get_time();

  MicroPrintf("Invoking the interpreter\n");
  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke()) {
    MicroPrintf("Invoke failed.");
  }

  detect_time = (esp_timer_get_time() - detect_time) / 1000;
  MicroPrintf("Time required for the inference is %u ms", detect_time);

  TfLiteTensor* output = interpreter->output(0);  // CHECK
  //     printTensorDimensions(output);

  std::vector<Prediction> predictions;
  convertOutputToFloat(output, predictions, kCategoryCount);  //<< BUG

  auto nms_predictions = non_maximum_suppression(
      predictions, kConfidenceThreshold, kIoUThreshold, kNumCols, kNumRows);

  // print number of non-maximum suppressed predictions
  MicroPrintf("Number of non-maximum suppressed predictions: %d",
              nms_predictions.size());

  auto detection_classes =
      get_detection_classes(nms_predictions, kConfidenceThreshold);

  for (auto detection_class : detection_classes) {
    MicroPrintf("Detected %s", kCategoryLabels[detection_class]);
  }
  vTaskDelay(1);  // to avoid watchdog trigger
}
