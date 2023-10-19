import io
import socket
import struct
import time
import picamera

# Set up UDP socket
UDP_IP = "192.168.240.212"
UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)

# Set up PiCamera
camera = picamera.PiCamera()
camera.resolution = (640, 480)
camera.framerate = 24

# Start recording video
stream = io.BytesIO()
for _ in camera.capture_continuous(stream, 'jpeg', use_video_port=True):
    # Get frame data
    frame = stream.getvalue()

    # Send frame over UDP socket
    sock.sendto(frame, (UDP_IP, UDP_PORT))

    # Reset stream for next frame
    stream.seek(0)
    stream.truncate()

    # Wait for next frame
    time.sleep(0.01)