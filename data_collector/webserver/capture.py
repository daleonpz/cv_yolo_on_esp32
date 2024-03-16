import os
import subprocess
import sys
import signal

def get_latest_image_number(directory):
    images = [f for f in os.listdir(directory) if f.endswith('.png')]
    if not images:
        return 0
    return max(int(image.split('.')[0]) for image in images)

def download_image(server_url, output_filename): 
    # has to be run twice
    subprocess.run(['python', 'uploader.py', '-s', server_url, '-f', output_filename])
    subprocess.run(['python', 'uploader.py', '-s', server_url, '-f', output_filename])

def signal_handler(sig, frame):
    print('\nExiting...')
    sys.exit(0)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)

    server_url = 'http://192.168.178.53:81/picture'
    output_directory = 'static/uploads/'
    
    last_image_number = get_latest_image_number(output_directory)
    
    print("Press ENTER to download the latest image or press ESC to exit.")

    count = last_image_number + 1

    while True:
        try:
            user_input = input()
            if user_input == '':
                output_filename = f"{count:03d}.png"
                download_image(server_url, output_filename)
                print(f"Downloaded image: {output_filename}")
                count += 1
            elif user_input == '\x1b':
                break  # ESC key
        except KeyboardInterrupt:
            print('\nExiting...')
            sys.exit(0)

