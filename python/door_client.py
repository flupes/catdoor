import paho.mqtt.client as mqtt

def doormaster_command(msg):
    print("Got command : ")
    # print(type(msg.payload))
    print(bytes.decode(msg.payload))
    value = int(bytes.decode(msg.payload))
    if value == 1 :
        print("open")
    else:
        print("closed")

def deckdoor_state(msg):
    print("Got state : "+str(msg.payload))

def on_connect(client, userdata, flags, rc):
    print("Connected with results code "+str(rc))
    client.subscribe("/doormaster/#")
    client.subscribe("/deckdoor/#")

def on_message(client, userdata, msg):
    print("got new message")
    # print(client)
    # print(msg.topic)
    # print(msg.payload)
    # print(userdata)
    # print(userdata[msg.topic])
    # doormaster_command(msg)
    userdata[msg.topic](msg)

topiclist = {   '/doormaster/command' : doormaster_command,
                '/deskdoor/state' : deckdoor_state
}

print(doormaster_command)

client = mqtt.Client("DoorClient", True, topiclist)
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost")

client.loop_forever()
