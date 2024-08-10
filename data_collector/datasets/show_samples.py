# Description: This script displays random samples from the train, validation, and test sets with bounding boxes.

import os
import random
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image

import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


def parse_yolo_label(label_path):
    """Parse YOLO format labels."""
    with open(label_path, "r") as file:
        content = file.readlines()

    boxes = []
    for line in content:
        values = line.strip().split()
        class_label, x, y, w, h = map(float, values)
        boxes.append((class_label, x, y, w, h))

    return boxes


def display_images_with_boxes(image_folder, label_folder, num_samples):
    """Display random samples with bounding boxes."""
    # Get random samples
    file_list = os.listdir(image_folder)
    samples = random.sample(file_list, num_samples)

    for filename in samples:
        image_path = os.path.join(image_folder, filename)
        label_path = os.path.join(label_folder, filename.replace(".png", ".txt"))

        # Load image
        img = Image.open(image_path)

        # Parse YOLO labels
        boxes = parse_yolo_label(label_path)

        # Plot image and bounding boxes
        plot_anchor_boxes(img, boxes)


def plot_anchor_boxes(image, anchor_boxes):
    """Plot image and anchor boxes."""
    import matplotlib.pyplot as plt
    import matplotlib.patches as patches

    fig, ax = plt.subplots(1)
    ax.imshow(image)
    image_width, image_height = image.size
    logger.info(f"image_width: {image_width}, image_height: {image_height}")
    for anchor_box in anchor_boxes:
        logger.info(f"anchor_box: {anchor_box}")
        x = (anchor_box[1] - anchor_box[3] / 2) * image_width
        y = (anchor_box[2] - anchor_box[4] / 2) * image_height
        w = anchor_box[3] * image_width
        h = anchor_box[4] * image_height
        rect = patches.Rectangle(
            (x, y), w, h, linewidth=1, edgecolor="r", facecolor="none"
        )
        ax.add_patch(rect)
    plt.show()


# Define directories
image_dir = "images"
label_dir = "labels"
num_samples = 3  # Number of random samples to display

# Display random samples with bounding boxes for train, validation, and test sets
for dataset in ["train", "val", "test"]:
    dataset_image_folder = os.path.join(image_dir, dataset)
    dataset_label_folder = os.path.join(label_dir, dataset)
    logger.info(f"Displaying {num_samples} random samples from {dataset} set:")
    display_images_with_boxes(dataset_image_folder, dataset_label_folder, num_samples)
