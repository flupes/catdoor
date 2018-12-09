import time

import paho.mqtt.client as mqtt

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from watchdog.events import FileCreatedEvent
from watchdog.events import FileDeletedEvent
from watchdog.events import FileModifiedEvent

#
# Commanded States:
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


mqtt_client = mqtt.Client("DoorMaster")
door_master = DoorMaster(mqtt_client, "localhost")

event_handler = FileChangedHandler()
event_handler.read_state()

observer = Observer()
observer.schedule(event_handler, ".")
observer.start()
mqtt_client.loop_start()

flip = True
elapsed = time.time()
try:
    while True:
        now = time.time()
        if (now-elapsed > 6):
            if event_handler.state == 2:
                if flip:
                    door_master.push_state(1)
                    flip = False
                else:
                    door_master.push_state(0)
                    flip = True
            else:
                if event_handler.state == 1:
                    door_master.push_state(1)
                else:
                    door_master.push_state(0)
            elapsed = now
        #if 9 <= now.tm_hour and now.tm_hour <= 17:
        time.sleep(1)

except KeyboardInterrupt:
    observer.stop()
    mqtt_client.loop_stop()
observer.join()
