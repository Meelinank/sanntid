import socket
import threading
import io
import time
import json
from sphero_sdk import Colors
from picamera import PiCamera
from sphero_sdk import SpheroRvrObserver
from sphero_sdk import RvrStreamingServices

class SpheroServer:
    def __init__(self):
        self.init_camera()
        self.init_rvr()
        self.setup_sockets()
        self.setup_threading_variables()
        self.init_sensor_control()

    def init_camera(self):
        try:
            self.camera = PiCamera()
            self.camera.resolution = (350, 250)
            self.camera.framerate = 10
        except Exception as e:
            print(f"Failed to initialize camera: {e}")

    def init_rvr(self):
        try:
            self.rvr = SpheroRvrObserver()
            self.rvr.wake()
            #self.rvr.enable_color_detection(is_enabled=True)
        except Exception as e:
            print(f"Failed to initialize Sphero RVR: {e}")

    def setup_sockets(self):
        self.video_socket   = self.create_socket(8000)
        self.command_socket = self.create_socket(8001)
        self.sensor_socket  = self.create_socket(8002)

    def create_socket(self, port):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('10.25.45.112', port))
        s.listen(1)
        return s

    def init_sensor_control(self):
        try:
            # Add sensor data handlers
            
            self.rvr.sensor_control.add_sensor_data_handler(
                service=RvrStreamingServices.color_detection,
                handler=self.rvrColor_handler
            )
            self.rvr.sensor_control.add_sensor_data_handler(
                service=RvrStreamingServices.imu,
                handler=self.rvrIMU_handler
            )
            self.rvr.sensor_control.add_sensor_data_handler(
                service=RvrStreamingServices.ambient_light,
                handler=self.rvrAmbientLight_handler
            )

            # Request battery percentage
            self.rvr.get_battery_percentage(handler=self.rvrBatteryPercentage_handler)
            
            # Start sensor data streaming
            
            self.rvr.sensor_control.start(interval=1000)
        except Exception as e:
            print(f"Failed to initialize sensor control: {e}")

    def setup_threading_variables(self):
        self.exit_flag      = False
        self.command        = None
        self.sensor_data    = {}
        self.direction      = None
        self.heading        = None
        self.speed          = None
        self.lock           = threading.Lock()  # Add a lock for thread-safe operations


    def start_server(self):
        print("Starting video server...")
        video_thread = threading.Thread(target=self.video_server)
        video_thread.start()

        print("Starting command server...")
        command_thread = threading.Thread(target=self.command_server)
        command_thread.start()

        print("Starting sensor server...")
        sensor_thread = threading.Thread(target=self.sensor_server)
        sensor_thread.start()

        print("Starting robot control...")
        robot_thread = threading.Thread(target=self.control_robot)
        robot_thread.start()

        command_thread.join()
        sensor_thread.join()
        robot_thread.join()

    def video_server(self):
        while not self.exit_flag:
            try:
                client_socket, addr = self.video_socket.accept()
                print("Video client connected:", addr)
                self.process_video_stream(client_socket)
            except Exception as e:
                print(f"Video server error: {e}")
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
        except Exception as e:
            print(f"Error capturing video: {e}")
        finally:
            client_socket.close()
    def command_server(self):
        while not self.exit_flag:
            try:
                client_socket, addr = self.command_socket.accept()
                print("Command client connected:", addr)
                self.handle_client(client_socket)
            except Exception as e:
                print(f"Command server error: {e}")
                time.sleep(1)

    def handle_client(self, client_socket):
        while not self.exit_flag:
            message = client_socket.recv(1024).decode('utf-8')
            if not message:
                break
            print(f"Received message: {message}")
            try:
                data = json.loads(message)
                with self.lock:  # Use lock to safely update shared variables
                    self.command = data.get("command", "MANUAL")
                    self.direction = data.get("direction", "S")
                    self.heading = max(-100, min(100, data.get("heading", 0)))
                    self.speed = data.get("speed", 1)
                print(f"Decoded command: {self.command}, Direction: {self.direction}, Heading: {self.heading}, Speed: {self.speed}")
            except json.JSONDecodeError:
                print(f"Received bad message: {self.command}, Heading: {self.heading}, Speed: {self.speed}")

    def sensor_server(self):
        while not self.exit_flag:
            try:
                client_socket, addr = self.sensor_socket.accept()
                print("Sensor client connected:", addr)
                self.sensor_updater(client_socket)
            except Exception as e:
                print(f"Sensor server error: {e}")
                time.sleep(1)

    def sensor_updater(self, client_socket):
        while not self.exit_flag:
            try:
                with self.lock:
                    sensor_json = json.dumps(self.sensor_data) + "\n"
                client_socket.sendall(sensor_json.encode())

                time.sleep(0.01)
            except Exception as e:
                print(f"Error in sensor_updater: {e}")
                break
        client_socket.close()

    def control_robot(self):
        base_speed = 101
        turn_adjustment = 70
        try:
            while not self.exit_flag:
                with self.lock:
                    current_command   = self.command
                    current_direction = self.direction
                    current_heading   = self.heading
                    current_speed     = self.speed

                if current_command is None:
                    continue

                if current_command == 'AUTO':
                    adjusted_speed_left  = int((base_speed - current_heading) * current_speed)
                    adjusted_speed_right = int((base_speed + current_heading) * current_speed)
                    self.rvr.raw_motors(1, adjusted_speed_left, 1, adjusted_speed_right)
                elif current_direction == 'F':
                    self.rvr.raw_motors(1, int(base_speed * current_speed), 1, int(base_speed * current_speed))
                elif current_direction == 'B':
                    self.rvr.raw_motors(2, int(base_speed * current_speed), 2, int(base_speed * current_speed))
                elif current_direction == 'L':
                    self.rvr.raw_motors(1, int((base_speed - turn_adjustment) * current_speed), 1,int((base_speed + turn_adjustment) * current_speed))
                elif current_direction == 'R':
                    self.rvr.raw_motors(1, int((base_speed + turn_adjustment) * current_speed), 1,int((base_speed - turn_adjustment) * current_speed))
                elif current_direction == 'S':
                    self.rvr.raw_motors(0, 0, 0, 0)
                else:
                    print(f"Unknown command: {current_command}")

                time.sleep(0.01)
        except Exception as e:
            print(f"Error in control_robot: {e}")

    def rvrBatteryPercentage_handler(self, battery_percentage):
        new_data = battery_percentage.get("percentage")
        with self.lock:
            self.sensor_data["Battery"] = new_data

    def rvrColor_handler(self, color_data):
        new_data = {
            "ColorSensor": {
                "R": color_data.get("ColorDetection", {}).get("R"),
                "G": color_data.get("ColorDetection", {}).get("G"),
                "B": color_data.get("ColorDetection", {}).get("B")
            }
        }
        with self.lock:
            self.sensor_data["ColorSensor"] = new_data

    def rvrIMU_handler(self, imu_data):
        new_data = {
            "IMU": {
                "Pitch": imu_data.get("IMU",            {}).get("Pitch"),
                "Yaw"  : imu_data.get("IMU",            {}).get("Yaw"  ),
                "Roll" : imu_data.get("IMU",            {}).get("Roll" ),
            }
        }
        with self.lock:
            self.sensor_data["IMU"] = new_data

    def rvrAmbientLight_handler(self, ambient_light_data):
        new_data = ambient_light_data.get("AmbientLight", {}).get("Light")
        with self.lock:
            self.sensor_data["AmbientLight"] = new_data

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
""""

                "X"    : imu_data.get("Accelerometer",  {}).get("X"    ),
                "Y"    : imu_data.get("Accelerometer",  {}).get("Y"    ),
                "Z"    : imu_data.get("Accelerometer",  {}).get("Z"    )"""