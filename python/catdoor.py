# Monitor the catdoor using MQTT topics

from time import localtime, strftime
from threading import Timer

import paho.mqtt.client as mqtt

ALLOWED_SILENCE_SEC = 30

def failed_heartbeat(last_heard):
    print("Catdoor did not publish heartbeat since: "+
          strftime("%Y-%m-%d_%H:%M:%S",catdoor_heartbeat.last_beat))
    
def catdoor_heartbeat(msg):
    global ALLOWED_SILENCE_SEC
    if catdoor_heartbeat.last_beat == None:
        catdoor_heartbeat.last_beat = localtime()
        print("First time I heard of the catdoor: "+
              strftime("%Y-%m-%d_%H:%M:%S",catdoor_heartbeat.last_beat))
        # set timer
        catdoor_heartbeat.timer = Timer(ALLOWED_SILENCE_SEC, failed_heartbeat,
                                        args=[catdoor_heartbeat.last_beat])
        catdoor_heartbeat.timer.start()
    else:
        catdoor_heartbeat.last_beat = localtime()
        # reset timer
        if catdoor_heartbeat.timer != None:
            catdoor_heartbeat.timer.cancel()
        catdoor_heartbeat.timer = Timer(ALLOWED_SILENCE_SEC, failed_heartbeat,
                                        args=[catdoor_heartbeat.last_beat])
        catdoor_heartbeat.timer.start()
    print("Got avgproxim : "+msg.payload+" / time="+
          strftime("%Y-%m-%d_%H:%M:%S",catdoor_heartbeat.last_beat))

catdoor_heartbeat.last_beat  = None
catdoor_heartbeat.timer = None

def catdoor_state(msg):
    print ("Got new state : "+msg.payload)


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/catdoor/#")
    
def on_message(client, userdata, msg):
    userdata[msg.topic](msg)
    
topiclist = { "/catdoor/avgproxim" : catdoor_heartbeat,
              "/catdoor/state" : catdoor_state
              }
    
client = mqtt.Client("CatdoorSubscriber", True, topiclist)
client.on_connect = on_connect
client.on_message = on_message

client.connect("172.16.0.11")
          
client.loop_forever()

