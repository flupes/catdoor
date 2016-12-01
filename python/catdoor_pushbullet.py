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
        print(time2str(self.last_beat)+" | got heartbeat"+
              " | average proximity = "+msg.payload)

class DoorState(object):
    def __init__(self, max_opened_state_sec):
        self.state = None
        self.up = None
        self.timer = None
        self.duration = max_opened_state_sec

    def door_stuck(self):
        self.up = False
        title = "Catdoor Warning"
        msg = "door may be stuck in opened position!"
        print(time2str(localtime())+" | " + msg)
        publish(title, msg)

    def door_cycle(self):
        global twilioClient
        self.up = False
        print(time2str(localtime())+" | door cycle complete.")
        title = "Catdoor Cycled"
        msg = "Misty just got in @ "+time2str(localtime())
        publish(title, msg)
        
    def new_state(self, msg):
        print (time2str(localtime())+" | new state = "+msg.payload)
        self.state = msg.payload
        if self.state == "open":
            self.up = True
            self.timer = threading.Timer(self.duration, self.door_stuck)
            self.timer.start()
        if self.state == "closed":
            if self.up == True:
                self.timer.cancel()
                self.door_cycle()
            else:
                print(time2str(localtime())+" | door back to closed position...")

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/catdoor/#")
    
def on_message(client, userdata, msg):
    userdata[msg.topic](msg)

beatcheck = Heartbeat(60)
doorstate = DoorState(6)
               
topiclist = { "/catdoor/avgproxim" : beatcheck.heartbeat,
              "/catdoor/state" : doorstate.new_state
              }
    
mqttClient = mqtt.Client("CatdoorSubscriber", True, topiclist)
mqttClient.on_connect = on_connect
mqttClient.on_message = on_message

mqttClient.connect("172.16.0.11")
          
mqttClient.loop_forever()
