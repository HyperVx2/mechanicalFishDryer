import paho.mqtt.client as mqtt
import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl
import time
from datetime import datetime, timedelta

# MQTT Configuration
MQTT_BROKER = "chadapi.local"
MQTT_PORT = 1883
MQTT_USER = "chada"
MQTT_PASSWORD = "foobar123"
TOPICS = ["machine/temperature", "machine/humidity", "machine/weight", "machine/heater", "machine/fan", "machine/heater_ctrl", "machine/fan_ctrl"]

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

# MQTT Callbacks
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    for topic in TOPICS:
        client.subscribe(topic)

def on_message(client, userdata, msg):
    global humidity_value
    if msg.topic == "machine/humidity":
        humidity_value = float(msg.payload.decode())

# Initialize MQTT Client
client = mqtt.Client()
client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Drying Process State
session_active = False
drying_counter = 0
low_humidity_repeats = 0
humidity_value = 0

def start_drying_session():
    global session_active, drying_counter, low_humidity_repeats
    session_active = True
    drying_counter = 0
    low_humidity_repeats = 0
    client.publish("machine/heater_ctrl", "on")
    client.publish("machine/fan_ctrl", "on")
    print("Drying session started with fixed initial drying time of 2 hours and 30 minutes")

def end_drying_session():
    global session_active
    session_active = False
    client.publish("machine/heater_ctrl", "off")
    client.publish("machine/fan_ctrl", "off")
    print("Drying session ended")

# Main Loop
try:
    client.loop_start()
    start_drying_session()

    # Fixed initial drying time of 2 hours and 30 minutes
    fixed_drying_time_minutes = 150
    fixed_end_time = datetime.now() + timedelta(minutes=fixed_drying_time_minutes)
    print(f"Initial fixed drying period until: {fixed_end_time.strftime('%Y-%m-%d %H:%M:%S')}")
    time.sleep(fixed_drying_time_minutes * 60)

    while True:
        time.sleep(5)  # Wait to get the humidity reading

        if session_active and humidity_value > 0:  # Check if we have a valid humidity reading
            drying_sim.input['humidity'] = humidity_value
            drying_sim.compute()
            additional_time = drying_sim.output['drying_time']
            current_time = datetime.now()
            end_time = current_time + timedelta(minutes=additional_time)
            print(f"Humidity: {humidity_value}, Additional drying time: {additional_time} mins")
            print(f"Command issued at: {current_time.strftime('%Y-%m-%d %H:%M:%S')}")
            print(f"Expected drying finish time: {end_time.strftime('%Y-%m-%d %H:%M:%S')}")

            if humidity_value <= 35:
                low_humidity_repeats += 1
            else:
                low_humidity_repeats = 0

            drying_counter += additional_time
            time.sleep(additional_time * 60)  # Simulate the drying time in seconds

            if low_humidity_repeats >= 3:
                end_drying_session()
        else:
            print("Waiting for humidity data...")

except KeyboardInterrupt:
    print("Exiting...")
    end_drying_session()
    client.loop_stop()

client.loop_forever()