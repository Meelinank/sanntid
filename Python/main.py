import socket
import picamera
import time
# Set up camera
camera = picamera.PiCamera()
camera.resolution = (640, 480)
camera.framerate = 24

"""
HOST = '0.0.0.0'  # Listen on all interfaces
PORT = 12345

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Server listening...')
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        data = conn.recv(1024)
        print('Received:', data.decode())
"""

# Set up UDP socket
UDP_IP = "127.0.0.1"
UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Start recording
camera.start_recording('udp://{}:{}'.format(UDP_IP, UDP_PORT), format='h264')

# Stop recording
#camera.stop_recording()

# Close socket
#sock.close()
