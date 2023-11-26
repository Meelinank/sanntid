import threading
import socket
import json
from picamera import PiCamera
from sphero_sdk import SpheroRvrObserver
import time
class RobotController:
    def __init__(self, ip_address, camera_port, sensor_port, command_port):
        self.ip_address   = "10.25.45.112"
        self.camera_port  = 8000
        self.sensor_port  = 8001
        self.command_port = 8002
        
        self.camera_thread = threading.Thread(target=self.camera_thread_function)
        self.sensor_thread = threading.Thread(target=self.sensor_thread_function)
        self.command_thread = threading.Thread(target=self.command_thread_function)
        
        self.rvr_command = SpheroRvrObserver()

    def start(self):
        self.rvr_command.wake()
        self.camera_thread.start()
        self.sensor_thread.start()
        self.command_thread.start()

    def camera_thread_function(self):
        camera = PiCamera()
        camera_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        camera_socket.bind((self.ip_address, self.camera_port))
        camera_socket.listen(1)
        
        while True:
            client, _ = camera_socket.accept()
            try:
                camera.capture(client.makefile('wb'), 'jpeg')
            except Exception as e:
                print(f"Camera error: {e}")
            finally:
                client.close()

    def sensor_thread_function(self):
        
        sensor_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sensor_socket.bind((self.ip_address, self.sensor_port))
        sensor_socket.listen(1)
        
        while True:
            client, _ = sensor_socket.accept()
            try:
                sensor_data = {
                "Battery"     : self.rvr_command.get_battery_percentage(), 
                "MotorTemp"   : self.rvr_command.get_motor_temperature(),
                "LightSensor" : self.rvr_command.get_rgbc_sensor_values(),
                "AmbientLight": self.rvr_command.get_ambient_light_sensor_value()}
                
                sensor_json = json.dumps(sensor_data)
                client.send(sensor_json.encode())
                
            except Exception as e:
                print(f"Sensor error: {e}")
            finally:
                client.close()

    def command_thread_function(self):
        
        command_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        command_socket.bind((self.ip_address, self.command_port))
        command_socket.listen(1)
        
        while True:
            client, _ = command_socket.accept()
            try:
                command_data = client.recv(1024).decode()
                command_json = json.loads(command_data)
                self.execute_command(command_json)
            except Exception as e:
                print(f"Command error: {e}")
            finally:
                client.close()

    def execute_command(self, command_json):
        #TODO Implement Sphero SDK commands here based on the received JSON data
        pass

robot_controller = RobotController()
robot_controller.start()

while True:
    time.sleep(0.001)
