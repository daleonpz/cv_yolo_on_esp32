import torch
import os 
import random
import shutil
from sklearn.model_selection import train_test_split
import xml.etree.ElementTree as ET
from xml.dom import minidom
from tqdm import tqdm
from PIL import Image, ImageDraw
import numpy as np
import matplotlib.pyplot as plt

random.seed(108)

# Defining a function to calculate Intersection over Union (IoU) 
def iou(box1, box2, is_pred=True): 
	if is_pred: 
		# IoU score for prediction and label 
		# box1 (prediction) and box2 (label) are both in [x, y, width, height] format 
		
		# Box coordinates of prediction 
		b1_x1 = box1[..., 0:1] - box1[..., 2:3] / 2
		b1_y1 = box1[..., 1:2] - box1[..., 3:4] / 2
		b1_x2 = box1[..., 0:1] + box1[..., 2:3] / 2
		b1_y2 = box1[..., 1:2] + box1[..., 3:4] / 2

		# Box coordinates of ground truth 
		b2_x1 = box2[..., 0:1] - box2[..., 2:3] / 2
		b2_y1 = box2[..., 1:2] - box2[..., 3:4] / 2
		b2_x2 = box2[..., 0:1] + box2[..., 2:3] / 2
		b2_y2 = box2[..., 1:2] + box2[..., 3:4] / 2

		# Get the coordinates of the intersection rectangle 
		x1 = torch.max(b1_x1, b2_x1) 
		y1 = torch.max(b1_y1, b2_y1) 
		x2 = torch.min(b1_x2, b2_x2) 
		y2 = torch.min(b1_y2, b2_y2) 
		# Make sure the intersection is at least 0 
		intersection = (x2 - x1).clamp(0) * (y2 - y1).clamp(0) 

		# Calculate the union area 
		box1_area = abs((b1_x2 - b1_x1) * (b1_y2 - b1_y1)) 
		box2_area = abs((b2_x2 - b2_x1) * (b2_y2 - b2_y1)) 
		union = box1_area + box2_area - intersection 

		# Calculate the IoU score 
		epsilon = 1e-6
		iou_score = intersection / (union + epsilon) 

		# Return IoU score 
		return iou_score 
	
	else: 
		# IoU score based on width and height of bounding boxes 
		
		# Calculate intersection area 
		intersection_area = torch.min(box1[..., 0], box2[..., 0]) * torch.min(box1[..., 1], box2[..., 1]) 

		# Calculate union area 
		box1_area = box1[..., 0] * box1[..., 1] 
		box2_area = box2[..., 0] * box2[..., 1] 
		union_area = box1_area + box2_area - intersection_area 

		# Calculate IoU score 
		iou_score = intersection_area / union_area 

		# Return IoU score 
		return iou_score


# Non-maximum suppression function to remove overlapping bounding boxes 
def nms(bboxes, iou_threshold, threshold): 
	# Filter out bounding boxes with confidence below the threshold. 
	bboxes = [box for box in bboxes if box[1] > threshold] 

	# Sort the bounding boxes by confidence in descending order. 
	bboxes = sorted(bboxes, key=lambda x: x[1], reverse=True) 

	# Initialize the list of bounding boxes after non-maximum suppression. 
	bboxes_nms = [] 

	while bboxes: 
		# Get the first bounding box. 
		first_box = bboxes.pop(0) 

		# Iterate over the remaining bounding boxes. 
		for box in bboxes: 
		# If the bounding boxes do not overlap or if the first bounding box has 
		# a higher confidence, then add the second bounding box to the list of 
		# bounding boxes after non-maximum suppression. 
			if box[0] != first_box[0] or iou( 
				torch.tensor(first_box[2:]), 
				torch.tensor(box[2:]), 
			) < iou_threshold: 
				# Check if box is not in bboxes_nms 
				if box not in bboxes_nms: 
					# Add box to bboxes_nms 
					bboxes_nms.append(box) 

	# Return bounding boxes after non-maximum suppression. 
	return bboxes_nms

# Function to convert cells to bounding boxes 
def convert_cells_to_bboxes(predictions, anchors, s, is_predictions=True): 
	# Batch size used on predictions 
	batch_size = predictions.shape[0] 
	# Number of anchors 
	num_anchors = len(anchors) 
	# List of all the predictions 
	box_predictions = predictions[..., 1:5] 

	# If the input is predictions then we will pass the x and y coordinate 
	# through sigmoid function and width and height to exponent function and 
	# calculate the score and best class. 
	if is_predictions: 
		anchors = anchors.reshape(1, len(anchors), 1, 1, 2) 
		box_predictions[..., 0:2] = torch.sigmoid(box_predictions[..., 0:2]) 
		box_predictions[..., 2:] = torch.exp( 
			box_predictions[..., 2:]) * anchors 
		scores = torch.sigmoid(predictions[..., 0:1]) 
		best_class = torch.argmax(predictions[..., 5:], dim=-1).unsqueeze(-1) 
	
	# Else we will just calculate scores and best class. 
	else: 
		scores = predictions[..., 0:1] 
		best_class = predictions[..., 5:6] 

	# Calculate cell indices 
	cell_indices = ( 
		torch.arange(s) 
		.repeat(predictions.shape[0], 3, s, 1) 
		.unsqueeze(-1) 
		.to(predictions.device) 
	) 

	# Calculate x, y, width and height with proper scaling 
	x = 1 / s * (box_predictions[..., 0:1] + cell_indices) 
	y = 1 / s * (box_predictions[..., 1:2] +
				cell_indices.permute(0, 1, 3, 2, 4)) 
	width_height = 1 / s * box_predictions[..., 2:4] 

	# Concatinating the values and reshaping them in 
	# (BATCH_SIZE, num_anchors * S * S, 6) shape 
	converted_bboxes = torch.cat( 
		(best_class, scores, x, y, width_height), dim=-1
	).reshape(batch_size, num_anchors * s * s, 6) 

	# Returning the reshaped and converted bounding box list 
	return converted_bboxes.tolist()

# Function to plot images with bounding boxes and class labels 
def plot_image(image, boxes): 
	# Getting the color map from matplotlib 
	colour_map = plt.get_cmap("tab20b") 
	# Getting 20 different colors from the color map for 20 different classes 
	colors = [colour_map(i) for i in np.linspace(0, 1, len(class_labels))] 

	# Reading the image with OpenCV 
	img = np.array(image) 
	# Getting the height and width of the image 
	h, w, _ = img.shape 

	# Create figure and axes 
	fig, ax = plt.subplots(1) 

	# Add image to plot 
	ax.imshow(img) 

	# Plotting the bounding boxes and labels over the image 
	for box in boxes: 
		# Get the class from the box 
		class_pred = box[0] 
		# Get the center x and y coordinates 
		box = box[2:] 
		# Get the upper left corner coordinates 
		upper_left_x = box[0] - box[2] / 2
		upper_left_y = box[1] - box[3] / 2

		# Create a Rectangle patch with the bounding box 
		rect = patches.Rectangle( 
			(upper_left_x * w, upper_left_y * h), 
			box[2] * w, 
			box[3] * h, 
			linewidth=2, 
			edgecolor=colors[int(class_pred)], 
			facecolor="none", 
		) 
		
		# Add the patch to the Axes 
		ax.add_patch(rect) 
		
		# Add class name to the patch 
		plt.text( 
			upper_left_x * w, 
			upper_left_y * h, 
			s=class_labels[int(class_pred)], 
			color="white", 
			verticalalignment="top", 
			bbox={"color": colors[int(class_pred)], "pad": 0}, 
		) 

	# Display the plot 
	plt.show()

# Function to save checkpoint 
def save_checkpoint(model, optimizer, filename="my_checkpoint.pth.tar"): 
	print("==> Saving checkpoint") 
	checkpoint = { 
		"state_dict": model.state_dict(), 
		"optimizer": optimizer.state_dict(), 
	} 
	torch.save(checkpoint, filename)


# Function to load checkpoint 
def load_checkpoint(checkpoint_file, model, optimizer, lr): 
	print("==> Loading checkpoint") 
	checkpoint = torch.load(checkpoint_file, map_location=device) 
	model.load_state_dict(checkpoint["state_dict"]) 
	optimizer.load_state_dict(checkpoint["optimizer"]) 

	for param_group in optimizer.param_groups: 
		param_group["lr"] = lr 


# Function to get the data from XML Annotation
def extract_info_from_xml(xml_file):
    root = ET.parse(xml_file).getroot()
    
    # Initialise the info dict 
    info_dict = {}
    info_dict['bboxes'] = []

    # Parse the XML Tree
    for elem in root:
        # Get the file name 
        if elem.tag == "filename":
            info_dict['filename'] = elem.text
            
        # Get the image size
        elif elem.tag == "size":
            image_size = []
            for subelem in elem:
                image_size.append(int(subelem.text))
            
            info_dict['image_size'] = tuple(image_size)
        
        # Get details of the bounding box 
        elif elem.tag == "object":
            bbox = {}
            for subelem in elem:
                if subelem.tag == "name":
                    bbox["class"] = subelem.text
                    
                elif subelem.tag == "bndbox":
                    for subsubelem in subelem:
                        bbox[subsubelem.tag] = int(subsubelem.text)            
            info_dict['bboxes'].append(bbox)
    
    return info_dict

# Dictionary that maps class names to IDs
class_name_to_id_mapping = {"black": 0,
                           "small": 1,
                           "big": 2}

class_id_to_name_mapping = dict(zip( class_name_to_id_mapping.values(), class_name_to_id_mapping.keys()))

# Convert the info dict to the required yolo format and write it to disk
def convert_to_yolov5(info_dict):
    print_buffer = []
    
    # For each bounding box
    for b in info_dict["bboxes"]:
        try:
            class_id = class_name_to_id_mapping[b["class"]]
        except KeyError:
            print("Invalid Class. Must be one from ", class_name_to_id_mapping.keys())
        
        # Transform the bbox co-ordinates as per the format required by YOLO v5
        b_center_x = (b["xmin"] + b["xmax"]) / 2 
        b_center_y = (b["ymin"] + b["ymax"]) / 2
        b_width    = (b["xmax"] - b["xmin"])
        b_height   = (b["ymax"] - b["ymin"])
        
        # Normalise the co-ordinates by the dimensions of the image
        image_w, image_h, image_c = info_dict["image_size"]  
        b_center_x /= image_w 
        b_center_y /= image_h 
        b_width    /= image_w 
        b_height   /= image_h 
        
        #Write the bbox details to the file 
        print_buffer.append("{} {:.3f} {:.3f} {:.3f} {:.3f}".format(class_id, b_center_x, b_center_y, b_width, b_height))
        
    # Name of the file which we have to save 
    save_file_name = os.path.join("labels", info_dict["filename"].replace("png", "txt"))
    
    # Save the annotation to disk
    print("\n".join(print_buffer), file= open(save_file_name, "w"))


def plot_bounding_box(image, annotation_list):
    labels = np.array(annotation_list)
    w, h = image.size
    
    plotted_image = ImageDraw.Draw(image)

    transformed_labels = np.copy(labels)
    transformed_labels[:,[1,3]] = labels[:,[1,3]] * w
    transformed_labels[:,[2,4]] = labels[:,[2,4]] * h 
    
    transformed_labels[:,1] = transformed_labels[:,1] - (transformed_labels[:,3] / 2)
    transformed_labels[:,2] = transformed_labels[:,2] - (transformed_labels[:,4] / 2)
    transformed_labels[:,3] = transformed_labels[:,1] + transformed_labels[:,3]
    transformed_labels[:,4] = transformed_labels[:,2] + transformed_labels[:,4]
    
    for ann in transformed_labels:
        obj_cls, x0, y0, x1, y1 = ann
        plotted_image.rectangle(((x0,y0), (x1,y1)))
        
        plotted_image.text((x0, y0 - 10), class_id_to_name_mapping[(int(obj_cls))])
    
    plt.imshow(np.array(image))
    plt.show()

