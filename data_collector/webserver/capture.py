# Description: This script downloads the latest image from the ESP32 server
# and saves it to the static/uploads directory. The script will continue to
# download images until the user presses the ESC key.

import os
import subprocess
import sys
import signal

import logging

logging.getLogger().setLevel(logging.INFO)
logger = logging.getLogger(__name__)


def get_latest_image_number(directory):
    """Get the latest image number in the directory."""
    images = [f for f in os.listdir(directory) if f.endswith(".png")]
    if not images:
        return 0
    return max(int(image.split(".")[0]) for image in images)


def download_image(server_url, output_filename):
    """Download the latest image from the server."""
    # has to be run twice to get the latest image
    subprocess.run(["python", "uploader.py", "-s", server_url, "-f", output_filename])
    subprocess.run(["python", "uploader.py", "-s", server_url, "-f", output_filename])


def signal_handler(sig, frame):
    logger.info("Exiting...")
    sys.exit(0)


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    server_url = os.environ.get("ESP32_SERVER_URL")
    if server_url is None:
        logger.error("Please set the ESP32_SERVER_URL environment variable.")
        sys.exit(1)

    server_url = server_url + "/picture"
    output_directory = "static/uploads/"

    last_image_number = get_latest_image_number(output_directory)
    logger.info("Press ENTER to download the latest image or press ESC to exit.")

    count = last_image_number + 1

    while True:
        try:
            user_input = input()
            if user_input == "":
                output_filename = f"{count:03d}.png"
                download_image(server_url, output_filename)
                logger.info(f"Downloaded image: {output_filename}")
                count += 1
            elif user_input == "\x1b":
                break  # ESC key
        except KeyboardInterrupt:
            logger.info("Exiting...")
            sys.exit(0)
