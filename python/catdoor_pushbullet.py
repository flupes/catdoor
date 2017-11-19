# Monitors the catdoor using MQTT topics
# and sends notification using pushbullet

from time import localtime, strftime, sleep

import threading

import netrc

import paho.mqtt.client as mqtt

from pushbullet import Pushbullet

def time2str(t):
    return strftime("%Y-%m-%d_%H:%M:%S",t)

try:
    netrckeys = netrc.netrc()
    API_KEY = netrckeys.authenticators('catdoor')[2]
except Exception:
    print("Failed to get the catdoor key from the .netrc file!")
    quit()

def pb_publish(title, msg):
    global API_KEY
    published = False
    while not published:
        try:
            print "pushing notification:"
            print "  title = " + title
            print "  message = " + msg
            pb = Pushbullet(API_KEY)
            push = pb.push_note(title, msg)
            published = True
        except Exception, ex:
            print "Unable to send notification: %s" % ex
            print "Will try again in 2 minutes"
            sleep(120)

def publish(title, msg):
    thread = threading.Thread(target=pb_publish, args=[title, msg])
    thread.start()

class Heartbeat(object):
    def __init__(self, max_silence_sec=60):
        self.alive = False
        self.last_beat = None
        self.timer = None
        self.duration = max_silence_sec

    def fresh_heartbeat(self):
        self.alive = True
        title = "Catdoor Notification"
        msg = "Catdoor is alive (first heartbeat @ " + time2str(self.last_beat) + ")"
        print(time2str(localtime()) + " | " + msg)
        publish(title, msg)

    def missing_heartbeat(self):
        self.alive = False
        title = "Catdoor Error"
        msg = "Catdoor considered dead (last published heartbeat @ "+time2str(self.last_beat)+")"
        print(time2str(localtime())+" | " +msg)
        publish(title, msg)

    def heartbeat(self, msg):
        self.last_beat = localtime()
        if not self.alive:
            self.fresh_heartbeat()
        if self.timer != None:
            self.timer.cancel()
        self.timer = threading.Timer(self.duration, self.missing_heartbeat)
        self.timer.start()
        print(time2str(self.last_beat)+" | got heartbeat msg : "+msg.payload)

class DoorState(object):
    def __init__(self, max_opened_state_sec):
        self.state = None
        self.open_out = None
        self.open_in = None
        self.jammed = None
        self.timer = None
        self.duration = max_opened_state_sec

    def door_jammed(self):
        self.jammed = True
        title = "Catdoor Warning"
        msg = "Door did not close correctly! @ "+time2str(localtime())
        print(time2str(localtime())+" | " + msg)
        publish(title, msg)

    def door_unstuck(self):
        self.jammed = False
        title = "Catdoor Notification"
        msg = "Door closed normally after being stuck @ "+time2str(localtime())
        print(time2str(localtime())+" | " + msg)
        publish(title, msg)

    def door_cycle(self):
        if self.open_out:
            self.open_out = False
            direction = "OUT"
            msg = "Misty went out for a walk @ "+time2str(localtime())
        if self.open_in:
            self.open_in = False
            direction = "IN"
            msg = "Misty came in for confort @ "+time2str(localtime())
        print(time2str(localtime())+" | door cycle " + direction + " complete.")
        title = "Catdoor Cycled " + direction
        publish(title, msg)

    def new_state(self, msg):
        print (time2str(localtime())+" | got new doorstate : "+msg.payload)
        self.state = msg.payload.split()[2]
        if self.state == "OPEN_OUT":
            self.open_out = True
        if self.state == "OPEN_IN":
            self.open_in = True
        if self.state == "OPEN_OUT" or self.state == "AJAR_OUT":
            if self.timer != None:
                self.timer = threading.Timer(self.duration, self.door_jammed)
                self.timer.start()
        if self.state == "CLOSED":
            print(time2str(localtime())+" | door back to closed position.")
            if self.timer != None:
                self.timer.cancel()
                self.timer = None
            if self.jammed == True:
                self.door_unstuck()
            if self.open_in == True or self.open_out == True:
                self.door_cycle()


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/catdoor/#")

def on_message(client, userdata, msg):
    userdata[msg.topic](msg)

def catdoor_battery_v(msg):
    print (time2str(localtime())+" | got new battery_v : "+msg.payload)

def catdoor_message(msg):
    print (time2str(localtime())+" | got new message : "+msg.payload)

def catdoor_solenoids(msg):
    print (time2str(localtime())+" | got new solenoids : "+msg.payload)

def catdoor_proximity(msg):
    print (time2str(localtime())+" | got new proximity : "+msg.payload)

beatcheck = Heartbeat(90)
doorstate = DoorState(60)

topiclist = {
    "/catdoor/message" : catdoor_message,
    "/catdoor/heartbeat" : beatcheck.heartbeat,
    "/catdoor/solenoids" : catdoor_solenoids,
    "/catdoor/doorstate" : doorstate.new_state,
    "/catdoor/proximity" : catdoor_proximity,
    "/catdoor/battery_v" : catdoor_battery_v
}

mqttClient = mqtt.Client("CatdoorSubscriber", True, topiclist)
mqttClient.on_connect = on_connect
mqttClient.on_message = on_message

mqttClient.connect("172.16.0.11")

mqttClient.loop_forever()
