#ifndef __MODEL_UTILS_H__
#define __MODEL_UTILS_H__

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#ifndef UNIT_TESTING
#include "tensorflow/lite/micro/micro_interpreter.h"
#endif  // UNIT_TESTING

struct Prediction {
  float x;
  float y;
  float width;
  float height;
  float confidence;
  std::vector<float> class_confidences;

  // Custom swap function for Prediction , for the second version of
  // non_maximum_suppression
  friend void swap(Prediction& first, Prediction& second) noexcept {
    using std::swap;
    swap(first.x, second.x);
    swap(first.y, second.y);
    swap(first.width, second.width);
    swap(first.height, second.height);
    swap(first.confidence, second.confidence);
    swap(first.class_confidences, second.class_confidences);
  }
  friend void swap(const Prediction& first, const Prediction& second) noexcept {
    Prediction tmp(first);
    swap(const_cast<Prediction&>(first), const_cast<Prediction&>(second));
    swap(const_cast<Prediction&>(second), tmp);
  }
};

std::vector<float> anchor_to_box(int image_width, int image_height,
                                 const Prediction& anchor_box);

float calculate_iou(const Prediction& prediction1,
                    const Prediction& prediction2, int image_width,
                    int image_height);

std::vector<Prediction> non_maximum_suppression(
    const std::vector<Prediction>& predictions,
    float confidence_threshold = 0.5, float iou_threshold = 0.5,
    int image_width = 416, int image_height = 416);

enum class DetectionClass { kBlack = 0, kSmall = 1, kBig = 2 };

std::vector<uint8_t> get_detection_classes(
    const std::vector<Prediction>& predictions,
    float confidence_threshold = 0.5);

void convert_rgb565_to_rgb888(uint8_t* rgb565, uint8_t* rgb888, int image_width,
                              int image_height);

float dequantize(uint8_t quantized_value, float scale, int zero_point);

#ifdef UNIT_TESTING
void convertOutputToFloat(const uint8_t* output, const int num_prediction,
                          std::vector<Prediction>& predictions,
                          int num_classes);
#else
void convertOutputToFloat(const TfLiteTensor* output,
                          std::vector<Prediction>& predictions,
                          int num_classes);
#endif

#ifndef UNIT_TESTING
void printTensorDimensions(TfLiteTensor* tensor);
void RespondToDetection(float person_score, float no_person_score);
#endif

#endif  // __MODEL_UTILS_H__
