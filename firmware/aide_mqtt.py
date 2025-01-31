# Create a socket pool
pool = socketpool.SocketPool(wifi.radio)
mqtt_message_queue = []

# Set up MQTT client
mqtt_client = MQTT(
    broker=os.getenv("MQTT_BROKER"),
    port=os.getenv("MQTT_PORT"),
    username=os.getenv("MQTT_USERNAME"),
    password=os.getenv("MQTT_PASSWORD"),
    socket_pool=pool,
    ssl_context=ssl.create_default_context(),
)

# Define MQTT callbacks (optional but recommended)
def connected(client, userdata, flags, rc):
    print("Connected to MQTT Broker!")
    print(f"Connected to MQTT broker! Listening for topic changes on {default_topic}")
    # Subscribe to all changes on the default_topic feed.
    client.subscribe(default_topic)

def disconnected(client, userdata, rc):
    print("Disconnected from MQTT Broker!")

def subscribe(mqtt_client, userdata, topic, granted_qos):
    # This method is called when the mqtt_client subscribes to a new feed.
    print(f"Subscribed to {topic} with QOS level {granted_qos}")


def unsubscribe(mqtt_client, userdata, topic, pid):
    # This method is called when the mqtt_client unsubscribes from a feed.
    print(f"Unsubscribed from {topic} with PID {pid}")


def publish(mqtt_client, userdata, topic, pid):
    # This method is called when the mqtt_client publishes data to a feed.
    print(f"Published to {topic} with PID {pid}")


def message(client, topic, message):
    print(f"New message on topic {topic}: {message}")
    mqtt_message_queue.append((topic, message))

# Assign callbacks
mqtt_client.on_connect = connected
mqtt_client.on_disconnect = disconnected
mqtt_client.on_subscribe = subscribe
mqtt_client.on_unsubscribe = unsubscribe
mqtt_client.on_publish = publish
mqtt_client.on_message = message
try:
    mqtt_client.will_set('device/status/','Goodbye!')
    # Connect to MQTT broker
    mqtt_client.connect()
except Exception as e:
    print("Error: ", e)
    #continue
            # Handle any exceptions, such as network disconnection or sensor errors 



PUBLISH_DELAY = 60
MQTT_TOPIC = "state/panels-active"
USE_DEEP_SLEEP = True

def send_mqtt(output = {  "state": 0,
        "activity": 0,}):
    print("Publishing to %s" % MQTT_TOPIC)
    mqtt_client.publish(MQTT_TOPIC, json.dumps(output))

def fetch_event_data():
    print("Running fetch_event_data")
    for item in mqtt_message_queue:      
            print(f' Working on {item}')
            print(f' Finished {item}')
            q.task_done()
    return 0
