import requests
import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl
import time
from datetime import datetime, timedelta

# URLs for sensor readings and relay control
BASE_URL = "http://192.168.4.1"
READINGS_URL = f"{BASE_URL}/readings"
HEATER_ON_URL = f"{BASE_URL}/on0"
HEATER_OFF_URL = f"{BASE_URL}/off0"
FAN_ON_URL = f"{BASE_URL}/on1"
FAN_OFF_URL = f"{BASE_URL}/off1"

# Fuzzy Logic System Setup
humidity = ctrl.Antecedent(np.arange(0, 101, 1), 'humidity')
drying_time = ctrl.Consequent(np.arange(0, 16, 1), 'drying_time')

humidity['low'] = fuzz.trimf(humidity.universe, [0, 0, 32])
humidity['medium'] = fuzz.trimf(humidity.universe, [30, 34, 40])
humidity['high'] = fuzz.trimf(humidity.universe, [32, 100, 100])

drying_time['short'] = fuzz.trimf(drying_time.universe, [0, 0, 5])
drying_time['medium'] = fuzz.trimf(drying_time.universe, [5, 10, 10])
drying_time['long'] = fuzz.trimf(drying_time.universe, [10, 15, 15])

rule1 = ctrl.Rule(humidity['low'], drying_time['short'])
rule2 = ctrl.Rule(humidity['medium'], drying_time['medium'])
rule3 = ctrl.Rule(humidity['high'], drying_time['long'])

drying_ctrl = ctrl.ControlSystem([rule1, rule2, rule3])
drying_sim = ctrl.ControlSystemSimulation(drying_ctrl)

class DryingSession:
    def __init__(self, fish_name):
        self.fish_name = fish_name
        self.session_active = False
        self.drying_counter = 0
        self.low_humidity_repeats = 0
        self.humidity_value = 0
        self.fixed_drying_time_minutes = 150  # 2 hours and 30 minutes
        self.log_entries = []

    def fetch_readings(self):
        try:
            response = requests.get(READINGS_URL, timeout=5)
            response.raise_for_status()
            data = response.json()
            self.humidity_value = float(data["humidity"])
            return data
        except (requests.exceptions.RequestException, ValueError) as e:
            print(f"Error fetching readings: {e}")
            return None

    def control_heater(self, state):
        url = HEATER_ON_URL if state == "on" else HEATER_OFF_URL
        try:
            requests.get(url)
            print(f"Heater turned {state}")
            self.log(f"Heater turned {state}")
        except requests.exceptions.RequestException as e:
            print(f"Error controlling heater: {e}")

    def control_fan(self, state):
        url = FAN_ON_URL if state == "on" else FAN_OFF_URL
        try:
            requests.get(url)
            print(f"Fan turned {state}")
            self.log(f"Fan turned {state}")
        except requests.exceptions.RequestException as e:
            print(f"Error controlling fan: {e}")

    def log(self, message):
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        log_entry = f"[{timestamp}] {message}"
        self.log_entries.append(log_entry)

    def save_log(self):
        filename = f"drying_log_{self.fish_name}.txt"
        with open(filename, 'w') as log_file:
            for entry in self.log_entries:
                log_file.write(entry + '\n')
        print(f"Log saved to {filename}")

    def start_drying_session(self):
        self.session_active = True
        self.drying_counter = 0
        self.low_humidity_repeats = 0
        self.control_heater("on")
        self.control_fan("on")
        print(f"Drying session started for {self.fish_name} with a fixed initial drying time of 2 hours and 30 minutes")
        self.log(f"Drying session started for {self.fish_name}")

    def end_drying_session(self):
        self.session_active = False
        self.control_heater("off")
        self.control_fan("off")
        print(f"Drying session ended for {self.fish_name}")
        self.log(f"Drying session ended for {self.fish_name}")
        self.save_log()

    def run(self):
        self.start_drying_session()
        fixed_end_time = datetime.now() + timedelta(minutes=self.fixed_drying_time_minutes)
        print(f"Initial fixed drying period until: {fixed_end_time.strftime('%Y-%m-%d %H:%M:%S')}")
        self.log(f"Fixed drying period until: {fixed_end_time.strftime('%Y-%m-%d %H:%M:%S')}")
        
        # Run fixed drying session for 2 hours 30 minutes (15-minute intervals for printing readings)
        start_time = datetime.now()
        try:
            while (datetime.now() - start_time).total_seconds() < self.fixed_drying_time_minutes * 60:
                time.sleep(15 * 60)  # Every 15 minutes
                readings = self.fetch_readings()
                if readings:
                    print(f"Readings: {readings}")
                    self.log(f"Readings: {readings}")
            
            # Now begin the fuzzy logic-based drying session
            while self.session_active:
                time.sleep(5)  # Fetch data every 5 seconds
                readings = self.fetch_readings()

                if self.session_active and readings:
                    if self.humidity_value > 0:
                        drying_sim.input['humidity'] = self.humidity_value
                        drying_sim.compute()
                        additional_time = drying_sim.output['drying_time']
                        current_time = datetime.now()
                        end_time = current_time + timedelta(minutes=additional_time)

                        print(f"Humidity: {self.humidity_value}, Additional drying time: {additional_time} mins")
                        print(f"Expected drying finish time: {end_time.strftime('%Y-%m-%d %H:%M:%S')}")
                        self.log(f"Humidity: {self.humidity_value}, Additional drying time: {additional_time} mins")

                        if self.humidity_value <= 35:
                            self.low_humidity_repeats += 1
                        else:
                            self.low_humidity_repeats = 0

                        self.drying_counter += additional_time
                        time.sleep(additional_time * 60)  # Simulate the drying time in seconds

                        if self.low_humidity_repeats >= 3:
                            self.end_drying_session()
                    else:
                        print("Invalid humidity data, waiting for next readings...")
                else:
                    print("Waiting for humidity data...")

        except KeyboardInterrupt:
            print("Drying process interrupted by user.")
            self.log("Drying process interrupted by user.")
            self.end_drying_session()

# Main program
def main():
    # Ask user for the name of the fish
    fish_name = input("Hello! Please name the fish you are drying: ")
    
    # Ask if they want to start the drying process
    start_drying = input(f"Okay we're drying {fish_name}! Start now? (y/n): ").strip().lower()
    while start_drying not in ["y", "n"]:
        start_drying = input("Invalid input. Please enter 'y' or 'n': ").strip().lower()

    if start_drying == "y":
        session = DryingSession(fish_name)
        session.run()
    else:
        print("Exiting program.")

if __name__ == "__main__":
    main()
