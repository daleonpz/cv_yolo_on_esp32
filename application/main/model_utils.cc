#include "model_utils.h"

#include <algorithm>

std::vector<float> anchor_to_box(int image_width, int image_height,
                                 const Prediction& anchor_box) {
  float x1 = (anchor_box.x - anchor_box.width / 2) * image_width;
  float y1 = (anchor_box.y - anchor_box.height / 2) * image_height;
  float x2 = (anchor_box.x + anchor_box.width / 2) * image_width;
  float y2 = (anchor_box.y + anchor_box.height / 2) * image_height;
  return {x1, y1, x2, y2};
}

float calculate_iou(const Prediction& prediction1,
                    const Prediction& prediction2, int image_width,
                    int image_height) {
  auto box1 = anchor_to_box(image_width, image_height, prediction1);
  auto box2 = anchor_to_box(image_width, image_height, prediction2);

  float x1 = std::max(box1[0], box2[0]);
  float y1 = std::max(box1[1], box2[1]);
  float x2 = std::min(box1[2], box2[2]);
  float y2 = std::min(box1[3], box2[3]);

  float intersection =
      std::max(0.0f, x2 - x1 + 1) * std::max(0.0f, y2 - y1 + 1);

  float area_box1 = (box1[2] - box1[0] + 1) * (box1[3] - box1[1] + 1);
  float area_box2 = (box2[2] - box2[0] + 1) * (box2[3] - box2[1] + 1);

  float union_area = area_box1 + area_box2 - intersection;

  float iou = intersection / union_area;
  return iou;
}

std::vector<Prediction> non_maximum_suppression(
    const std::vector<Prediction>& predictions, float confidence_threshold,
    float iou_threshold, int image_width, int image_height) {
  //     MicroPrintf("Number of predictions before confidence threshold: %d\n",
  //     predictions.size());
  // some predictions may have width and height equal to 0, we need to filter
  // them out because they will cause an IoU of 0 with any other prediction, and
  // will be selected in the non-maximum suppression
  auto is_confident = [confidence_threshold](const Prediction& prediction) {
    return prediction.confidence >= confidence_threshold &&
           prediction.width > 0 && prediction.height > 0;
  };

  // The filtering based on confidence threshold is done using std::partition,
  // which rearranges elements in the vector, putting elements satisfying the
  // condition (is_confident) to the front. This avoids the need for an
  // additional std::remove_if call
  std::vector<Prediction> filtered_predictions(
      predictions.begin(),
      std::partition(predictions.begin(), predictions.end(), is_confident));

  //     MicroPrintf("Number of predictions before NMS: %d\n",
  //     filtered_predictions.size());

  std::sort(filtered_predictions.begin(), filtered_predictions.end(),
            [](const Prediction& a, const Prediction& b) {
              return a.confidence > b.confidence;
            });

  std::vector<Prediction> selected_predictions;
  while (!filtered_predictions.empty()) {
    selected_predictions.push_back(filtered_predictions[0]);
    filtered_predictions.erase(filtered_predictions.begin());

    std::vector<float> iou_values;
    for (const auto& prediction : filtered_predictions) {
      iou_values.push_back(calculate_iou(
          selected_predictions.back(), prediction, image_width, image_height));
    }

    filtered_predictions.erase(
        std::remove_if(
            filtered_predictions.begin(), filtered_predictions.end(),
            [&filtered_predictions, iou_threshold,
             &iou_values](const Prediction& prediction) {
              return iou_values[&prediction - &filtered_predictions[0]] >
                     iou_threshold;
            }),
        filtered_predictions.end());
  }

  return selected_predictions;
}

std::vector<uint8_t> get_detection_classes(
    const std::vector<Prediction>& predictions, float confidence_threshold) {
  std::vector<uint8_t> detection_classes = {};
  for (const auto& prediction : predictions) {
    auto max_it = std::max_element(prediction.class_confidences.begin(),
                                   prediction.class_confidences.end());

    //         MicroPrintf("prediction confidence: %f %f %f\n",
    //         prediction.class_confidences[0], prediction.class_confidences[1],
    //         prediction.class_confidences[2]);

    if (max_it != prediction.class_confidences.end()) {
      uint8_t idx = std::distance(prediction.class_confidences.begin(), max_it);
      if (prediction.class_confidences[idx] >= confidence_threshold) {
        detection_classes.push_back(idx);
      }
    }
  }
  return detection_classes;
}

#ifndef UNIT_TESTING
void printTensorDimensions(TfLiteTensor* tensor) {
  MicroPrintf("Tensor Rank: %d\n", tensor->dims->size);

  MicroPrintf("Tensor Dimensions: ");
  for (int i = 0; i < tensor->dims->size; ++i) {
    MicroPrintf("%d ", tensor->dims->data[i]);
  }
  MicroPrintf("\n");
  MicroPrintf("Tensor Type: %d, expected 3 (uint8) \n", tensor->type);
}
#endif  // UNIT_TESTING

float dequantize(uint8_t quantized_value, float scale, int zero_point) {
  return static_cast<float>(quantized_value) / 255.0f;
  //     return scale * (static_cast<float>(quantized_value) -
  //     static_cast<float>(zero_point));
}

#ifdef UNIT_TESTING
#define OUTPUT_DATA_UINT8 output
void convertOutputToFloat(const uint8_t* output, const int num_predictions,
                          std::vector<Prediction>& predictions,
                          int num_classes) {
  const int len_prediction = 5 + num_classes;

  const float scale = 1.0f;
  const int zero_point = 0;
#else
#define OUTPUT_DATA_UINT8 output->data.uint8
void convertOutputToFloat(const TfLiteTensor* output,
                          std::vector<Prediction>& predictions,
                          int num_classes) {
  const int num_predictions = output->dims->data[1];
  const int len_prediction = output->dims->data[2];

  const float scale = output->params.scale;
  const int zero_point = output->params.zero_point;
#endif  // UNIT_TESTING

  predictions.clear();

  for (int b = 0; b < num_predictions; ++b) {
    Prediction prediction;
    prediction.x =
        dequantize(OUTPUT_DATA_UINT8[b * len_prediction], scale, zero_point);
    prediction.y = dequantize(OUTPUT_DATA_UINT8[b * len_prediction + 1], scale,
                              zero_point);
    prediction.width = dequantize(OUTPUT_DATA_UINT8[b * len_prediction + 2],
                                  scale, zero_point);
    prediction.height = dequantize(OUTPUT_DATA_UINT8[b * len_prediction + 3],
                                   scale, zero_point);
    prediction.confidence = dequantize(
        OUTPUT_DATA_UINT8[b * len_prediction + 4], scale, zero_point);

    prediction.class_confidences.clear();
    for (int c = 0; c < num_classes; ++c) {
      prediction.class_confidences.push_back(dequantize(
          OUTPUT_DATA_UINT8[b * len_prediction + 5 + c], scale, zero_point));
    }
    predictions.push_back(prediction);
  }
}

// rgb565 is MxNx2, uint8_t
void convert_rgb565_to_rgb888(uint8_t* rgb565, uint8_t* rgb888, int image_width,
                              int image_height) {
  // this is the C++ version of the conversion
  for (int i = 0; i < image_width * image_height; i++) {
    uint8_t r5 = (rgb565[i * 2] & 0xF8) >> 3;
    uint8_t g6 =
        ((rgb565[i * 2] & 0x07) << 3) | ((rgb565[i * 2 + 1] & 0xE0) >> 5);
    uint8_t b5 = (rgb565[i * 2 + 1] & 0x1F);

    // Calculate RGB888 values using bit shifting
    rgb888[i * 3] = (r5 << 3) | (r5 >> 2);
    rgb888[i * 3 + 1] = (g6 << 2) | (g6 >> 4);
    rgb888[i * 3 + 2] = (b5 << 3) | (b5 >> 2);
  }
}
