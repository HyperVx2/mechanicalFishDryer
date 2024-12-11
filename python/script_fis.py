import paho.mqtt.client as mqtt
import json
import time
import threading
import logging
from datetime import datetime

# Configuration
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_USERNAME = "mosquitto"
MQTT_PASSWORD = "foobar123"

# MQTT Topics
TOPIC_SENSORS = "esp32/sensors"
TOPIC_HEATER = "esp32/heater"
TOPIC_FAN = "esp32/fan"
TOPIC_TIMER = "esp32/timer"
TOPIC_TIMER_START = "esp32/timer/start"
TOPIC_TIMER_ADD = "esp32/timer/add"
TOPIC_TIMER_RESET = "esp32/timer/reset"

# Temperature Ranges for Trays
TEMP_RANGES = {
    1: (55, 60),
    2: (55, 60),
    3: (58, 64),
    4: (60, 65),
    5: (60, 70),
    6: (65, 70)
}

# Initial timer (8 hours in milliseconds)
INITIAL_TIMER_MS = 8 * 60 * 60 * 1000
FAN_EXTRA_TIME_MS = 10 * 60 * 1000  # 10 minutes in milliseconds

# Logging setup (with console output)
logging.basicConfig(
    filename="drying_system.log",
    level=logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
logger = logging.getLogger()
console_handler = logging.StreamHandler()
console_handler.setLevel(logging.INFO)
console_handler.setFormatter(logging.Formatter("%(asctime)s - %(message)s", datefmt="%Y-%m-%d %H:%M:%S"))
logger.addHandler(console_handler)

# Global Variables
user_data = {"fish_name": "", "tray_count": 0}
timer_remaining_ms = INITIAL_TIMER_MS
timer_active = False
heater_status = False
fan_status = False
latest_temperature = None
mqtt_client = None


# Helper Functions
def log_event(event):
    logger.info(event)


def validate_sensor_data(data):
    required_keys = {"tem", "hum", "wei", "cur", "vol"}
    return all(key in data for key in required_keys)


def process_sensor_data(data):
    """Store sensor data for periodic processing."""
    global latest_temperature
    latest_temperature = data.get("tem")
    log_event(f"Sensor data received: {data}")


def enforce_temperature_rules():
    """Apply heater control rules based on the latest sensor data."""
    global heater_status, mqtt_client, latest_temperature

    if latest_temperature is None:
        return  # No data to process yet

    tray_count = user_data["tray_count"]
    if tray_count in TEMP_RANGES:
        min_temp, max_temp = TEMP_RANGES[tray_count]

        if latest_temperature < min_temp and not heater_status:
            heater_status = True
            mqtt_client.publish(TOPIC_HEATER, json.dumps(True))
            log_event(f"Heater turned ON. Temperature: {latest_temperature}°C")

        elif latest_temperature > max_temp and heater_status:
            heater_status = False
            mqtt_client.publish(TOPIC_HEATER, json.dumps(False))
            log_event(f"Heater turned OFF. Temperature: {latest_temperature}°C")


def on_message(client, userdata, msg):
    """Handle incoming MQTT messages."""
    if msg.topic == TOPIC_SENSORS:
        try:
            data = json.loads(msg.payload.decode())
            if validate_sensor_data(data):
                process_sensor_data(data)
            else:
                log_event("Invalid sensor data received.")
        except json.JSONDecodeError:
            log_event("Malformed JSON received on sensor topic.")


def on_connect(client, userdata, flags, rc):
    """Handle MQTT connection events."""
    if rc == 0:
        log_event("Connected to MQTT broker.")
        client.subscribe(TOPIC_SENSORS)
        log_event(f"Subscribed to topic: {TOPIC_SENSORS}")
    else:
        log_event(f"Failed to connect to MQTT broker. Return code: {rc}")


def setup_mqtt_client():
    """Setup and connect the MQTT client."""
    client = mqtt.Client()
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            break
        except Exception as e:
            log_event(f"MQTT connection failed: {e}. Retrying in 5 seconds...")
            time.sleep(5)

    return client


def user_setup():
    """Setup user input for fish name and tray count."""
    global user_data

    while True:
        fish_name = input("Enter fish name: ").strip()
        try:
            tray_count = int(input("Enter number of trays (1-6): ").strip())
            if tray_count not in range(1, 7):
                raise ValueError
        except ValueError:
            print("Invalid input. Tray count must be a number between 1 and 6.")
            continue

        print(f"Fish Name: {fish_name}, Tray Count: {tray_count}")
        confirm = input("Confirm this setup? (y/n): ").strip().lower()
        if confirm == 'y':
            user_data["fish_name"] = fish_name
            user_data["tray_count"] = tray_count
            break


def start_timer():
    """Start the drying timer."""
    global timer_active, timer_remaining_ms, mqtt_client, fan_status, heater_status

    timer_active = True
    fan_status = True
    heater_status = True
    mqtt_client.publish(TOPIC_TIMER_START, json.dumps(INITIAL_TIMER_MS))
    mqtt_client.publish(TOPIC_HEATER, json.dumps(True))
    mqtt_client.publish(TOPIC_FAN, json.dumps(True))
    log_event("Timer started. Heater and fan turned ON.")


def timer_thread():
    """Manage the countdown and extra fan runtime."""
    global timer_remaining_ms, timer_active, fan_status, heater_status, mqtt_client

    while timer_active:
        time.sleep(1)
        timer_remaining_ms -= 1000

        if timer_remaining_ms <= 0:
            timer_active = False
            heater_status = False
            mqtt_client.publish(TOPIC_HEATER, json.dumps(False))
            log_event("Timer expired. Heater turned OFF.")

            # Extra fan runtime
            fan_status = True
            mqtt_client.publish(TOPIC_FAN, json.dumps(True))
            log_event("Fan will remain ON for an additional 10 minutes.")
            time.sleep(FAN_EXTRA_TIME_MS / 1000)
            fan_status = False
            mqtt_client.publish(TOPIC_FAN, json.dumps(False))
            log_event("Fan turned OFF after extra runtime.")


def add_timer_time():
    """Add 10 minutes to the timer."""
    global timer_remaining_ms, mqtt_client
    timer_remaining_ms += 10 * 60 * 1000
    mqtt_client.publish(TOPIC_TIMER_ADD, json.dumps(10 * 60 * 1000))
    log_event("10 minutes added to the timer.")


def keyboard_listener():
    """Listen for keyboard input to adjust the timer."""
    while timer_active:
        user_input = input()
        if user_input.strip().lower() == 'a':
            add_timer_time()


def monitor_temperature():
    """Periodically check temperature and apply heater control rules."""
    global timer_active

    while timer_active:
        time.sleep(5)
        enforce_temperature_rules()
        log_event("Periodic temperature check executed.")


# Main Function
def main():
    global mqtt_client

    # Setup Phase
    user_setup()

    # MQTT Client Setup
    mqtt_client = setup_mqtt_client()
    mqtt_client.loop_start()

    # Start Timer and Process
    start_timer()
    threading.Thread(target=timer_thread, daemon=True).start()
    threading.Thread(target=keyboard_listener, daemon=True).start()
    threading.Thread(target=monitor_temperature, daemon=True).start()

    # Keep the main thread alive
    while timer_active or fan_status:
        time.sleep(1)


if __name__ == "__main__":
    main()
