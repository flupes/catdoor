# Monitors the catdoor using MQTT topics
# and sends notification using pushbullet

from time import localtime, strftime, sleep
from datetime import datetime
from array import array

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
        self.out_of_sync = None
        self.duration = max_silence_sec

    def fresh_heartbeat(self):
        self.alive = True
        title = "Catdoor Notification"
        msg = "Catdoor is alive (first heartbeat @ " + time2str(self.last_beat) + ")"
        print(time2str(localtime()) + " | " + msg)
        publish(title, msg)

    def missing_heartbeat(self):
        self.alive = False
        self.out_of_sync = None
        title = "Catdoor Error"
        msg = "Catdoor considered dead (last published heartbeat @ "+time2str(self.last_beat)+")"
        print(time2str(localtime())+" | " +msg)
        publish(title, msg)

    def heartbeat(self, msg):
        self.last_beat = localtime()
        print(time2str(self.last_beat)+" | got heartbeat msg : "+msg.payload)
        if not self.alive:
            self.fresh_heartbeat()
        if self.timer != None:
            self.timer.cancel()
        self.timer = threading.Timer(self.duration, self.missing_heartbeat)
        self.timer.start()
        self.check_sync(msg)

    def check_sync(self, msg):
        args = msg.payload.split()
        strt = args[0]+" "+args[1]
        rtc = datetime.strptime(strt, '%Y-%m-%d %H:%M:%S')
        now = datetime.now()
        dst = time.localtime().tm_isdst
        diff = (rtc-now+dst*3600).total_seconds()
        if abs(diff) > 20:
            if self.out_of_sync != True:
                title = "Catdoor Notification"
                offset = "Catdoor clock is not in sync! (offset="+\
                    str(int(diff))+"s)"
                publish(title, offset)
                self.out_of_sync = True
        else:
            if self.out_of_sync != False:
                title = "Catdoor Notification"
                offset = "Catdoor clock is in sync. (offset="+\
                    str(int(diff))+"s)"
                publish(title, offset)
                self.out_of_sync = False


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
            msg = "Luna went out for a walk @ "+time2str(localtime())
        if self.open_in:
            self.open_in = False
            direction = "IN"
            msg = "Luna came in for confort @ "+time2str(localtime())
        print(time2str(localtime())+" | door cycle " + direction + " complete.")
        title = "Catdoor Cycled " + direction
        publish(title, msg)

    def new_state(self, msg):
        print (time2str(localtime())+" | got new doorstate : "+msg.payload)
        self.state = msg.payload.split()[3]
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

class BatteryMonitor(object):
    def __init__(self, averaging_window_size):
        self.low_battery_threshold = 3.65
        self.use_battery_threshold = 4.15
        self.battery_full_threshold = 4.20
        self.charging_threshold = 3.75
        self.no_battery_threshold = 4.35
        self.significant_change = 0.04
        self.values = array('f')
        self.window = averaging_window_size
        self.index = 0
        self.mode = 'UNKNOWN'
        self.prev_volts = self.use_battery_threshold

    def __add(self, v):
        if len(self.values) < self.window:
            self.values.append(v)
        else :
            self.values[self.index] = v
        self.index = self.index+1
        if self.index == self.window:
            self.index = 0

    def __avg(self):
        sum = 0.0
        # inefficient method to avoid juggling with indexes
        for v in self.values:
            sum = sum + v
        return sum / len(self.values)

    def new_voltage(self, msg):
        print (time2str(localtime())+" | got new battery_v : "+msg.payload)
        self.__add(float(msg.payload.split()[3]))
        volts = self.__avg()
        print ("Average Voltage = %.3f V / current mode = %s") % (volts, self.mode)
        if volts > self.no_battery_threshold:
            if self.mode != 'NO_BATTERY':
                self.mode = 'NO_BATTERY'
                publish("Catdoor Warning", "No battery present!")
        elif volts < self.low_battery_threshold:
            if self.mode != 'LOW_BATTERY':
                self.mode = 'LOW_BATTERY'
                publish("Catdoor Error", "LOW battery!")
        elif self.mode == 'LOW_BATTERY':
            if volts > self.charging_threshold:
                self.mode = 'CHARGING'
            elif volts > self.low_battery_threshold:
                self.mode = 'BATTERY'
        elif self.battery_full_threshold < volts and volts < self.no_battery_threshold:
            if self.mode != 'FULL':
                self.mode = 'FULL'
                publish("Catdoor Notification", "Battery is fully charged!")
        elif volts < self.use_battery_threshold and \
            volts < self.prev_volts:
            if self.mode != 'BATTERY':
                self.mode = 'BATTERY'
                publish("Catdoor Warning", "Running on Battery Power")
        elif volts > self.charging_threshold and \
            volts > self.prev_volts:
            if self.mode != 'CHARGING':
                self.mode = 'CHARGING'
                publish("Catdoor Notification", "Running on External Power")
        if abs(self.prev_volts-volts) > self.significant_change:
            self.prev_volts = volts

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/catdoor/#")

def on_message(client, userdata, msg):
    userdata[msg.topic](msg)

def catdoor_message(msg):
    print (time2str(localtime())+" | got new message : "+msg.payload)
    title = "Catdoor Notification"
    args = msg.payload.split()
    key = args[3]
    if key == "LOCKED":
        msg = "LOCKED (will re-open tomorrow morning around "+args[3]+")"
    elif key == "UNLOCKED":
        msg = "UNLOCKED (will close this afternoon at "+args[3]+")"
    else:
        msg = msg.payload
    publish(title, msg)

def catdoor_solenoids(msg):
    print (time2str(localtime())+" | got new solenoids : "+msg.payload)

def catdoor_proximity(msg):
    print (time2str(localtime())+" | got new proximity : "+msg.payload)

beatcheck = Heartbeat(90)
doorstate = DoorState(60)
battery_monitor = BatteryMonitor(9)

print("catdoor_pusbullet starting...")

topiclist = {
    "/catdoor/message" : catdoor_message,
    "/catdoor/heartbeat" : beatcheck.heartbeat,
    "/catdoor/solenoids" : catdoor_solenoids,
    "/catdoor/doorstate" : doorstate.new_state,
    "/catdoor/proximity" : catdoor_proximity,
    "/catdoor/battery_v" : battery_monitor.new_voltage
}

mqttClient = mqtt.Client("CatdoorSubscriber", True, topiclist)
mqttClient.on_connect = on_connect
mqttClient.on_message = on_message

mqttClient.connect("172.16.0.11")

mqttClient.loop_forever()
