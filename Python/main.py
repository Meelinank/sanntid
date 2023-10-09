import asyncio
import websockets
import cv2
import numpy as np
import socket
from PiServoHat_Py import PiServoHat

async def receive_data(websocket, path, servo_hat, tcp_socket):
    async for message in websocket:
        # Parse incoming message as sensor data
        sensor_data = message.split(",")
        sensor_data = [float(x) for x in sensor_data]

        # Send sensor data to servos
        servo_angles = [int(x) for x in servo_hat.map(sensor_data)]
        servo_hat.move_servo_angle(0, servo_angles[0])
        servo_hat.move_servo_angle(1, servo_angles[1])

        # Send servo angles back to client over TCP
        tcp_socket.send(",".join(str(x) for x in servo_angles).encode())

async def webcam_stream(udp_socket):
    # Capture frames from webcam and stream over UDP
    cap = cv2.VideoCapture(0)
    while True:
        ret, frame = cap.read()
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
        _, jpeg_frame = cv2.imencode('.jpg', frame, encode_param)
        udp_socket.sendto(jpeg_frame, ("localhost", 5000))

async def main():
    # Initialize PiServoHat
    servo_hat = PiServoHat()

    # Set up TCP socket for sensor data
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp_socket.bind(("localhost", 8888))
    tcp_socket.listen(1)

    # Set up UDP socket for webcam stream
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Set up WebSocket server
    async with websockets.serve(lambda ws, path: receive_data(ws, path, servo_hat, tcp_socket), "localhost", 8765):
        # Start webcam stream in a separate task
        asyncio.create_task(webcam_stream(udp_socket))

        # Accept incoming TCP connections and send sensor data
        while True:
            conn, addr = tcp_socket.accept()
            async with conn:
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    await asyncio.sleep(0.1)  # simulate processing time
                    conn.send(data)

asyncio.run(main())