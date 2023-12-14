import os
import sys
import time
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../')))
from sphero_sdk import SpheroRvrObserver
rvr = SpheroRvrObserver()


def battery_voltage_state_change_handler(battery_voltage_state):
    print('Battery voltage state: ', battery_voltage_state)


def main():
    """ This program demonstrates how to enable battery state change notifications.
    """
    rvr.wake()
    i = 0
        # Give RVR time to wake up
    time.sleep(1)
    rvr.enable_battery_voltage_state_change_notify(is_enabled=True)
    while True:
        
        print(i)
        i = i + 1
        try:
            rvr.on_battery_voltage_state_change_notify(handler=battery_voltage_state_change_handler)
            print("ping")
        except KeyboardInterrupt:
            print('\nProgram terminated with keyboard interrupt.')             
    rvr.close()       

if __name__ == '__main__':
        main()