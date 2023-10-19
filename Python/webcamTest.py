import cv2
import numpy as np
import socket

# Set up UDP socket
UDP_IP = "192.168.240.212"
UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# Set up OpenCV window
cv2.namedWindow("Video", cv2.WINDOW_NORMAL)

# Receive and display video stream
while True:
    # Receive frame from UDP socket
    data, addr = sock.recvfrom(65507)

    # Convert frame to numpy array
    frame = np.frombuffer(data, dtype=np.uint8)

    # Decode frame
    frame = cv2.imdecode(frame, cv2.IMREAD_COLOR)

    # Display frame
    cv2.imshow("Video", frame)

    # Exit if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Close socket and OpenCV window
sock.close()
cv2.destroyAllWindows()