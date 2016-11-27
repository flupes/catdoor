import paho.mqtt.client as mqtt

def catdoor_avgproxim(msg):
    print("Got avgproxim : "+msg.payload)

def catdoor_state(msg):
    print ("Got new state : "+msg.payload)


def on_connect(client, userdata, flags, rc):
    print("Connected with results code "+str(rc))
    client.subscribe("/catdoor/#")
    
def on_message(client, userdata, msg):
    userdata[msg.topic](msg)
    
topiclist = { "/catdoor/avgproxim" : catdoor_avgproxim,
              "/catdoor/state" : catdoor_state
              }
    
client = mqtt.Client("CatdoorListener", True, topiclist)
client.on_connect = on_connect
client.on_message = on_message

client.connect("172.16.0.11")
          
client.loop_forever()

