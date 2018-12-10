import time

import paho.mqtt.client as mqtt

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from watchdog.events import FileCreatedEvent
from watchdog.events import FileDeletedEvent
from watchdog.events import FileModifiedEvent

#
# File Commanded States:
# 0 --> CLOSED
# 1 --> OPEN
# 2 --> AUTO (time of day)
#

class FileChangedHandler(FileSystemEventHandler):

    def __init__(self):
        self.filename ="door.state"
        self.state = 0

    def on_deleted(self, event):
        if isinstance(event, FileDeletedEvent):
            print("File deleted", event.src_path)
            if event.src_path == "./" + self.filename:
                self.state = 2

    def on_created(self, event):
        if isinstance(event, FileCreatedEvent):
            print("File created", event.src_path)
            if event.src_path == "./" + self.filename:
                self.state = 0

    def on_modified(self, event):
        if isinstance(event, FileModifiedEvent):
            print("File modified:", event.src_path)
            if event.src_path == "./" + self.filename:
                self.read_state()

    def read_state(self):
        try:
            with open(self.filename) as file:
                line = file.readline()
                try:
                    self.state = int(line)
                except Exception as error:
                    print("Line is not valid!")
                    print(error)
                    self.state = 0
                file.close
        except Exception as error:
            print("File could not be read!")
            print(error)
            self.state = 2

class DoorMaster(object):
    def __init__(self, mqtt_client, server):
        self.client = mqtt_client
        self.topic = "/doormaster/command"
        print("connect to ", server)
        self.client.connect(server)

    def push_state(self, state):
        print("pushing", state)
        self.client.publish(self.topic, state)

    def on_connect(self, client, userdata, flags, rc):
        print("Connected "+str(client)+" with result code "+str(rc))
        self.client.subscribe("/deckdoor/#")

    def on_message(self, client, userdata, flags, msg):
        userdata[msg.topic](msg)

def deckdoor_state(msg):
    print("deckdoor state: "+msg.payload)


topiclist = {
    '/deckdoor/state' : deckdoor_state
}

mqtt_client = mqtt.Client("DoorMaster", True, topiclist)
door_master = DoorMaster(mqtt_client, "localhost")
mqtt_client.on_connect = door_master.on_connect
mqtt_client.on_message = door_master.on_message

event_handler = FileChangedHandler()

observer = Observer()
observer.schedule(event_handler, ".")
observer.start()
mqtt_client.loop_start()

time.sleep(3)
event_handler.read_state()

testing = True
flip = True
elapsed = time.time()
try:
    while True:
        if event_handler.state == 2: # this is automatic mode
            if not testing: # set open or close depending the time of day
                now = time.localtime()
                if 9 <= now.tm_hour and now.tm_hour <= 17:
                    door_master.push_state(1)
                else:
                    door_master.push_state(0)
            else: # flip-flop open-closed for testing
                if flip:
                    door_master.push_state(1)
                    flip = False
                else:
                    door_master.push_state(0)
                    flip = True
        else: # this is override mode (file based)
            if event_handler.state == 1:
                door_master.push_state(1)
            else:
                door_master.push_state(0)
        time.sleep(6)
except KeyboardInterrupt:
    observer.stop()
    mqtt_client.loop_stop()
observer.join()
