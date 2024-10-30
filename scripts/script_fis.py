import requests
import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl
import time
from datetime import datetime, timedelta
import threading
import os

# URLs for sensor readings and relay control
BASE_URL = "http://192.168.4.1"
READINGS_URL = f"{BASE_URL}/readings"
HEATER_ON_URL = f"{BASE_URL}/on2"
HEATER_OFF_URL = f"{BASE_URL}/off2"
FAN_ON_URL = f"{BASE_URL}/on1"
FAN_OFF_URL = f"{BASE_URL}/off1"

# Setup fuzzy logic for humidity control
def setup_fuzzy_logic():
    humidity = ctrl.Antecedent(np.arange(0, 101, 1), 'humidity')
    drying_time = ctrl.Consequent(np.arange(0, 16, 1), 'drying_time')

    humidity['low'] = fuzz.trimf(humidity.universe, [0, 0, 32])
    humidity['medium'] = fuzz.trimf(humidity.universe, [30, 34, 40])
    humidity['high'] = fuzz.trimf(humidity.universe, [32, 100, 100])

    drying_time['short'] = fuzz.trimf(drying_time.universe, [0, 0, 5])
    drying_time['medium'] = fuzz.trimf(drying_time.universe, [5, 10, 10])
    drying_time['long'] = fuzz.trimf(drying_time.universe, [10, 15, 15])

    rules = [ctrl.Rule(humidity['low'], drying_time['short']),
             ctrl.Rule(humidity['medium'], drying_time['medium']),
             ctrl.Rule(humidity['high'], drying_time['long'])]

    return ctrl.ControlSystemSimulation(ctrl.ControlSystem(rules))


class SensorController:
    """Handles sensor readings and appliance control."""
    
    def fetch_readings(self):
        try:
            response = requests.get(READINGS_URL, timeout=5)
            response.raise_for_status()
            return response.json()
        except (requests.exceptions.RequestException, ValueError):
            return None

    def control(self, appliance, state):
        url = HEATER_ON_URL if appliance == "heater" and state == "on" else \
              HEATER_OFF_URL if appliance == "heater" and state == "off" else \
              FAN_ON_URL if appliance == "fan" and state == "on" else FAN_OFF_URL
        try:
            requests.get(url)
            print(f"{appliance.capitalize()} turned {state}")
        except requests.exceptions.RequestException:
            pass


class DryingSession:
    """Manages drying session with humidity and temperature control."""

    def __init__(self, fish_name, layer_count, drying_time_minutes=240):
        self.fish_name = fish_name
        self.sensor_controller = SensorController()
        self.simulator = setup_fuzzy_logic()
        self.drying_time_minutes = drying_time_minutes
        self.layer_count = layer_count
        self.temperature_range = self.set_temperature_range(layer_count)
        self.log_entries = []
        self.low_humidity_repeats = 0
        self.extended_once = False
        self.session_active = False

    def set_temperature_range(self, layers):
        """Set temperature range based on the number of layers used."""
        if layers in [1, 2]:
            return (55, 60)
        elif layers == 3:
            return (58, 64)
        elif layers == 4:
            return (60, 65)
        elif layers == 5:
            return (60, 70)
        elif layers == 6:
            return (65, 70)
        else:
            raise ValueError("Invalid layer count. Please choose between 1 and 6.")

    def log(self, message):
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        self.log_entries.append(f"[{timestamp}] {message}")

    def save_log(self):
        """Saves log file, appending suffix if needed to avoid name clashes."""
        date_str = datetime.now().strftime('%Y%m%d')
        base_filename = f"drying_log_{date_str}_{self.fish_name}.txt"
        filename = base_filename
        counter = 1
        while os.path.exists(filename):
            filename = f"{base_filename.rsplit('.', 1)[0]}_{counter}.txt"
            counter += 1
        with open(filename, 'w') as log_file:
            log_file.write('\n'.join(self.log_entries))
        print(f"Log saved to {filename}")

    def start(self):
        """Start the drying session and activate the appliances."""
        self.session_active = True
        self.sensor_controller.control("heater", "on")
        self.sensor_controller.control("fan", "on")
        self.log(f"Drying session started for {self.fish_name} using {self.layer_count} layers with {self.temperature_range}°C temperature range")
        print(f"Drying session started. Keeping the chamber between {self.temperature_range[0]}°C and {self.temperature_range[1]}°C.")

    def end(self):
        """End the session, log the session end time, run the fan for 5 more minutes and save the log."""
        end_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        self.session_active = False
        self.sensor_controller.control("heater", "off")
        self.sensor_controller.control("fan", "on")
        print("Fan will remain on for 1 minutes before stopping.")
        self.log(f"Drying session ended at {end_time}. Fan turned on for 1 more minutes.")
        time.sleep(1 * 60)  # Fan stays on for 1 minutes
        self.sensor_controller.control("fan", "off")
        self.log(f"Fan turned off after 1 minutes.")
        self.save_log()

    def check_temperature(self):
        """Monitor temperature and control heater based on the set range."""
        while self.session_active:
            readings = self.sensor_controller.fetch_readings()
            if readings and "temperature" in readings:
                temperature = float(readings["temperature"])
                if temperature >= self.temperature_range[1]:
                    self.sensor_controller.control("heater", "off")
                    self.log(f"Temperature {temperature}°C, heater turned off")
                elif temperature <= self.temperature_range[0]:
                    self.sensor_controller.control("heater", "on")
                    self.log(f"Temperature {temperature}°C, heater turned on")
            time.sleep(60)

    def run_fuzzy_drying(self):
        """Perform fuzzy logic-based drying session with humidity control."""
        while self.session_active:
            time.sleep(5)
            readings = self.sensor_controller.fetch_readings()
            if readings and "humidity" in readings:
                humidity = float(readings["humidity"])
                self.simulator.input['humidity'] = humidity
                self.simulator.compute()
                additional_time = self.simulator.output['drying_time']
                self.log(f"Humidity {humidity}%, additional drying time {additional_time} minutes")

                if humidity <= 35:
                    self.low_humidity_repeats += 1
                else:
                    self.low_humidity_repeats = 0

                if self.low_humidity_repeats >= 3:
                    self.end()

                time.sleep(additional_time * 60)

    def extend_session(self):
        """Ask user for session extension after initial drying."""
        if self.extended_once:
            return 240  # Automatically extend for 4 hours

        user_input = input("Extend drying? (y/n, default: y): ").strip().lower() or "y"
        if user_input == "y":
            additional_hours = int(input("How many hours (1-4): ").strip() or "4")
            return min(4, max(1, additional_hours)) * 60
        return 0

    def show_reminders(self, end_time):
        """Display reminders 30, 15, 10, 5, 3, 2, 1 minutes before session ends."""
        reminders = [30, 15, 10, 5, 3, 2, 1]
        now = datetime.now()

        for minutes_before in reminders:
            reminder_time = end_time - timedelta(minutes=minutes_before)
            if now < reminder_time:
                time_until_reminder = (reminder_time - now).total_seconds()
                time.sleep(time_until_reminder)
                if not self.extended_once:
                    print(f"Reminder: {minutes_before} minutes remaining until the session ends.")
                    self.log(f"Reminder: {minutes_before} minutes remaining until the session ends.")
                else:
                    break  # Stop showing reminders if the session was extended

    def run(self):
        """Run the entire drying session, including temperature and humidity control."""
        self.start()
        temperature_thread = threading.Thread(target=self.check_temperature)
        temperature_thread.start()

        start_time = datetime.now()
        end_time = start_time + timedelta(minutes=self.drying_time_minutes)

        reminder_thread = threading.Thread(target=self.show_reminders, args=(end_time,))
        reminder_thread.start()

        # Run initial drying for 5 hours, print logs every 5 minutes
        while (datetime.now() - start_time).total_seconds() < self.drying_time_minutes * 60:
            time.sleep(5 * 60)  # Print logs every 5 minutes
            readings = self.sensor_controller.fetch_readings()
            if readings:
                self.log(f"Sensor readings: {readings}")
                print(f"Sensor readings: {readings}")

        additional_minutes = self.extend_session()
        if additional_minutes > 0:
            self.drying_time_minutes += additional_minutes
            print(f"Extended drying for {additional_minutes // 60} hours")

            end_time = datetime.now() + timedelta(minutes=additional_minutes)

            while (datetime.now() - start_time).total_seconds() < self.drying_time_minutes * 60:
                time.sleep(5 * 60)  # Continue logs every 5 minutes
                readings = self.sensor_controller.fetch_readings()
                if readings:
                    self.log(f"Sensor readings: {readings}")
                    print(f"Sensor readings: {readings}")
            self.end()
        else:
            self.end()


def main():
    """Main entry point to start the drying session."""
    fish_name = input("Enter fish name: ").strip()

    # Ask user how many layers are being used
    while True:
        try:
            layer_count = int(input("Enter the number of layers used in the chamber (1-6): ").strip())
            if 1 <= layer_count <= 6:
                break
            else:
                print("Please enter a valid number between 1 and 6.")
        except ValueError:
            print("Invalid input. Please enter a number between 1 and 6.")

    start_drying = input(f"Start drying {fish_name}? (y/n): ").strip().lower()

    if start_drying == "y":
        session = DryingSession(fish_name, layer_count)
        session.run()
    else:
        print("Drying session not started.")


if __name__ == "__main__":
    main()
