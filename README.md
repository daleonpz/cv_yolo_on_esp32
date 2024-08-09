# TensorFlow Lite Micro for Espressif Chipsets

[![Component Registry](https://components.espressif.com/components/espressif/esp-tflite-micro/badge.svg)](https://components.espressif.com/components/espressif/esp-tflite-micro)

- As per TFLite Micro guidelines for vendor support, this repository has the `esp-tflite-micro` component and the examples needed to use Tensorflow Lite Micro on Espressif Chipsets (e.g., ESP32) using ESP-IDF platform.
- The base repo on which this is based can be found [here.](https://github.com/tensorflow/tflite-micro)

## Build Status

|   Build Type  |  Status    |
| -----------   |  --------- |
| Examples Build | [![CI](https://github.com/espressif/esp-tflite-micro/actions/workflows/ci.yml/badge.svg)](https://github.com/espressif/esp-tflite-micro/actions/workflows/ci.yml)

## How to Install

### Install the ESP IDF

Follow the instructions of the
[ESP-IDF get started guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)
to setup the toolchain and the ESP-IDF itself.

The next steps assume that this installation is successful and the
[IDF environment variables are set](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#step-4-set-up-the-environment-variables). Specifically,
* the `IDF_PATH` environment variable is set
* the `idf.py` and Xtensa-esp32 tools (e.g., `xtensa-esp32-elf-gcc`) are in `$PATH`

## Using the component

Run the following command in your ESP-IDF project to install this component:

```bash
idf.py add-dependency "esp-tflite-micro"
```

## Building the example

To get the example, run the following command:

```bash
idf.py create-project-from-example "esp-tflite-micro:<example_name>"
```

Note:
  - If you have cloned the repo, the examples come as the part of the clone. Simply go to the example directory (`examples/<example_name>`) and build the example.

Available examples are:
 - hello_world
 - micro_speech
 - person_detection

Set the IDF_TARGET (For ESP32-S3 target, minimum IDF version `release/v4.4` is needed)

```bash
idf.py set-target esp32s3
```

To build the example, run:

```bash
idf.py build
```

### Load and run the example

To flash (replace `/dev/ttyUSB0` with the device serial port):
```bash
idf.py --port /dev/ttyUSB0 flash
```

Monitor the serial output:
```bash
idf.py --port /dev/ttyUSB0 monitor
```

Use `Ctrl+]` to exit.

The previous two commands can be combined:
```bash
idf.py --port /dev/ttyUSB0 flash monitor
```

  - Please follow example READMEs for more details.

## ESP-NN Integration
[ESP-NN](https://github.com/espressif/esp-nn) contains optimized kernel implementations for kernels used in TFLite Micro. The library is integrated with this repo and gets compiled as a part of every example. Additional information along with performance numbers can be found [here](https://github.com/espressif/esp-nn#performance).

### Performance Comparison

A quick summary of ESP-NN optimisations, measured on various chipsets:

|   Target  |   TFLite Micro Example  | without ESP-NN  | with ESP-NN | CPU Freq  |
| --------- | ----------------------- | --------------- | ----------- |-----------|
| ESP32-S3  |   Person Detection      |     2300ms      |     54ms    |  240MHz   |
| ESP32     |   Person Detection      |     4084ms      |    380ms    |  240MHz   |
| ESP32-C3  |   Person Detection      |     3355ms      |    426ms    |  160MHz   |

Note:
  - The above is time taken for execution of the `invoke()` call
  - Internal memory used

Detailed kernelwise performance can be found [here](https://github.com/espressif/esp-nn).

## Sync to latest TFLite Micro upstream

As per the upstream repository policy, the tflite-lib is copied into the components directory in this repository. We keep updating this to the latest upstream version from time to time. Should you, in any case, wish to update it locally, you may run the `scripts/sync_from_tflite_micro.sh` script.

## Contributing
- If you find an issue in these examples, or wish to submit an enhancement request, please use the Issues section on Github.
- For ESP-IDF related issues please use [esp-idf](https://github.com/espressif/esp-idf) repo.
- For TensorFlow related information use [tflite-micro](https://github.com/tensorflow/tflite-micro) repo.

## License

This component and the examples are provided under Apache 2.0 license, see [LICENSE](LICENSE.md) file for details.

TensorFlow library code and third_party code contains their own license specified under respective [repos](https://github.com/tensorflow/tflite-micro).

# How to 
this project consist in two main parts:
Data collection and model training
Model deployment on ESP32

Install the ESP IDF
Follow the instructions of the ESP-IDF get started guide to setup the toolchain and the ESP-IDF itself. [link](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html)

## Data collection and model training
### Data collection
Go to the data_collector folder, there are three folders:
- datasets: contains the scripts to split the dataset and the dataset itself
- esp32: contains the code to run on the ESP32
- webserver: contains the code to run the webserver
- labels.txt: contains the labels of the dataset used to label the images in [makesense.ai](https://www.makesense.ai/)

```bash
$ tree
├── datasets
│   ├── images
│   ├── labels
│   ├── quant_test_model.py
│   ├── split_dataset.py
│   └── utils.py
├── esp32
│   ├── build
│   ├── CMakeLists.txt
│   ├── dependencies.lock
│   ├── main
│   ├── managed_components
│   ├── sdkconfig
│   ├── sdkconfig.defaults
│   └── sdkconfig.old
├── labels.txt
└── webserver
    ├── capture.py
    ├── static
    ├── templates
    ├── uploader.py
    └── webserver.py
```

#### Flash the ESP32
Go to the esp32 folder, start the ESP32 virtual environment and clean the build folder.

```bash
cd esp32/
get_idf
idf.py fullclean
```

Set up your WiFi with `idf.py menuconfig` and go to the `Example Connection Configuration` and set your WiFi SSID and password.

Once you have set up your WiFi, you can build and flash the application with the following commands:

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor
```

You would be able to see the IP address of the ESP32, you can access the webserver with the IP address of the ESP32.

```bash
I (4180) WIFI_MODULE: got ip:X.X.X.X
I (4184) WIFI_MODULE: connected to ap <CONFIGURED_SSID> password:<CONFIGURED_PASSWORD>
I (4193) gpio: GPIO[13]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
I (4202) gpio: GPIO[14]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
I (4211) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:2 
I (4222) cam_hal: cam init ok
I (4225) sccb: pin_sda 18 pin_scl 23
I (4228) sccb: sccb_i2c_port=1
I (4246) camera: Detected camera at address=0x30
I (4260) camera: Detected OV2640 camera
I (4260) camera: Camera PID=0x26 VER=0x42 MIDL=0x7f MIDH=0xa2
I (4518) esp32 ll_cam: node_size: 3072, nodes_per_line: 1, lines_per_node: 4, dma_half_buffer_min:  3072, dma_half_buffer: 12288,lines_per_half_buffer: 16, dma_buffer_size: 24576, image_size: 18432
I (4525) cam_hal: buffer_size: 24576, half_buffer_size: 12288, node_buffer_size: 3072, node_cnt: 8, total_cnt: 1
I (4536) cam_hal: Allocating 18432 Byte frame buffer in OnBoard RAM
I (4543) cam_hal: cam config ok
I (4547) ov2640: Set PLL: clk_2x: 0, clk_div: 3, pclk_auto: 1, pclk_div: 8
I (4780) STREAM_SERVER: Starting picture server on port: '81'
```

#### Webserver
The webserver is a simple application that receives the data from the ESP32 and saves it in a folder called uploads. To run the webserver, you need to install the requirements and run the python script `webserver.py` and in another terminal run the python script `capture.py` every time you want to capture a new image. The images will be saved in the uploads folder following the pattern `XXX.png` where `XXX` is the number of the image, if you want to change the pattern, you can change the `capture.py` script `output_filename = f"{count:03d}.png"`.

```bash
$ cd webserver/
$ tree
.
├── capture.py
├── static
│   └── uploads
├── templates
│   └── index.html
├── uploader.py
└── webserver.py
```

To start capturing the images, first you need to set the environment variable `ESP32_SERVER_URL` with the IP address of the ESP32, then run the following commands:

```bash
export ESP32_SERVER_URL=http://<ESP32_IP>:81
```

Start the local webserver in a new terminal:

```bash
python3 webserver.py
```

In another terminal, start capturing the images:

```bash
python3 capture.py
```

You should see the following output:

```bash
Press ENTER to download the latest image or press ESC to exit.
```

now you can press `ENTER` to capture a new image, the image will be saved in the uploads folder.

```bash
### capture.py output
Image uploaded successfully to your server!
Image uploaded successfully to your server!
Downloaded image: 001.png

### webserver.py output
127.0.0.1 - - [09/Aug/2024 18:43:45] "POST /upload HTTP/1.1" 302 -
127.0.0.1 - - [09/Aug/2024 18:43:45] "GET / HTTP/1.1" 200 -
127.0.0.1 - - [09/Aug/2024 18:43:45] "POST /upload HTTP/1.1" 302 -
127.0.0.1 - - [09/Aug/2024 18:43:45] "GET / HTTP/1.1" 200 -

### esp32 output
I (478946) STREAM_SERVER: RAW: 18KB 16ms
I (479253) STREAM_SERVER: RAW: 18KB 22ms
```

As mentioned before, your images will be saved in the uploads folder.

```bash
$ ls static/uploads/
001.png
```

### Model training

#### Labeling
For data labeling I used [makesense.ai](https://www.makesense.ai/), you don't need an account to start labeling your images. Once you have labeled your images, you can download the dataset in the YOLO format.

- Open makesense.ai and click on `Get Started`. 
- Upload the images from `data_collector/webserver/static/uploads/` to the makesense.ai platform and click on `Object Detection`.
- Click on "Load labels from file" and select the `labels.txt` file from the `data_collector` folder.
- You should see "3 labels found", if so, click on "Start project".

This is an example of how the interface should look like:

![Interface](readme_extras/labeling_makesense.png)

Now you can start labeling your images, once you have labeled all the images, click on "Actions>Export Annotations" and select the YOLO format.
Here is a example of the YOLO format: 

```bash
0 0.509715 0.331606 0.190415 0.272021
1 0.591321 0.654793 0.107513 0.224093
```

the first number is the label, the next four numbers are the bounding box coordinates in the format `x_center y_center width height` scaled to the image size. For example, if the image size is 640x480, the bounding box `0.509715 0.331606 0.190415 0.272021` would be `x_center=0.509715*640=326.29 y_center=0.331606*480=159.14 width=0.190415*640=121.86 height=0.272021*480=130.57`.





