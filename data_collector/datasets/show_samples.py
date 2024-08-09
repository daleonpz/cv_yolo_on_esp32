# Description: This script displays random samples from the train, validation, and test sets with bounding boxes.

import os
import random
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image

# Function to parse YOLO format labels
def parse_yolo_label(label_path):
    with open(label_path, 'r') as file:
        content = file.readlines()

    boxes = []
    for line in content:
        values = line.strip().split()
        class_label, x, y, w, h = map(float, values)
        boxes.append((class_label, x, y, w, h))

    return boxes

# Function to display images with bounding boxes
def display_images_with_boxes(image_folder, label_folder, num_samples):
    # Get random samples
    file_list = os.listdir(image_folder)
    samples = random.sample(file_list, num_samples)

    for filename in samples:
        image_path = os.path.join(image_folder, filename)
        label_path = os.path.join(label_folder, filename.replace('.png', '.txt'))

        # Load image
        img = Image.open(image_path)

        # Parse YOLO labels
        boxes = parse_yolo_label(label_path)

        # Plot image and bounding boxes
#         fig, ax = plt.subplots()
        plot_anchor_boxes( img, boxes )
#         ax.imshow(img)
# 
#         # Add bounding boxes to the plot
#         for box in boxes:
#             class_label, x, y, w, h = box
#             x *= img.width
#             y *= img.height
#             w *= img.width
#             h *= img.height
#             x1 = x - w / 2
#             y1 = y - h / 2
# 
#             rect = patches.Rectangle((x1, y1), w, h, linewidth=1, edgecolor='r', facecolor='none')
#             ax.add_patch(rect)
# 
#         plt.title(f'Image: {filename}')
#         plt.show()
# 
def plot_anchor_boxes(image, anchor_boxes):
    import matplotlib.pyplot as plt
    import matplotlib.patches as patches
    fig, ax = plt.subplots(1)
    ax.imshow(image)
    image_width, image_height = image.size
    print(f'image_width: {image_width}, image_height: {image_height}')
    for anchor_box in anchor_boxes:
        print(f'anchor_box: {anchor_box}')
        x = ( anchor_box[1] - anchor_box[3] / 2 ) * image_width
        y = ( anchor_box[2] - anchor_box[4] / 2 ) * image_height
        w = anchor_box[3] * image_width
        h = anchor_box[4] * image_height
        rect = patches.Rectangle((x, y), w, h, linewidth=1, edgecolor='r', facecolor='none')
        ax.add_patch(rect)
    plt.show()  

# Define directories
image_dir = 'images'
label_dir = 'labels'
num_samples = 3  # Number of random samples to display

# Display random samples with bounding boxes for train, validation, and test sets
for dataset in ['train', 'val', 'test']:
    dataset_image_folder = os.path.join(image_dir, dataset)
    dataset_label_folder = os.path.join(label_dir, dataset)
    print(f"Displaying {num_samples} random samples from {dataset} set:")
    display_images_with_boxes(dataset_image_folder, dataset_label_folder, num_samples)

