# Usage: python quant_test_model.py saved_model_path input_image_path

import numpy as np
import tensorflow as tf
from PIL import Image
import sys

import os
import logging

logging.getLogger("tensorflow").setLevel(logging.ERROR)
np.set_printoptions(suppress=True)
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

np.set_printoptions(threshold=np.inf)

CONF_THRESHOLD = 0.5
IOU_THRESHOLD = 0.5
Q_IUO_THRESHOLD = 0.3
Q_CONF_THRESHOLD = 0.5
INPUT_SIZE = 96
OUTPUT_DIR = "results"

# For testing
# def representative_dataset():
#     dataset_path = 'images/val'
#     datagen = tf.keras.preprocessing.image.ImageDataGenerator(rescale=1./255)
#     dataset = datagen.flow_from_directory(dataset_path, target_size=(96, 96), batch_size=32, class_mode=None, shuffle=False)
#
#     for imagebatch in dataset:
#         yield [imagebatch.astype(np.float32)]


def representative_dataset():  # TODO: improve this, may impact accuracy
    """Representative dataset for quantization."""
    for _ in range(100):
        data = np.random.rand(1, 96, 96, 3)
        yield [data.astype(np.float32)]


def convert_quantized_model_to_tflite(quantized_model_path, tflite_model_path):
    """Converts a quantized model to a TensorFlow Lite model."""
    converter = tf.lite.TFLiteConverter.from_saved_model(quantized_model_path)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.uint8
    converter.inference_output_type = tf.uint8
    tflite_quant_model = converter.convert()

    # Save the model
    with open(tflite_model_path, "wb") as f:
        f.write(tflite_quant_model)

    logger.info("TFLite model saved!")
    return tflite_quant_model


def convert_float_model_to_tflite(float_model_path, tflite_model_path):
    """Converts a float model to a TensorFlow Lite model."""
    converter = tf.lite.TFLiteConverter.from_saved_model(float_model_path)
    tflite_float_model = converter.convert()

    # Save the model
    with open(tflite_model_path, "wb") as f:
        f.write(tflite_float_model)

    logger.info("TFLite model saved!")
    return tflite_float_model


def plot_anchor_boxes(image, anchor_boxes, filename):
    """Plots the anchor boxes on the image."""
    import matplotlib.pyplot as plt
    import matplotlib.patches as patches

    fig, ax = plt.subplots(1)
    ax.imshow(image)
    image_width, image_height = image.size
    logger.info(f"image_width: {image_width}, image_height: {image_height}")

    for anchor_box in anchor_boxes:
        logger.info(f"anchor_box: {anchor_box}")
        x = (anchor_box[0] - anchor_box[2] / 2) * image_width
        y = (anchor_box[1] - anchor_box[3] / 2) * image_height
        w = anchor_box[2] * image_width
        h = anchor_box[3] * image_height

        # get class with highest probability
        class_id = np.argmax(anchor_box[5:])
        probability = anchor_box[5 + class_id]
        rect = patches.Rectangle(
            (x, y), w, h, linewidth=1, edgecolor="r", facecolor="none"
        )
        ax.text(x, y, f"{class_id}: {probability:.2f}", color="red")
        ax.add_patch(rect)

    # save the image
    fig.savefig(filename, bbox_inches="tight")


def anchor_to_box(image_width, image_height, anchor_box):
    """Converts anchor box coordinates to box coordinates."""
    x1 = (anchor_box[0] - anchor_box[2] / 2) * image_width
    y1 = (anchor_box[1] - anchor_box[3] / 2) * image_height
    x2 = (anchor_box[0] + anchor_box[2] / 2) * image_width
    y2 = (anchor_box[1] + anchor_box[3] / 2) * image_height
    return [x1, y1, x2, y2]


def non_maximum_suppression(
    predictions,
    confidence_threshold=0.5,
    iou_threshold=0.5,
    image_width=416,
    image_height=416,
):
    """Applies Non-Maximum Suppression (NMS) to the predictions.
    Args:
        predictions: a list of predictions, each represented as [x, y, w, h, confidence, class1, class2, ...]
        confidence_threshold: the confidence threshold
        iou_threshold: the Intersection over Union (IoU) threshold
        image_width: the image width
        image_height: the image height
    Returns:
        selected_predictions: a list of selected predictions
    """

    logger.debug(f"     BEGIN non_maximum_suppression")
    logger.debug(f"predictions.shape: {predictions.shape}")
    logger.debug(f"predictions: {predictions}")

    def calculate_iou(prediction1, prediction2):
        """Calculates Intersection over Union (IoU) for two bounding boxes."""
        # Calculate intersection coordinates
        box1 = anchor_to_box(image_width, image_height, prediction1)
        box2 = anchor_to_box(image_width, image_height, prediction2)

        x1 = max(box1[0], box2[0])
        y1 = max(box1[1], box2[1])
        x2 = min(box1[2], box2[2])
        y2 = min(box1[3], box2[3])

        # Calculate intersection area
        intersection = max(0, x2 - x1 + 1) * max(0, y2 - y1 + 1)

        # Calculate areas of each box
        area_box1 = (box1[2] - box1[0] + 1) * (box1[3] - box1[1] + 1)
        area_box2 = (box2[2] - box2[0] + 1) * (box2[3] - box2[1] + 1)

        # Calculate union area
        union = area_box1 + area_box2 - intersection

        # Calculate IoU
        iou = intersection / union
        return iou

    # Filter predictions based on confidence threshold
    filtered_predictions = [
        prediction
        for prediction in predictions
        if prediction[4] >= confidence_threshold
    ]

    # Sort predictions by confidence score (probability)
    filtered_predictions.sort(key=lambda x: x[4], reverse=True)

    # Apply Non-Maximum Suppression
    selected_predictions = []
    while len(filtered_predictions) > 0:
        selected_predictions.append(filtered_predictions[0])

        # Remove the selected prediction
        del filtered_predictions[0]

        # Apply NMS
        iou = [
            calculate_iou(selected_predictions[-1], prediction)
            for prediction in filtered_predictions
        ]

        filtered_predictions = [
            prediction
            for i, prediction in enumerate(filtered_predictions)
            if iou[i] < iou_threshold
        ]

    logger.debug(
        f"     END non_maximum_suppression selected_predictions.shape: {len(selected_predictions)}"
    )
    return selected_predictions


def parse_image(image_path):
    """Parses the image and preprocesses it."""
    image = Image.open(input_image_path)

    # convert image to numpy array
    image_data = np.asarray(image)

    logger.info(f"image_data.shape: {image_data.shape}")
    if image.mode == "RGBA":
        image_data_raw = image_data[:, :, :3]
    else:
        image_data_raw = image_data

    imagef32 = image_data_raw.astype(np.float32)
    imagef32 = imagef32 / 255.0
    imagef32 = np.expand_dims(imagef32, axis=0)

    imagei8 = image_data_raw.astype(np.uint8)
    imagei8 = np.expand_dims(imagei8, axis=0)

    return image, imagef32, imagei8


def convert_tflite_to_cpp(tflite_path, cpp_path, array_name="g_model"):
    """Converts a TensorFlow Lite model to a C++ source file."""
    with open(tflite_path, "rb") as f:
        data = f.read()

    # Prepare the C++ source file content
    cpp_content = '#include "model.h"\n'
    cpp_content += "alignas(8) const unsigned char {}[] = {{\n".format(array_name)

    # Format data in hex
    hex_data = ", ".join(f"0x{byte:02x}" for byte in data)

    # Split the data into lines of 36 bytes each
    hex_lines = [hex_data[i : i + 72] for i in range(0, len(hex_data), 72)]
    # Add the lines to the C++ content
    cpp_content += "\n".join("  " + line for line in hex_lines)
    cpp_content += "\n};\n"

    # Add the length of the array
    cpp_content += f"const int {array_name}_len = {len(data)};\n"

    # Write the content to the output file
    with open(cpp_path, "w") as f:
        f.write(cpp_content)

    logger.info(f"Converted {tflite_path} to {cpp_path}")


def parse_model(model_path, tflite_format="int8"):
    """Parses saved model and converts it to TFLite format."""
    model = tf.saved_model.load(model_path)

    if tflite_format == "int8":
        q_model = convert_quantized_model_to_tflite(
            model_path, OUTPUT_DIR + "/quantized_model.tflite"
        )
        convert_tflite_to_cpp(
            OUTPUT_DIR + "/quantized_model.tflite", OUTPUT_DIR + "/model.cc", "g_model"
        )

    else:
        q_model = convert_float_model_to_tflite(
            model_path, OUTPUT_DIR + "/float_model.tflite"
        )

    return model, q_model


def get_predictionf32(model, image):
    """Get prediction from float32 model."""
    out = model(image)

    # convert tuple to numpy array
    out = np.array(out)  # shape 1x1x432x9 , 432 = 12x12x3 (3 anchors, 12x12 grid)

    # delete all predictions with low confidence score
    # YOLO = [ x, y, w, h, confidence, class1, class2, ... ]
    candidates = out[out[:, :, :, 4] >= CONF_THRESHOLD]
    pred_nms = non_maximum_suppression(
        candidates, CONF_THRESHOLD, IOU_THRESHOLD, INPUT_SIZE, INPUT_SIZE
    )

    ## Calculate classes with highest probability
    pred = []
    boxes = []
    for i in range(len(pred_nms)):
        idx = np.argmax(pred_nms[i][5:])
        if pred_nms[i][5 + idx] > CONF_THRESHOLD:
            pred.append(idx)
            boxes.append(pred_nms[i])

    return pred, boxes


def get_predictioni8(model, image):
    """Get prediction from int8 model."""
    # Test 8bit quantized model
    interpreter = tf.lite.Interpreter(model_content=model)
    interpreter.allocate_tensors()

    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    interpreter.set_tensor(input_details[0]["index"], image)
    # print first 10 bytes of the input tensor
    interpreter.invoke()
    out = interpreter.get_tensor(output_details[0]["index"])  # shape 1x432x9

    logger.info(f"Q>>> output_data.shape: {out.shape}")

    # convert tuple to numpy array
    out = np.array(out) / 255.0

    candidates = out[out[:, :, 4] >= CONF_THRESHOLD]
    # delete candidates where w and h are 0, due to quantization
    candidates = candidates[candidates[:, 2] > 0]
    candidates = candidates[candidates[:, 3] > 0]

    # convert candidates to float
    candidates = candidates.astype(np.float32)

    q_pred_nms = non_maximum_suppression(
        candidates, Q_CONF_THRESHOLD, Q_IUO_THRESHOLD, INPUT_SIZE, INPUT_SIZE
    )

    ## Calculate classes with highest probability
    q_pred = []
    q_boxes = []
    for i in range(len(q_pred_nms)):
        idx = np.argmax(q_pred_nms[i][5:])
        if q_pred_nms[i][5 + idx] > Q_CONF_THRESHOLD:
            q_pred.append(idx)
            q_boxes.append(q_pred_nms[i])
    return q_pred, q_boxes


model_path = sys.argv[1]
input_image_path = sys.argv[2]

# create results directory if it does not exist
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

image_raw, imagef32, imagei8 = parse_image(input_image_path)
modelf32, q_model = parse_model(model_path, "int8")

predictionf32, boxesf32 = get_predictionf32(modelf32, imagef32)
logger.info("======== Prediction for Float32 model =========")
logger.info(f"predictionf32: {predictionf32}")
logger.info(f"boxesf32: {boxesf32}")
plot_anchor_boxes(image_raw, boxesf32, OUTPUT_DIR + "/outputf32.png")

predictioni8, boxesi8 = get_predictioni8(q_model, imagei8)
logger.info("======== Prediction for Int8 model =========")
logger.info(f"predictioni8: {predictioni8}")
logger.info(f"boxesi8: {boxesi8}")
plot_anchor_boxes(image_raw, boxesi8, OUTPUT_DIR + "/outputi8.png")

logger.info("Done!")
logger.info("=== output files are under results directory ===")
