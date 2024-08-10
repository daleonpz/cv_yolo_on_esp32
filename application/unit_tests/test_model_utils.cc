#include "test_model_utils.h"

namespace {
const float kTolerance = 1e-5;
const float kConfidenceThreshold = 0.5;
const float kIouThreshold = 0.3;
const int kImageWidth = 96;
const int kImageHeight = 96;
}  // namespace

TEST(ModelUtilsTest, AnchorToBox) {
  // Create sample anchor box for testing
  Prediction anchor_box = {0.4392157,  0.59607846,
                           0.18039216, 0.12941177,
                           0.5019608,  {0.01960784, 0.57254905, 0.48235294}};
  // Perform anchor to box conversion
  std::vector<float> result =
      anchor_to_box(kImageWidth, kImageHeight, anchor_box);
  // Validate the result based on your expectations
  std::vector<float> expected_output = {33.505882, 51.011765, 50.82353,
                                        63.435295};

  ASSERT_EQ(result.size(),
            expected_output.size());  // Replace with your expected result size

  // Add more specific validation based on your expectations
  for (size_t i = 0; i < expected_output.size(); i++) {
    ASSERT_NEAR(result[i], expected_output[i], kTolerance);
  }
}

TEST(ModelUtilsTest, CalculateIou) {
  // Create sample predictions for testing
  Prediction prediction1 = {0.4392157, 0.59607846,
                            0.1764706, 0.1254902,
                            0.6156863, {0.01960784, 0.62352943, 0.5019608}};
  Prediction prediction2 = {0.4392157,  0.59607846,
                            0.15294118, 0.16470589,
                            0.54901963, {0.01960784, 0.5803922, 0.50980395}};
  // Perform IOU calculation
  float result =
      calculate_iou(prediction1, prediction2, kImageWidth, kImageHeight);
  // Validate the result based on your expectations
  float expected_output = 0.6980392;

  ASSERT_NEAR(result, expected_output, kTolerance);
}

TEST(ModelUtilsTest, NonMaximumSuppression) {
  // Create sample predictions for testing
  std::vector<Prediction> predictions = {{0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.17254902,
                                          0.84705883,
                                          {0.00784314, 0.62352943, 0.62352943}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18039216,
                                          0.12941177,
                                          0.85882354,
                                          {0.00784314, 0.6431373, 0.6745098}},
                                         {0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.17254902,
                                          0.6901961,
                                          {0.00784314, 0.5803922, 0.54901963}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18039216,
                                          0.12941177,
                                          0.72156864,
                                          {0.01176471, 0.59607846, 0.5294118}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.17254902,
                                          0.6509804,
                                          {0.02352941, 0.6313726, 0.49019608}},
                                         {0.627451,
                                          0.3764706,
                                          0.18039216,
                                          0.12941177,
                                          0.6666667,
                                          {0.01176471, 0.5294118, 0.49019608}},
                                         {0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.17254902,
                                          0.59607846,
                                          {0.03137255, 0.5568628, 0.45882353}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18039216,
                                          0.12941177,
                                          0.59607846,
                                          {0.02352941, 0.49019608, 0.49019608}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.17254902,
                                          0.6901961,
                                          {0.03921569, 0.49019608, 0.50980395}},
                                         {0.627451,
                                          0.3764706,
                                          0.18039216,
                                          0.12941177,
                                          0.6509804,
                                          {0.03529412, 0.50980395, 0.44313726}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.17254902,
                                          0.79607844,
                                          {0.00784314, 0.6901961, 0.5568628}},
                                         {0.627451,
                                          0.3764706,
                                          0.18039216,
                                          0.12941177,
                                          0.80784315,
                                          {0.00784314, 0.65882355, 0.5568628}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.17254902,
                                          0.5803922,
                                          {0.01568628, 0.59607846, 0.52156866}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18039216,
                                          0.12941177,
                                          0.54901963,
                                          {0.01568628, 0.59607846, 0.5372549}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.17254902,
                                          0.50980395,
                                          {0.01960784, 0.5882353, 0.49019608}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18039216,
                                          0.12941177,
                                          0.5294118,
                                          {0.01960784, 0.5803922, 0.4745098}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.11372549,
                                          0.5882353,
                                          {0.01568628, 0.6313726, 0.48235294}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18039216,
                                          0.12941177,
                                          0.59607846,
                                          {0.01960784, 0.6313726, 0.5019608}}};

  // Perform non-maximum suppression
  std::vector<Prediction> result =
      non_maximum_suppression(predictions, kConfidenceThreshold, kIouThreshold,
                              kImageWidth, kImageHeight);

  // Validate the result based on your expectations
  size_t expected_result_size = 3;
  std::vector<Prediction> expected_output = {
      {0.21960784,
       0.3137255,
       0.18039216,
       0.12941177,
       0.85882354,
       {0.00784314, 0.6431373, 0.6745098}},
      {0.627451,
       0.3764706,
       0.18039216,
       0.12941177,
       0.80784315,
       {0.00784314, 0.65882355, 0.5568628}},
      {0.4392157,
       0.59607846,
       0.18039216,
       0.12941177,
       0.59607846,
       {0.01960784, 0.6313726, 0.5019608}}};

  ASSERT_EQ(result.size(),
            expected_result_size);  // Replace with your expected result size

  // Add more specific validation based on your expectations
  for (size_t i = 0; i < expected_result_size; i++) {
    ASSERT_NEAR(result[i].x, expected_output[i].x, kTolerance);
    ASSERT_NEAR(result[i].y, expected_output[i].y, kTolerance);
    ASSERT_NEAR(result[i].width, expected_output[i].width, kTolerance);
    ASSERT_NEAR(result[i].height, expected_output[i].height, kTolerance);
    ASSERT_NEAR(result[i].confidence, expected_output[i].confidence,
                kTolerance);
    for (size_t j = 0; j < result[i].class_confidences.size(); j++) {
      ASSERT_NEAR(result[i].class_confidences[j],
                  expected_output[i].class_confidences[j], kTolerance);
    }
  }
}

TEST(ModelUtilsTest, NonMaximumSuppressionUnsortedConfindence) {
  std::vector<Prediction> predictions = {{0.21960784,
                                          0.3137255,
                                          0.0,
                                          0.0,
                                          0.5019608,
                                          {0.00784314, 0.54509807, 0.64705884}},
                                         {0.21960784,
                                          0.3137255,
                                          0.0,
                                          0.1764706,
                                          0.69803923,
                                          {0.01176471, 0.6, 0.53333336}},
                                         {0.2509804,
                                          0.3137255,
                                          0.18431373,
                                          0.13333334,
                                          0.7294118,
                                          {0.01176471, 0.6, 0.5176471}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.1764706,
                                          0.64705884,
                                          {0.02352941, 0.62352943, 0.49019608}},
                                         {0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.0,
                                          0.827451,
                                          {0.00784314, 0.6156863, 0.6156863}},
                                         {0.627451,
                                          0.3764706,
                                          0.18431373,
                                          0.13333334,
                                          0.6666667,
                                          {0.01176471, 0.5176471, 0.49019608}},
                                         {0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.1764706,
                                          0.60784316,
                                          {0.03137255, 0.5529412, 0.44313726}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18431373,
                                          0.13333334,
                                          0.6,
                                          {0.02352941, 0.50980395, 0.48235294}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.1764706,
                                          0.6901961,
                                          {0.03921569, 0.48235294, 0.5176471}},
                                         {0.627451,
                                          0.3764706,
                                          0.18431373,
                                          0.13333334,
                                          0.65882355,
                                          {0.03921569, 0.5019608, 0.44313726}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.1764706,
                                          0.8039216,
                                          {0.00784314, 0.6666667, 0.5529412}},
                                         {0.627451,
                                          0.3764706,
                                          0.18431373,
                                          0.13333334,
                                          0.81960785,
                                          {0.00784314, 0.6392157, 0.5529412}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.1764706,
                                          0.5764706,
                                          {0.01568628, 0.5921569, 0.5176471}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18431373,
                                          0.13333334,
                                          0.54509807,
                                          {0.01568628, 0.5764706, 0.5254902}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.11764706,
                                          0.50980395,
                                          {0.02352941, 0.5921569, 0.48235294}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18431373,
                                          0.13333334,
                                          0.5176471,
                                          {0.01960784, 0.5686275, 0.46666667}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.11764706,
                                          0.60784316,
                                          {0.01568628, 0.6392157, 0.48235294}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18431373,
                                          0.13333334,
                                          0.8352941,
                                          {0.00784314, 0.6313726, 0.6666667}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18431373,
                                          0.13333334,
                                          0.62352943,
                                          {0.01960784, 0.6392157, 0.49019608}}};

  // Perform non-maximum suppression
  std::vector<Prediction> result =
      non_maximum_suppression(predictions, kConfidenceThreshold, kIouThreshold,
                              kImageWidth, kImageHeight);

  // Validate the result based on your expectations
  size_t expected_result_size = 3;
  std::vector<Prediction> expected_output = {
      {0.21960784,
       0.3137255,
       0.18431373,
       0.13333334,
       0.8352941,
       {0.00784314, 0.6313726, 0.6666667}},
      {0.627451,
       0.3764706,
       0.18431373,
       0.13333334,
       0.81960785,
       {0.00784314, 0.6392157, 0.5529412}},
      {0.4392157,
       0.59607846,
       0.18431373,
       0.13333334,
       0.62352943,
       {0.01960784, 0.6392157, 0.49019608}}};

  ASSERT_EQ(result.size(),
            expected_result_size);  // Replace with your expected result size

  // Add more specific validation based on your expectations
  for (size_t i = 0; i < expected_result_size; i++) {
    ASSERT_NEAR(result[i].x, expected_output[i].x, kTolerance);
    ASSERT_NEAR(result[i].y, expected_output[i].y, kTolerance);
    ASSERT_NEAR(result[i].width, expected_output[i].width, kTolerance);
    ASSERT_NEAR(result[i].height, expected_output[i].height, kTolerance);
    ASSERT_NEAR(result[i].confidence, expected_output[i].confidence,
                kTolerance);
    for (size_t j = 0; j < result[i].class_confidences.size(); j++) {
      ASSERT_NEAR(result[i].class_confidences[j],
                  expected_output[i].class_confidences[j], kTolerance);
    }
  }
}

TEST(ModelUtilsTest, NonMaximumSuppressionZeroWidthHeight) {
  std::vector<Prediction> predictions = {{0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.0,
                                          0.827451,
                                          {0.00784314, 0.6156863, 0.6156863}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18431373,
                                          0.13333334,
                                          0.8352941,
                                          {0.00784314, 0.6313726, 0.6666667}},
                                         {0.21960784,
                                          0.3137255,
                                          0.0,
                                          0.0,
                                          0.5019608,
                                          {0.00784314, 0.54509807, 0.64705884}},
                                         {0.21960784,
                                          0.3137255,
                                          0.0,
                                          0.1764706,
                                          0.69803923,
                                          {0.01176471, 0.6, 0.53333336}},
                                         {0.2509804,
                                          0.3137255,
                                          0.18431373,
                                          0.13333334,
                                          0.7294118,
                                          {0.01176471, 0.6, 0.5176471}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.1764706,
                                          0.64705884,
                                          {0.02352941, 0.62352943, 0.49019608}},
                                         {0.627451,
                                          0.3764706,
                                          0.18431373,
                                          0.13333334,
                                          0.6666667,
                                          {0.01176471, 0.5176471, 0.49019608}},
                                         {0.21960784,
                                          0.3137255,
                                          0.16078432,
                                          0.1764706,
                                          0.60784316,
                                          {0.03137255, 0.5529412, 0.44313726}},
                                         {0.21960784,
                                          0.3137255,
                                          0.18431373,
                                          0.13333334,
                                          0.6,
                                          {0.02352941, 0.50980395, 0.48235294}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.1764706,
                                          0.6901961,
                                          {0.03921569, 0.48235294, 0.5176471}},
                                         {0.627451,
                                          0.3764706,
                                          0.18431373,
                                          0.13333334,
                                          0.65882355,
                                          {0.03921569, 0.5019608, 0.44313726}},
                                         {0.627451,
                                          0.3764706,
                                          0.16078432,
                                          0.1764706,
                                          0.8039216,
                                          {0.00784314, 0.6666667, 0.5529412}},
                                         {0.627451,
                                          0.3764706,
                                          0.18431373,
                                          0.13333334,
                                          0.81960785,
                                          {0.00784314, 0.6392157, 0.5529412}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.1764706,
                                          0.5764706,
                                          {0.01568628, 0.5921569, 0.5176471}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18431373,
                                          0.13333334,
                                          0.54509807,
                                          {0.01568628, 0.5764706, 0.5254902}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.11764706,
                                          0.50980395,
                                          {0.02352941, 0.5921569, 0.48235294}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18431373,
                                          0.13333334,
                                          0.5176471,
                                          {0.01960784, 0.5686275, 0.46666667}},
                                         {0.4392157,
                                          0.59607846,
                                          0.16078432,
                                          0.11764706,
                                          0.60784316,
                                          {0.01568628, 0.6392157, 0.48235294}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18431373,
                                          0.13333334,
                                          0.62352943,
                                          {0.01960784, 0.6392157, 0.49019608}}};

  // Perform non-maximum suppression
  std::vector<Prediction> result =
      non_maximum_suppression(predictions, kConfidenceThreshold, kIouThreshold,
                              kImageWidth, kImageHeight);

  // Validate the result based on your expectations
  size_t expected_result_size = 3;
  std::vector<Prediction> expected_output = {
      {0.21960784,
       0.3137255,
       0.18431373,
       0.13333334,
       0.8352941,
       {0.00784314, 0.6313726, 0.6666667}},
      {0.627451,
       0.3764706,
       0.18431373,
       0.13333334,
       0.81960785,
       {0.00784314, 0.6392157, 0.5529412}},
      {0.4392157,
       0.59607846,
       0.18431373,
       0.13333334,
       0.62352943,
       {0.01960784, 0.6392157, 0.49019608}}};

  ASSERT_EQ(result.size(),
            expected_result_size);  // Replace with your expected result size

  // Add more specific validation based on your expectations
  for (size_t i = 0; i < expected_result_size; i++) {
    ASSERT_NEAR(result[i].x, expected_output[i].x, kTolerance);
    ASSERT_NEAR(result[i].y, expected_output[i].y, kTolerance);
    ASSERT_NEAR(result[i].width, expected_output[i].width, kTolerance);
    ASSERT_NEAR(result[i].height, expected_output[i].height, kTolerance);
    ASSERT_NEAR(result[i].confidence, expected_output[i].confidence,
                kTolerance);
    for (size_t j = 0; j < result[i].class_confidences.size(); j++) {
      ASSERT_NEAR(result[i].class_confidences[j],
                  expected_output[i].class_confidences[j], kTolerance);
    }
  }
}

TEST(ModelUtilsTest, GetDetectionClassesTest) {
  std::vector<Prediction> predictions = {{0.21960784,
                                          0.3137255,
                                          0.18039216,
                                          0.12941177,
                                          0.85882354,
                                          {0.00784314, 0.6431373, 0.6745098}},
                                         {0.627451,
                                          0.3764706,
                                          0.18039216,
                                          0.12941177,
                                          0.80784315,
                                          {0.00784314, 0.65882355, 0.5568628}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18039216,
                                          0.12941177,
                                          0.59607846,
                                          {0.01960784, 0.6313726, 0.5019608}}};

  // Perform get detection classes
  std::vector<uint8_t> result = get_detection_classes(predictions, 0.5);

  // Validate the result based on your expectations
  std::vector<uint8_t> expected_output = {2, 1, 1};

  ASSERT_EQ(result.size(),
            expected_output.size());  // Replace with your expected result size
  ASSERT_EQ(result, expected_output);
}

TEST(ModelUtilsTest, NoClassConfidencesTest) {
  std::vector<Prediction> predictions = {
      {0.21960784, 0.3137255, 0.18039216, 0.12941177, 0.85882354, {}},
      {0.627451, 0.3764706, 0.18039216, 0.12941177, 0.80784315, {}},
      {0.4392157, 0.59607846, 0.18039216, 0.12941177, 0.59607846, {}}};

  // Perform get detection classes
  std::vector<uint8_t> result = get_detection_classes(predictions, 0.5);

  // Validate the result based on your expectations
  std::vector<uint8_t> expected_output = {};

  ASSERT_EQ(result.size(),
            expected_output.size());  // Replace with your expected result size
  ASSERT_EQ(result, expected_output);
}

TEST(ModelUtilsTest, ClassDetectionConfidenceBelowThreshold) {
  std::vector<Prediction> predictions = {{0.21960784,
                                          0.3137255,
                                          0.18039216,
                                          0.12941177,
                                          0.85882354,
                                          {0.00784314, 0.6431373, 0.6745098}},
                                         {0.627451,
                                          0.3764706,
                                          0.18039216,
                                          0.12941177,
                                          0.80784315,
                                          {0.00784314, 0.65882355, 0.5568628}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18039216,
                                          0.12941177,
                                          0.59607846,
                                          {0.01960784, 0.6313726, 0.5019608}}};

  // Perform get detection classes
  std::vector<uint8_t> result = get_detection_classes(predictions, 0.9);

  // Validate the result based on your expectations
  std::vector<uint8_t> expected_output = {};

  ASSERT_EQ(result.size(),
            expected_output.size());  // Replace with your expected result size
  ASSERT_EQ(result, expected_output);
}

TEST(ModelUtilsTest, ClassDetectionUseConfidenceInsteadOfClassConfidence) {
  std::vector<Prediction> predictions = {{0.21960784,
                                          0.3137255,
                                          0.18039216,
                                          0.12941177,
                                          0.85882354,
                                          {0.00784314, 0.0431373, 0.0745098}},
                                         {0.627451,
                                          0.3764706,
                                          0.18039216,
                                          0.12941177,
                                          0.80784315,
                                          {0.00784314, 0.05882355, 0.0568628}},
                                         {0.4392157,
                                          0.59607846,
                                          0.18039216,
                                          0.12941177,
                                          0.59607846,
                                          {0.01960784, 0.0313726, 0.0019608}}};

  // Perform get detection classes
  std::vector<uint8_t> result = get_detection_classes(predictions, 0.2);

  // Validate the result based on your expectations
  std::vector<uint8_t> expected_output = {};

  ASSERT_EQ(result.size(),
            expected_output.size());  // Replace with your expected result size
  ASSERT_EQ(result, expected_output);
}

TEST(ModelUtilsTest, ConvertRGB565ToRGB888) {
  // Create sample RGB565 pixel for testing
  uint8_t rgb565[16] = {0x28, 0xc4, 0x28, 0xe4, 0x20, 0xc5, 0x21, 0x05,
                        0x28, 0xc5, 0x28, 0xc5, 0x30, 0xc2, 0x30, 0xc1};

  // Perform RGB565 to RGB888 conversion
  uint8_t rgb888[24] = {0};
  convert_rgb565_to_rgb888(rgb565, rgb888, 4, 2);

  // Validate the result based on your expectations
  uint8_t expected_output[24] = {
      0x29, 0x18, 0x21, 0x29, 0x1c, 0x21, 0x21, 0x18, 0x29, 0x21, 0x20, 0x29,
      0x29, 0x18, 0x29, 0x29, 0x18, 0x29, 0x31, 0x18, 0x10, 0x31, 0x18, 0x08,
  };

  // Add more specific validation based on your expectations
  for (size_t i = 0; i < 24; i++) {
    ASSERT_EQ(rgb888[i], expected_output[i]);
  }
}

TEST(ModelUtilsTest, ConvertOutputToFloat) {
  uint8_t dut[] = {16, 16, 40,  29, 5,  209, 30, 36,  16, 16, 46,
                   33, 5,  198, 32, 33, 16,  8,  85,  79, 2,  154,
                   26, 41, 32,  16, 40, 44,  21, 253, 7,  17};

  std::vector<Prediction> predictions;
  convertOutputToFloat(dut, 4, predictions, 3);

  std::vector<Prediction> expected_output = {
      {dut[0] / 255.0f,
       dut[1] / 255.0f,
       dut[2] / 255.0f,
       dut[3] / 255.0f,
       dut[4] / 255.0f,
       {dut[5] / 255.0f, dut[6] / 255.0f, dut[7] / 255.0f}},
      {dut[8] / 255.0f,
       dut[9] / 255.0f,
       dut[10] / 255.0f,
       dut[11] / 255.0f,
       dut[12] / 255.0f,
       {dut[13] / 255.0f, dut[14] / 255.0f, dut[15] / 255.0f}},
      {dut[16] / 255.0f,
       dut[17] / 255.0f,
       dut[18] / 255.0f,
       dut[19] / 255.0f,
       dut[20] / 255.0f,
       {dut[21] / 255.0f, dut[22] / 255.0f, dut[23] / 255.0f}},
      {dut[24] / 255.0f,
       dut[25] / 255.0f,
       dut[26] / 255.0f,
       dut[27] / 255.0f,
       dut[28] / 255.0f,
       {dut[29] / 255.0f, dut[30] / 255.0f, dut[31] / 255.0f}}};

  ASSERT_EQ(predictions.size(), expected_output.size());

  for (size_t i = 0; i < expected_output.size(); i++) {
    ASSERT_NEAR(predictions[i].x, expected_output[i].x, kTolerance);
    ASSERT_NEAR(predictions[i].y, expected_output[i].y, kTolerance);
    ASSERT_NEAR(predictions[i].width, expected_output[i].width, kTolerance);
    ASSERT_NEAR(predictions[i].height, expected_output[i].height, kTolerance);
    ASSERT_NEAR(predictions[i].confidence, expected_output[i].confidence,
                kTolerance);
    for (size_t j = 0; j < predictions[i].class_confidences.size(); j++) {
      ASSERT_NEAR(predictions[i].class_confidences[j],
                  expected_output[i].class_confidences[j], kTolerance);
    }
  }
}
