# Monitor the catdoor using MQTT topics

from time import localtime, strftime
from threading import Timer

import paho.mqtt.client as mqtt

from twilio.rest import TwilioRestClient
account_sid = "..."
auth_token = "..."
twilioClient = TwilioRestClient(account_sid, auth_token)

def time2str(t):
    return strftime("%Y-%m-%d_%H:%M:%S",t)

class Heartbeat(object):
    def __init__(self, max_silence_sec=60):
        self.alive = False
        self.last_beat = None
        self.timer = None
        self.duration = max_silence_sec
    
    def fresh_heartbeat(self):
        self.alive = True
        print("Catdoor is alive at: "+time2str(self.last_beat))
    
    def missing_heartbeat(self):
        self.alive = False
        print(time2str(localtime())+" | Catdoor considered dead ("+
              "last published heartbeat: "+time2str(self.last_beat)+")")
              
    def heartbeat(self, msg):
        self.last_beat = localtime()
        if not self.alive:
            self.fresh_heartbeat()
        if self.timer != None:
            self.timer.cancel()
        self.timer = Timer(self.duration, self.missing_heartbeat)
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
        print(time2str(localtime())+" | door may be stuck in opened position!")

    def door_cycle(self):
        global twilioClient
        self.up = False
        print(time2str(localtime())+" | door cycle complete.")
        # This is incomplete because not resilient to network issues!
        # Need to catch exception and retry if we want to use it
        # @TODO Fix this lame code before using
        message = twilioClient.messages.create(to="+16508610123",
                                               from_="+14084703047",
                                               body="Misty just go in @ "+time2str(localtime()))
        
    def new_state(self, msg):
        print (time2str(localtime())+" | new state = "+msg.payload)
        self.state = msg.payload
        if self.state == "open":
            self.up = True
            self.timer = Timer(self.duration, self.door_stuck)
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

beatcheck = Heartbeat(20)
doorstate = DoorState(6)
               
topiclist = { "/catdoor/avgproxim" : beatcheck.heartbeat,
              "/catdoor/state" : doorstate.new_state
              }
    
mqttClient = mqtt.Client("CatdoorSubscriber", True, topiclist)
mqttClient.on_connect = on_connect
mqttClient.on_message = on_message

mqttClient.connect("172.16.0.11")
          
mqttClient.loop_forever()

