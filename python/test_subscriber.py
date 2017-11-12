import paho.mqtt.client as mqtt

def catdoor_avgproxim(msg):
    print("Got avgproxim : "+msg.payload)

def catdoor_state(msg):
    print ("Got new state : "+msg.payload)

def catdoor_heartbeat(msg):
    print ("Got new heartbeat : "+msg.payload)

def catdoor_message(msg):
    print ("Got new message : "+msg.payload)

def catdoor_solenoids(msg):
    print ("Got new solenoids : "+msg.payload)

def catdoor_proximity(msg):
    print ("Got new proximity : "+msg.payload)

def catdoor_doorstate(msg):
    print ("Got new doorstate : "+msg.payload)

def on_connect(client, userdata, flags, rc):
    print("Connected with results code "+str(rc))
    client.subscribe("/catdoor2/#")
    
def on_message(client, userdata, msg):
    userdata[msg.topic](msg)
    
topiclist = { "/catdoor/avgproxim" : catdoor_avgproxim,
              "/catdoor/state" : catdoor_state,
              "/catdoor2/heartbeat" : catdoor_heartbeat,
              "/catdoor2/message" : catdoor_message,
              "/catdoor2/solenoids" : catdoor_solenoids,
              "/catdoor2/doorstate" : catdoor_doorstate,
              "/catdoor2/proximity" : catdoor_proximity
}
    
client = mqtt.Client("CatdoorListener", True, topiclist)
client.on_connect = on_connect
client.on_message = on_message

client.connect("172.16.0.11")
          
client.loop_forever()

