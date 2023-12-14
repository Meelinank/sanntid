import os
import sys
import time
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../')))
from sphero_sdk import SpheroRvrObserver
from sphero_sdk import BatteryVoltageStatesEnum as VoltageStates
rvr = SpheroRvrObserver()


def battery_voltage_handler(battery_voltage_state):
    print('Voltage state: ', battery_voltage_state)

    state_info = '[{}, {}, {}, {}]'.format(
        '{}: {}'.format(VoltageStates.unknown.name, VoltageStates.unknown.value),
        '{}: {}'.format(VoltageStates.ok.name, VoltageStates.ok.value),
        '{}: {}'.format(VoltageStates.low.name, VoltageStates.low.value),
        '{}: {}'.format(VoltageStates.critical.name, VoltageStates.critical.value)
    )
    print('Voltage states: ', state_info)

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
            rvr.get_battery_voltage_state(handler=battery_voltage_handler)
            print("ping")
        except KeyboardInterrupt:
            print('\nProgram terminated with keyboard interrupt.')             
    rvr.close()       

if __name__ == '__main__':
        main()