import requests
import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl
import time
from datetime import datetime, timedelta
import threading
import os  # For checking and appending suffix to log file names

# URLs for sensor readings and relay control
BASE_URL = "http://192.168.4.1"
READINGS_URL = f"{BASE_URL}/readings"
HEATER_ON_URL = f"{BASE_URL}/on2"
HEATER_OFF_URL = f"{BASE_URL}/off2"
FAN_ON_URL = f"{BASE_URL}/on1"
FAN_OFF_URL = f"{BASE_URL}/off1"

# Fuzzy Logic System Setup for Humidity and Drying Time
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


class SensorController:
    """Handles sensor readings and appliance control."""
    
    def fetch_readings(self):
        """Fetch sensor readings from the device."""
        try:
            response = requests.get(READINGS_URL, timeout=5)
            response.raise_for_status()
            return response.json()
        except (requests.exceptions.RequestException, ValueError) as e:
            print(f"Error fetching readings: {e}")
            return None

    def control_appliance(self, url, action):
        """Send a control signal to an appliance (fan or heater)."""
        try:
            requests.get(url)
            print(f"{action.capitalize()} command sent to {url}")
        except requests.exceptions.RequestException as e:
            print(f"Error controlling appliance: {e}")

    def control_heater(self, state):
        """Turn the heater on or off."""
        url = HEATER_ON_URL if state == "on" else HEATER_OFF_URL
        self.control_appliance(url, f"Heater turned {state}")

    def control_fan(self, state):
        """Turn the fan on or off."""
        url = FAN_ON_URL if state == "on" else FAN_OFF_URL
        self.control_appliance(url, f"Fan turned {state}")


class DryingSession:
    """Handles the drying session, including humidity control and logging."""
    
    def __init__(self, fish_name, sensor_controller, initial_drying_time=240, temperature_threshold=(58, 65)):
        self.fish_name = fish_name
        self.sensor_controller = sensor_controller
        self.session_active = False
        self.low_humidity_repeats = 0
        self.fixed_drying_time_minutes = initial_drying_time
        self.log_entries = []
        self.temperature_threshold = temperature_threshold
        self.extended_session_once = False  # Only allow one session extension

    def log(self, message):
        """Log a message with a timestamp."""
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        self.log_entries.append(f"[{timestamp}] {message}")

    def generate_log_filename(self):
        """Generate log filename with date, appending suffix if necessary."""
        date_str = datetime.now().strftime('%Y%m%d')
        base_filename = f"drying_log_{date_str}_{self.fish_name}.txt"
        filename = base_filename
        counter = 1
        while os.path.exists(filename):
            filename = f"{base_filename.rsplit('.', 1)[0]}_{counter}.txt"
            counter += 1
        return filename

    def save_log(self):
        """Save the log to a file."""
        filename = self.generate_log_filename()
        with open(filename, 'w') as log_file:
            for entry in self.log_entries:
                log_file.write(entry + '\n')
        print(f"Log saved to {filename}")

    def start_drying_session(self):
        """Start the drying session with initial fan and heater controls."""
        self.session_active = True
        self.sensor_controller.control_heater("on")
        self.sensor_controller.control_fan("on")
        print(f"Drying session started for {self.fish_name} with 4 hours initial drying time")
        self.log(f"Drying session started for {self.fish_name}")

    def end_drying_session(self):
        """End the drying session, turning off the heater and running the fan for 1 minute."""
        self.session_active = False
        self.sensor_controller.control_heater("off")
        print(f"Drying session for {self.fish_name} is ending. Fan will remain on for 1 minute.")
        self.log(f"Drying session ending. Fan on for 1 minute.")
        
        # Run the fan for an additional minute before stopping
        self.sensor_controller.control_fan("on")
        time.sleep(60)  # Fan stays on for 1 minute
        self.sensor_controller.control_fan("off")
        
        print("Drying session fully ended.")
        self.log(f"Drying session fully ended for {self.fish_name}")
        self.save_log()

    def ask_user_for_extension(self):
        """Ask the user if they want to extend the session after the first 4 hours."""
        if self.extended_session_once:
            return 240  # Automatically continue for 4 hours if already extended

        print("4 hours have passed. Do you want to extend the drying time?")
        user_input = None
        user_input_received = threading.Event()

        def get_input():
            nonlocal user_input
            user_input = input("Enter 'y' to continue or 'n' to stop (default: continue for 4 hours): ").strip().lower()
            user_input_received.set()

        input_thread = threading.Thread(target=get_input)
        input_thread.start()
        user_input_received.wait(timeout=5 * 60)  # Wait 5 minutes for user response

        if user_input == "y":
            while True:
                try:
                    additional_hours = int(input("Enter how many hours to extend (1-4): ").strip())
                    if 1 <= additional_hours <= 4:
                        return additional_hours * 60
                    else:
                        print("Invalid input. Please enter a number between 1 and 4.")
                except ValueError:
                    print("Invalid input. Please enter a valid number.")
        elif user_input == "n":
            return 0
        else:
            print("No response received. Continuing for 4 more hours.")
            return 240

    def check_temperature_and_control_heater(self):
        """Monitor the temperature and control the heater based on set thresholds."""
        try:
            while self.session_active:
                readings = self.sensor_controller.fetch_readings()
                if readings and "temperature" in readings:
                    temperature = float(readings["temperature"])
                    if temperature >= self.temperature_threshold[1]:
                        self.sensor_controller.control_heater("off")
                        self.log(f"Temperature is {temperature}°C, heater turned OFF.")
                    elif temperature <= self.temperature_threshold[0]:
                        self.sensor_controller.control_heater("on")
                        self.log(f"Temperature is {temperature}°C, heater turned ON.")
                    else:
                        self.log(f"Temperature is {temperature}°C, heater state unchanged.")
                time.sleep(60)  # Check every minute
        except KeyboardInterrupt:
            self.log("Temperature monitoring interrupted by user.")
            self.end_drying_session()

    def run(self):
        """Run the drying session, checking temperature and humidity."""
        self.start_drying_session()

        temperature_thread = threading.Thread(target=self.check_temperature_and_control_heater)
        temperature_thread.start()

        start_time = datetime.now()
        try:
            # Initial fixed drying time (4 hours) with sensor readings every 5 minutes
            while (datetime.now() - start_time).total_seconds() < self.fixed_drying_time_minutes * 60:
                time.sleep(5 * 60)  # Fetch readings every 5 minutes
                readings = self.sensor_controller.fetch_readings()
                if readings:
                    self.log(f"Sensor Readings: {readings}")
                    print(f"Sensor Readings: {readings}")

            # After 4 hours, ask user for session extension
            additional_minutes = self.ask_user_for_extension()
            if additional_minutes > 0:
                print(f"Extending drying session by {additional_minutes // 60} hours.")
                start_time = datetime.now()  # Reset start time for new session
                self.log(f"Drying session extended by {additional_minutes // 60} hours.")
            else:
                self.end_drying_session()
                return

            # Continue with humidity checks during extended session
            while self.session_active:
                time.sleep(5)  # Check humidity every 5 seconds
                readings = self.sensor_controller.fetch_readings()
                if readings:
                    self.humidity_value = float(readings.get("humidity", 0))
                    drying_sim.input['humidity'] = self.humidity_value
                    drying_sim.compute()
                    additional_time = drying_sim.output['drying_time']
                    self.log(f"Humidity: {self.humidity_value}, Additional drying time: {additional_time} mins")
                    
                    if self.humidity_value <= 35:
                        self.low_humidity_repeats += 1
                    else:
                        self.low_humidity_repeats = 0

                    time.sleep(additional_time * 60)

                    if self.low_humidity_repeats >= 3:
                        self.end_drying_session()

        except KeyboardInterrupt:
            self.log("Drying process interrupted by user.")
            self.end_drying_session()


def main():
    """Main entry point for starting a drying session."""
    fish_name = input("Hello! Please name the fish you are drying: ")
    start_drying = input(f"Okay, we're drying {fish_name}! Start now? (y/n): ").strip().lower()

    while start_drying not in ["y", "n"]:
        start_drying = input("Invalid input. Please enter 'y' or 'n': ").strip().lower()

    if start_drying == "y":
        sensor_controller = SensorController()
        session = DryingSession(fish_name, sensor_controller)
        session.run()
    else:
        print("Exiting program.")


if __name__ == "__main__":
    main()
