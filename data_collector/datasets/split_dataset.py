import torch
import torch.nn as nn
import torch.optim as optim

from PIL import Image, ImageFile

ImageFile.LOAD_TRUNCATED_IMAGES = True

import albumentations as A
from albumentations.pytorch import ToTensorV2
import cv2

import os
import numpy as np
import pandas as pd

import matplotlib.pyplot as plt
import matplotlib.patches as patches

from tqdm import tqdm

from utils import *

random.seed(0)

# Get any random annotation file
labels = [os.path.join("labels", x) for x in os.listdir("labels") if x[-3:] == "txt"]
labels.sort()

annotation_file = random.choice(labels)
print(annotation_file)
with open(annotation_file, "r") as file:
    annotation_list = file.read().split("\n")
    annotation_list = [x.split(" ") for x in annotation_list]
    annotation_list = [[float(y) for y in x] for x in annotation_list]

# Get the corresponding image file
image_file = annotation_file.replace("labels", "images").replace("txt", "png")
assert os.path.exists(image_file)

# #Load the image
# image = Image.open(image_file)
#
# #Plot the Bounding Box
# plot_bounding_box(image, annotation_list)

# Read images and labels
images = [os.path.join("images", x) for x in os.listdir("images") if x[-3:] == "png"]
labels = [os.path.join("labels", x) for x in os.listdir("labels") if x[-3:] == "txt"]

images.sort()
labels.sort()

# Split the dataset into train-valid-test splits
train_images, val_images, train_labels, val_labels = train_test_split(
    images, labels, test_size=0.2, random_state=1
)
val_images, test_images, val_labels, test_labels = train_test_split(
    val_images, val_labels, test_size=0.1, random_state=1
)


# Utility function to move images
def move_files_to_folder(list_of_files, destination_folder):
    for f in list_of_files:
        try:
            shutil.move(f, destination_folder)
        except:
            print(f)
            assert False


# Move the splits into their folders
# Create the folders if they do not exist
os.makedirs("images/train", exist_ok=True)
os.makedirs("images/val", exist_ok=True)
os.makedirs("images/test", exist_ok=True)
os.makedirs("labels/train", exist_ok=True)
os.makedirs("labels/val", exist_ok=True)
os.makedirs("labels/test", exist_ok=True)

move_files_to_folder(train_images, "images/train")
move_files_to_folder(val_images, "images/val/")
move_files_to_folder(test_images, "images/test/")
move_files_to_folder(train_labels, "labels/train/")
move_files_to_folder(val_labels, "labels/val/")
move_files_to_folder(test_labels, "labels/test/")
