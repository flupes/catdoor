import paho.mqtt.client as mqtt

def doormaster_command(msg):
    print("Got command : "+msg.payload)

def doormaster_msg(msg):
    print("Got msg : "+msg.payload)

def on_connect(client, userdata, flags, rc):
    print("Connected with results code "+str(rc))
    client.subscribe("/doormaster/#")

def on_message(client, userdata, msg):
    # print("got new message")
    # print(msg.topic)
    # print(msg.payload)
    userdata[msg.topic](msg)

topiclist = {   '/doormaster/command' : doormaster_command,
                '/doormaster/msg' : doormaster_msg
}

client = mqtt.Client("DoorClient", True, topiclist)
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost")

client.loop_forever()
