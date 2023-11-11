import io
import socket
import struct
from datetime import time #might be redundant?
import time
import sys
import picamera
import threading
import json
import pi_servo_hat

sys.path.append('/home/pi/sphero-sdk-raspberrypi-python')   #path to sphero-sdk-raspberrypi-python library on pi
from sphero_sdk import SpheroRvrObserver
from sphero_sdk import Colors                               #needed for rvr color commands

# initialize
SERVER_IP = "10.25.45.112"
SERVER_PORT_WEBCAM  = 8000
SERVER_PORT_CONTROL = 8001
SERVER_PORT_LOGGING = 8002
rvr = SpheroRvrObserver()
servo = pi_servo_hat.PiServoHat()
for i in range(0, 16):
    servo.position(i, 0)

def webcamStream():
    server_socket = socket.socket()
    server_socket.bind((SERVER_IP, SERVER_PORT_WEBCAM))
    server_socket.listen(0)
    connection = server_socket.accept()[0].makefile('wb')
    try:
        with picamera.PiCamera() as camera:
            camera.resolution = (620, 480)
            camera.framerate = 24
            camera.start_preview()
            time.sleep(2)
            
            # Note the start time and construct a stream to hold image data
            start = time.time()
            stream = io.BytesIO()
            
            for foo in camera.capture_continuous(stream, 'jpeg', use_video_port=True):
                # Write the length of the capture to the stream and flush to ensure it actually gets sent
                connection.write(struct.pack('<L', stream.tell()))
                connection.flush()
                # Rewind the stream and send the image data over the wire
                stream.seek(0)
                connection.write(stream.read())
                if time.time() - start > 60:
                    break
                # Reset the stream for the next capture
                stream.seek(0)
                stream.truncate()
        # Write a length of zero to the stream to signal we're done
        connection.write(struct.pack('<L', 0))
    finally:
        connection.close()
        server_socket.close()

def controllerCom():
    server_socket = socket.socket()
    server_socket.bind((SERVER_IP, SERVER_PORT_CONTROL))
    server_socket.listen(0)
    connection  = server_socket.accept()[0].makefile('wb') #might not need to makefile?

    try:
        while True:
            orders = server_socket.recv(1024)
            orders = orders.decode("utf-8")
    finally:
        connection.close()
        server_socket.close()

def sensorDataCom():
    server_socket = socket.socket()
    server_socket.bind((SERVER_IP, SERVER_PORT_LOGGING))
    server_socket.listen(0)
    connection  = server_socket.accept()[0].makefile('wb') #might not need to makefile?
    try:
        while True:
                rvrTemps        = rvr.get_motor_temperature()
                rvrLightSensor  = rvr.get_rgbc_sensor_values()
                rvrAmbientLight = rvr.get_ambient_light_sensor_value()
                rvrBattery      = rvr.get_battery_percentage()
                rvrServoPos     = []
                for i in range(0, 16):
                    rvrServoPos.append(servo.position(i))
                formatedMessage = "{"+"{},{},{},{}".format(rvrTemps,rvrLightSensor,rvrAmbientLight,rvrBattery)+"}"
                parcell = json.loads(formatedMessage)
                server_socket.sendall(parcell)
    finally:
        connection.close()
        server_socket.close()

threadWebcam        = threading.Thread(target=webcamStream , daemon=True)
threadController    = threading.Thread(target=controllerCom, daemon=False) # kept as non daemon to enable easy remote shutdown
threadSensorLogging = threading.Thread(target=sensorDataCom, daemon=True)

if __name__ == '__main__':
    try:
        threadWebcam.start()
        threadController.start()
        threadSensorLogging.start()
        
        threadWebcam.join()
        threadController.join()
        threadSensorLogging.join()
        
    except KeyboardInterrupt:
        print('\nProgram terminated with keyboard interrupt.')
        
    finally:
        rvr.close()