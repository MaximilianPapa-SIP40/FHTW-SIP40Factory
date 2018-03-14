#!/usr/bin/env python
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import time
import serial

taskQueueMOR = str("")
morIdentity = -1
connected = False
stateMachineState = 0

ser = serial.Serial('/dev/ttyACM0',9600)
time.sleep(5) # der Arduino resettet nach einer seriellen Verbindung, daher muss kurz gewartet werden.
    
def UpdateMORTaskQueue(taskQueue):
    global taskQueueMOR
    taskQueueMOR = str(taskQueue, 'utf-8')
    print(taskQueueMOR)
    
def InitMOR(identity):
    global morIdentity
    morIdentity = str(identity, 'utf-8')
    print("MOR received the identity: " + str(morIdentity))
    
def TakeTask(taskID):
    publish.single("SIP40_Factory/MOR_" + str(morIdentity) + "/TakeTask", taskID, hostname="192.168.1.96")
    print("MOR_" + str(morIdentity) + " takes the task with id: " + str(taskID))

def on_connect(client, userdata, flags, rc):
    global stateMachineState
    print("Connected with result code " + str(rc))
    client.subscribe("SIP40_Factory/MOR_General/TaskQueue")
    client.subscribe("SIP40_Factory/Anmeldung/MOR/Identity")
    publish.single("SIP40_Factory/Anmeldung/MOR", 1, hostname="192.168.1.96")
    connected = True
    stateMachineState = 1

def on_message(client, userdata, msg):
    print("Received on : " + msg.topic)
        
    if msg.topic == str("SIP40_Factory/MOR_General/TaskQueue"):
        UpdateMORTaskQueue(msg.payload)
        
    elif msg.topic == str("SIP40_Factory/Anmeldung/MOR/Identity"):
        InitMOR(msg.payload)

#client.loop_stop

try:
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("192.168.1.96", 1883, 60)
    client.loop_start()
    time.sleep(5)
    
    while True:
        if ((stateMachineState) == 0 and (morIdentity == -1)):
            print("Retry to connect in 5 seconds!")
            time.sleep(5)
            client.connect("192.168.1.96", 1883, 60)
            
        if ((stateMachineState == 1) and (taskQueueMOR)):
            TakeTask(0)
            stateMachineState = 2;
            
        if (stateMachineState == 2):
            ser.write(str("F").encode())
            time.sleep(5) # work at task
            ser.write(str("S").encode())
            time.sleep(5) # work at task
            stateMachineState = 1;           
            
# Aufraeumarbeiten nachdem das Programm beendet wurde
except KeyboardInterrupt:
    print("Programm wurde geschlossen!")
    ser.close()
    #GPIO.cleanup()
