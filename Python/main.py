import socket
import picamera
import time
import io

# Set up camera
camera = picamera.PiCamera()
camera.resolution = (640, 480)
camera.framerate = 24

# Set up UDP socket
UDP_IP = "127.0.0.1"
UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Start recording
stream = io.BytesIO()
camera.start_recording(stream, format='h264')
try:
    while True:
        # Wait for the buffer to fill up
        time.sleep(1)

        # Send the buffer over UDP
        sock.sendto(stream.getvalue(), (UDP_IP, UDP_PORT))

        # Reset the buffer for the next frame
        stream.seek(0)
        stream.truncate()

finally:
    # Stop recording
    camera.stop_recording()

    # Close socket
    sock.close()