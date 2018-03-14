#!/usr/bin/env python
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import time

taskList = str("")

def SendInitTaskListToRobot(robot):
    publish.single("SIP40_Factory/" + robot + "/SendInitTaskListToRobot", taskList, hostname="localhost")

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("SIP40_Factory/MobileRobot/GetInitTaskListFromRobot")

def on_message(client, userdata, msg):
    if msg.topic == str("SIP40_Factory/MobileRobot/GetInitTaskListFromRobot"):
        SendInitTaskListToRobot(msg.payload)
        
    elif msg.topic == str("SIP40_Factory/Anmeldung/Station"):
        SendInitTaskListToRobot(msg.payload)
        
    elif msg.topic == str("SIP40_Factory/Anmeldung/Station"):
        SendInitTaskListToRobot(msg.payload)

#client.loop_stop

try:
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.loop_start()

    while True:
        print(x, "Sending ...")
        publish.single("SIP40_Factory/Thing_MOR_1/BATTERY", x, hostname="192.168.1.96")
        time.sleep(0.25)
        
        if x > 99:
            x = 0
        else:
            x = x + 1
            
# Aufraeumarbeiten nachdem das Programm beendet wurde
except KeyboardInterrupt:
    GPIO.cleanup()