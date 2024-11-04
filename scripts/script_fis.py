import requests
import sqlite3
import logging
import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl
import time
from datetime import datetime, timedelta
import threading
import os

# Constants
BASE_URL = "http://192.168.4.1"
READINGS_URL = f"{BASE_URL}/readings"
HEATER_ON_URL = f"{BASE_URL}/on2"
HEATER_OFF_URL = f"{BASE_URL}/off2"
FAN_ON_URL = f"{BASE_URL}/on1"
FAN_OFF_URL = f"{BASE_URL}/off1"
SPECIAL_TRIGGER_URL = f"{BASE_URL}/on0"  # New URL for 't' trigger
DB_FILE = "drying_session.db"
TIMEOUT = 5

# Logging setup
logging.basicConfig(filename='drying_session.log', level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Database setup
def setup_database():
    """Initialize the SQLite database and create tables if they don't exist."""
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS SensorReadings (
            id INTEGER PRIMARY KEY,
            timestamp TEXT,
            temperature REAL,
            humidity REAL,
            weight REAL,
            drying_time_left REAL,
            millis INTEGER  -- New column to store Arduino millis()
        )
    ''')
    conn.commit()
    conn.close()

setup_database()

# Fuzzy Logic Setup
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
    """Handles sensor readings and appliance control with timeout and retry options."""

    def __init__(self, timeout=TIMEOUT):
        self.timeout = timeout

    def fetch_readings(self):
        try:
            response = requests.get(READINGS_URL, timeout=self.timeout)
            response.raise_for_status()
            return response.json()
        except (requests.exceptions.RequestException, ValueError) as e:
            logging.error(f"Failed to fetch sensor readings: {e}")
            return None

    def control(self, appliance, state):
        """Controls appliances with improved error handling and logging."""
        url = HEATER_ON_URL if appliance == "heater" and state == "on" else \
              HEATER_OFF_URL if appliance == "heater" and state == "off" else \
              FAN_ON_URL if appliance == "fan" and state == "on" else FAN_OFF_URL
        try:
            response = requests.get(url, timeout=self.timeout)
            response.raise_for_status()
            logging.info(f"{appliance.capitalize()} turned {state}")
        except requests.exceptions.RequestException as e:
            logging.error(f"Failed to control {appliance}: {e}")

    def trigger_special_function(self):
        """Special trigger for user input 't'."""
        try:
            response = requests.get(SPECIAL_TRIGGER_URL, timeout=self.timeout)
            response.raise_for_status()
            logging.info("Special function triggered via 't' input.")
        except requests.exceptions.RequestException as e:
            logging.error(f"Failed to trigger special function: {e}")


class DryingSession:
    """Manages drying session with temperature, humidity control, and database logging."""

    def __init__(self, fish_name, layer_count, drying_time_minutes=240):
        self.fish_name = fish_name
        self.sensor_controller = SensorController()
        self.simulator = setup_fuzzy_logic()
        self.drying_time_minutes = drying_time_minutes
        self.layer_count = layer_count
        self.temperature_range = self.set_temperature_range(layer_count)
        self.low_humidity_repeats = 0
        self.session_active = False
        self.lock = threading.Lock()

    def set_temperature_range(self, layers):
        """Set temperature range based on the number of layers used."""
        ranges = {1: (55, 60), 2: (55, 60), 3: (58, 64), 4: (60, 65), 5: (60, 70), 6: (65, 70)}
        return ranges.get(layers, (55, 60))

    def log(self, message):
        logging.info(message)

    def insert_sensor_reading(self, temperature, humidity, weight, drying_time_left, millis):
        """Log sensor readings into the SQLite database."""
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO SensorReadings (timestamp, temperature, humidity, weight, drying_time_left, millis)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (timestamp, temperature, humidity, weight, drying_time_left, millis))
        conn.commit()
        conn.close()

    def start(self):
        """Start the drying session and activate appliances."""
        self.session_active = True
        self.sensor_controller.control("heater", "on")
        self.sensor_controller.control("fan", "on")
        self.log(f"Drying session started for {self.fish_name} with temperature range {self.temperature_range}°C.")

    def end(self):
        """Safely end the session, deactivate appliances, and log the event."""
        with self.lock:
            self.session_active = False
        self.sensor_controller.control("heater", "off")
        self.sensor_controller.control("fan", "off")
        self.log("Drying session ended.")

    def check_temperature(self):
        """Monitor and control the temperature within the set range."""
        while self.session_active:
            readings = self.sensor_controller.fetch_readings()
            if readings and "temperature" in readings:
                temperature = float(readings["temperature"])
                if temperature >= self.temperature_range[1]:
                    self.sensor_controller.control("heater", "off")
                    self.log(f"Temperature {temperature}°C - Heater turned off")
                elif temperature <= self.temperature_range[0]:
                    self.sensor_controller.control("heater", "on")
                    self.log(f"Temperature {temperature}°C - Heater turned on")
            time.sleep(60)

    def run_fuzzy_drying(self):
        """Perform fuzzy logic-based drying session with humidity control."""
        while self.session_active:
            time.sleep(5)
            readings = self.sensor_controller.fetch_readings()
            if readings and "humidity" in readings:
                humidity = float(readings["humidity"])
                weight = float(readings.get("weight", 0))  # Assume weight in readings
                millis = int(readings.get("millis", 0))  # New millis value from Arduino
                self.simulator.input['humidity'] = humidity
                self.simulator.compute()
                additional_time = self.simulator.output['drying_time']
                self.log(f"Humidity {humidity}%, additional drying time {additional_time} minutes")

                # Insert the reading including millis to the database
                self.insert_sensor_reading(readings.get("temperature"), humidity, weight, additional_time, millis)

                if humidity <= 35:
                    self.low_humidity_repeats += 1
                else:
                    self.low_humidity_repeats = 0

                if self.low_humidity_repeats >= 3:
                    self.end()

                time.sleep(additional_time * 60)

    def run(self):
        """Run the entire drying session, including temperature and humidity control."""
        try:
            self.start()
            temperature_thread = threading.Thread(target=self.check_temperature)
            temperature_thread.start()
            fuzzy_drying_thread = threading.Thread(target=self.run_fuzzy_drying)
            fuzzy_drying_thread.start()

            temperature_thread.join()
            fuzzy_drying_thread.join()
        finally:
            self.end()


def main():
    """Main entry point to start the drying session."""
    fish_name = input("Enter fish name: ").strip()

    while True:
        try:
            layer_count = int(input("Enter the number of layers used in the chamber (1-6): ").strip())
            if 1 <= layer_count <= 6:
                break
            else:
                print("Please enter a valid number between 1 and 6.")
        except ValueError:
            print("Invalid input. Please enter a number between 1 and 6.")

    start_drying = input(f"Start drying {fish_name}? (y/n/t): ").strip().lower()

    if start_drying == "y":
        session = DryingSession(fish_name, layer_count)
        session.run()
    elif start_drying == "t":
        # Trigger the special function
        sensor_controller = SensorController()
        sensor_controller.trigger_special_function()
        print("Special function triggered.")
    else:
        print("Drying session not started.")


if __name__ == "__main__":
    main()
