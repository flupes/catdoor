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

def catdoor_battery_v(msg):
    print ("Got new battery_v : "+msg.payload)

def on_connect(client, userdata, flags, rc):
    print("Connected with results code "+str(rc))
    client.subscribe("/catdoor/#")
    
def on_message(client, userdata, msg):
    userdata[msg.topic](msg)
    
topiclist = { "/catdoor/avgproxim" : catdoor_avgproxim,
              "/catdoor/state" : catdoor_state,
              "/catdoor/heartbeat" : catdoor_heartbeat,
              "/catdoor/message" : catdoor_message,
              "/catdoor/solenoids" : catdoor_solenoids,
              "/catdoor/doorstate" : catdoor_doorstate,
              "/catdoor/proximity" : catdoor_proximity,
              "/catdoor/battery_v" : catdoor_battery_v
}
    
client = mqtt.Client("CatdoorListener", True, topiclist)
client.on_connect = on_connect
client.on_message = on_message

client.connect("172.16.0.11")
          
client.loop_forever()

