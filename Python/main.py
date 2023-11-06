import io
import socket
import struct
import time
import picamera

# Create a socket server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('10.25.45.112', 8000))
server_socket.listen(0)

# Accept a single connection and make a file-like object out of it
connection = server_socket.accept()[0].makefile('wb')

try:
    with picamera.PiCamera() as camera:
        camera.resolution = (640, 480)  # Adjust resolution as needed
        camera.framerate = 30  # Adjust framerate as needed

        time.sleep(2)  # Warm-up time for the camera

        stream = io.BytesIO()
        for _ in camera.capture_continuous(stream, 'jpeg', use_video_port=True):
            connection.write(struct.pack('<L', stream.tell()))
            connection.flush()
            stream.seek(0)
            connection.write(stream.read())
            stream.seek(0)
            stream.truncate()

finally:
    connection.close()
    server_socket.close()