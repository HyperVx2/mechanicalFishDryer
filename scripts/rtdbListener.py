import firebase_admin
from firebase_admin import credentials, db
import csv
import os
from datetime import datetime

# Path to your service account key file
service_account_path = 'mechfishdryer-firebase-adminsdk-i6jag-e0247b5da1.json'

# Initialize the Firebase app with service account credentials
cred = credentials.Certificate(service_account_path)
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://mechfishdryer-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# Define the path in the database to monitor
db_ref = db.reference('/UsersData/Vaaq5v0fo9a6cNEWfUc8mFRmPml2')

# Function to write data to CSV
def write_to_csv(data):
    file_exists = os.path.isfile('data.csv')
    with open('data.csv', 'a', newline='') as csvfile:
        fieldnames = ['timestamp', 'temperature', 'humidity', 'weight']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        if not file_exists:
            writer.writeheader()  # Write header if file doesn't exist
        writer.writerow(data)

# Listener function to handle data changes
def listener(event):
    # Extract the data from the event
    data = event.data
    if isinstance(data, dict):
        # Prepare data for CSV
        csv_data = {
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'temperature': data.get('temperature'),
            'humidity': data.get('humidity'),
            'weight': data.get('weight')
        }
    elif isinstance(data, (int, float)):
        # Assuming the data is a single measurement value
        csv_data = {
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'temperature': data if 'temperature' in event.path else None,
            'humidity': data if 'humidity' in event.path else None,
            'weight': data if 'weight' in event.path else None
        }
    else:
        print(f"Unexpected data type: {type(data)}")
        return

    # Write data to CSV
    write_to_csv(csv_data)

# Attach listener to the database reference
db_ref.listen(listener)
