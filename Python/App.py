import socket
import threading
import io
import time
import json
from sphero_sdk import Colors
from picamera import PiCamera
from sphero_sdk import SpheroRvrObserver

class SpheroServer:
    def __init__(self):
        try:
            self.camera = PiCamera()
            self.camera.resolution = (280, 190)  # Lower resolution for faster processing
            self.camera.framerate = 10  # Adjust as needed
        except Exception as e:
            print(f"Failed to initialize camera: {e}")

        try:
            self.rvr = SpheroRvrObserver()
            self.rvr.wake()
        except Exception as e:
            print(f"Failed to initialize Sphero RVR: {e}")

        # Video streaming socket
        self.video_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.video_socket.bind(('10.25.45.112', 8000))
        self.video_socket.listen(1)

        # Command handling socket
        self.command_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.command_socket.bind(('10.25.45.112', 8001))
        self.command_socket.listen(1)

        self.exit_flag = False
        self.last_command = None

    def __del__(self):
        self.command_socket.close()
        self.video_socket.close()

    def start_server(self):
        print("Starting video server...")
        video_thread = threading.Thread(target=self.video_server)
        video_thread.start()

        print("Starting command server...")
        command_thread = threading.Thread(target=self.command_server)
        command_thread.start()

        video_thread.join()
        command_thread.join()

    def video_server(self):
        while not self.exit_flag:
            try:
                client_socket, addr = self.video_socket.accept()
                print("Video client connected:", addr)
                self.process_video_stream(client_socket)
            except Exception as e:
                print(f"Video server error: {e}")
                time.sleep(1)

    def command_server(self):
        while not self.exit_flag:
            try:
                client_socket, addr = self.command_socket.accept()
                print("Command client connected:", addr)
                self.handle_client(client_socket)
            except Exception as e:
                print(f"Command server error: {e}")
                time.sleep(1)

    def process_video_stream(self, client_socket):
        stream = io.BytesIO()
        try:
            for frame in self.camera.capture_continuous(stream, format="jpeg", use_video_port=True, quality=20):
                if self.exit_flag:
                    break
                size = stream.tell()
                try:
                    client_socket.sendall(size.to_bytes(4, byteorder='big'))
                    stream.seek(0)
                    client_socket.sendall(stream.read())
                except (BrokenPipeError, Exception) as e:
                    print(f"Error sending frame: {e}")
                    break
                stream.seek(0)
                stream.truncate()
        finally:
            print("Video streaming stopped")

    def handle_client(self, client_socket):
        try:
            while True:
                message = client_socket.recv(1024).decode('utf-8')
                if message:
                    print(f"Received message: {message}")  # Added for debugging
                    # Parse the outer JSON object
                    outer_data = json.loads(message)
                    # Extract and parse the nested JSON command
                    command_data = json.loads(outer_data["command"])
                    self.control_robot(command_data)  # Pass the parsed command data

                    self.control_robot_light(command_data)
        except Exception as e:
            print(f"Error handling client: {e}")
        finally:
            client_socket.close()
            print("Client socket closed")

    def control_robot(self, data):
        command = data.get("command")
        heading = data.get("heading", 0)
        base_speed = 90  # Base speed for forward and backward movement
        turn_adjustment = 70  # Speed adjustment for turning

        try:
            print(f"Received command: {command}, Heading: {heading}")
            if command == 'AUTO':
                adjusted_speed_left = base_speed - int(heading)
                adjusted_speed_right = base_speed + int(heading)
                self.rvr.raw_motors(1, adjusted_speed_left, 1, adjusted_speed_right)
            elif command == 'F':
                self.rvr.raw_motors(1, base_speed, 1, base_speed)
            elif command == 'B':
                self.rvr.raw_motors(2, base_speed, 2, base_speed)
            elif command == 'FL':
                self.rvr.raw_motors(1, base_speed - turn_adjustment, 1, base_speed + turn_adjustment)
            elif command == 'FR':
                self.rvr.raw_motors(1, base_speed + turn_adjustment, 1, base_speed - turn_adjustment)
            elif command == 'BL':
                self.rvr.raw_motors(2, base_speed - turn_adjustment, 2, base_speed + turn_adjustment)
            elif command == 'BR':
                self.rvr.raw_motors(2, base_speed + turn_adjustment, 2, base_speed - turn_adjustment)
            elif command == 'S':
                self.rvr.raw_motors(0, 0, 0, 0)
            else:
                print(f"Unknown command: {command}")
        except Exception as e:
            print(f"Error in control_robot: {e}")

    def control_robot_light(self, data):
        command = data.get("command")
        try:
            if command != self.last_command:
                if command in ['F', 'B', 'FL', 'FR', 'BL', 'BR']:
                    self.rvr.led_control.set_all_leds_color(color=Colors.yellow)
                elif command == 'S':
                    self.rvr.led_control.set_all_leds_color(color=Colors.red)
                elif command == 'AUTO':
                    self.rvr.led_control.set_all_leds_color(color=Colors.green)
                else:
                    self.rvr.led_control.set_all_leds_color(color=Colors.purple)
                self.last_command = command
        except Exception as e:
            print(f"Error in control_robot_light: {e}")

    def stop(self):
        self.exit_flag = True
        try:
            self.rvr.close()
        except Exception as e:
            print(f"Error stopping RVR: {e}")

if __name__ == "__main__":
    server = SpheroServer()
    try:
        server.start_server()
    finally:
        server.stop()
