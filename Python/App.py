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

        # Command handling socket
        self.sensor_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sensor_socket.bind(('10.25.45.112', 8002))
        self.sensor_socket.listen(1)

        self.exit_flag              = False
        self.command                = None
        self.heading                = None
        self.speed                  = None
        self.rvrBatteryPercentage   = None
        self.rvrColor               = None
        self.rvrTemps               = None
        self.rvrAmbientLight        = None
        self.rvrEncoders            = None
        self.rvrX                   = None
        self.rvrY                   = None
        self.rvrZ                   = None
        self.rvrPitch               = None
        self.rvrYaw                 = None
        self.rvrRoll                = None
        self.last_command           = None
    def __del__(self):
        self.command_socket.close()
        self.video_socket.close()
        self.sensor_socket.close()
    def start_server(self):
        print("Starting video   server...")
        video_thread = threading.Thread(target=self.video_server)
        video_thread.start()

        print("Starting command server...")
        command_thread = threading.Thread(target=self.command_server)
        command_thread.start()

        print("Starting sensor  server...")
        sensor_thread = threading.Thread(target=self.sensor_server)
        sensor_thread.start()

        #video_thread.join()
        #command_thread.join()
        #sensor_thread.join()
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
                self.last_command = self.command
            except Exception as e:
                print(f"Command server error: {e}")
                time.sleep(1)      
    def sensor_server(self):    
        while not self.exit_flag:
            try:
                client_socket, addr = self.sensor_socket.accept()
                print("Sensor client connected:", addr)
                self.sensor_updater(client_socket)
            except Exception as e:
                print(f"Sensor server error: {e}")
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
            while not self.exit_flag:
                message = client_socket.recv(1024).decode('utf-8')
                if not message:
                    break
                print(f"Received message: {message}")
                try:
                    command_data = json.loads(message)
                    self.command = command_data.get("command", "S")
                    self.heading = command_data.get("heading", 0)
                    self.speed   = command_data.get("speed", 1)
                    self.control_robot()
                    self.control_robot_light()
                except json.JSONDecodeError:
                    print(f"Received bad message: {self.command}, Heading: {self.heading}, Speed: {self.speed}")     
        except Exception as e:
            print(f"Error handling client: {e}")
        finally:
            client_socket.close()
            print("Client socket closed")
    def control_robot(self):
        base_speed      = 101
        turn_adjustment = 70
        try:
            if self.command == 'AUTO':
                adjusted_speed_left  = int((base_speed - self.heading)*self.speed)
                adjusted_speed_right = int((base_speed + self.heading)*self.speed)
                self.rvr.raw_motors(1, adjusted_speed_left, 1, adjusted_speed_right)
            elif self.command == 'F':
                self.rvr.raw_motors(1, int((base_speed)*self.speed), 1, int((base_speed)*self.speed))
            elif self.command == 'B':
                self.rvr.raw_motors(2, int((base_speed)*self.speed), 2, int((base_speed)*self.speed))
            elif self.command == 'FL':
                self.rvr.raw_motors(1, int((base_speed - turn_adjustment)*self.speed), 1, int((base_speed + turn_adjustment)*self.speed))
            elif self.command == 'FR':
                self.rvr.raw_motors(1, int((base_speed + turn_adjustment)*self.speed), 1, int((base_speed - turn_adjustment)*self.speed))
            elif self.command == 'BL':
                self.rvr.raw_motors(2, int((base_speed - turn_adjustment)*self.speed), 2, int((base_speed + turn_adjustment)*self.speed))
            elif self.command == 'BR':
                self.rvr.raw_motors(2, int((base_speed + turn_adjustment)*self.speed), 2, int((base_speed - turn_adjustment)*self.speed))
            elif self.command == 'S':
                self.rvr.raw_motors(0, 0, 0, 0)
            else:
                print(f"Unknown command: {self.command}")
        except Exception as e:
            print(f"Error in control_robot: {e}")  
    def sensor_updater(self, client_socket):
        while not self.exit_flag:
            try:
                self.rvr.enable_color_detection(is_enabled=True)
                self.rvr.sensor_control.add_sensor_data_handler(service=RvrStreamingServices.color_detection, handler=self.rvrColor_handler)
                self.rvr.sensor_control.add_sensor_data_handler(service=RvrStreamingServices.imu, handler=self.rvrIMU_handler)
                self.rvr.sensor_control.add_sensor_data_handler(service=RvrStreamingServices.ambient_light, handler=self.rvrAmbientLight_handler)
                # self.rvr.sensor_control.add_sensor_data_handler(service=RvrStreamingServices.encoders, handler=self.rvrEncoders_handler)
                self.rvr.get_battery_percentage(handler=self.rvrBatteryPercentage_handler)
                self.rvr.sensor_control.start(interval=100)

                while not self.exit_flag:
                    sensor_data = {
                        "x": self.rvrX,
                        "y": self.rvrY,
                        "z": self.rvrZ,
                        "pitch": self.rvrPitch,
                        "yaw": self.rvrYaw,
                        "roll": self.rvrRoll,
                        "LightSensor": self.rvrColor,
                        "AmbientLight": self.rvrAmbientLight
                        # include any other sensor data here
                    }
                    sensor_json = json.dumps(sensor_data) + "\n"  # Add newline character
                    print(f"Sending sensor data:"{sensor_json})
                    client_socket.sendall(sensor_json.encode())
                    #time.sleep(1)  # Adjust the frequency of updates as needed

            except Exception as e:
                print(f"Error in sensor_updater: {e}")
                time.sleep(1)
    def control_robot_light(self):
        try:
            if self.command != self.last_command:
                if self.command in ['F', 'B', 'FL', 'FR', 'BL', 'BR']:
                    self.rvr.led_control.set_all_leds_color(color=Colors.yellow)
                elif self.command == 'S':
                    self.rvr.led_control.set_all_leds_color(color=Colors.red)
                elif self.command == 'AUTO':
                    self.rvr.led_control.set_all_leds_color(color=Colors.green)
                else:
                    self.rvr.led_control.set_all_leds_color(color=Colors.purple)
        except Exception as e:
            print(f"Error in control_robot_light: {e}")
    def stop(self):
        self.exit_flag = True
        try:
            self.rvr.close()
        except Exception as e:
            print(f"Error stopping RVR: {e}")
    #sensor handlers
    def rvrBatteryPercentage_handler(self,battery_percentage):
        self.rvrBattery = battery_percentage
    def rvrColor_handler(self,color_data):
        self.rvrColor = color_data
    def rvrIMU_handler(self,imu_data):
        self.rvrX     = self.get_nested(imu_data, "Accelerometer", "X")
        self.rvrY     = self.get_nested(imu_data, "Accelerometer", "Y")
        self.rvrZ     = self.get_nested(imu_data, "Accelerometer", "Z")
        self.rvrPitch = self.get_nested(imu_data, "IMU"          , "Pitch")
        self.rvrYaw   = self.get_nested(imu_data, "IMU"          , "Yaw"  )
        self.rvrRoll  = self.get_nested(imu_data, "IMU"          , "Roll" )
    def rvrAmbientLight_handler(self,ambient_light_data):
        self.rvrAmbientLight = self.get_nested(ambient_light_data, "AmbientLight", "Light")
    def rvrEncoders_handler(self,encoder_data):
        self.rvrEncoders = encoder_data 
    def get_nested(self, dictionary, *keys):
        if keys and dictionary:
            element  = keys[0]
            if element:
                value = dictionary.get(element)
                return value if len(keys) == 1 else get_nested(value, *keys[1:])
        return None
if __name__ == "__main__":
    server = SpheroServer()
    try:
        server.start_server()
    finally:
        server.stop()