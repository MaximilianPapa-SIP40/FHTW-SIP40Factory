#!/usr/bin/env python
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import time

taskQueueMOR = str("")
    
def InitStation(client, stationName):
    publish.single("SIP40_Factory/" + str(stationName, 'utf-8') + "/TaskInProgress", 1, hostname="localhost")
    client.subscribe("SIP40_Factory/" + str(stationName, 'utf-8') + "/NextTaskForMOR")
    client.subscribe("SIP40_Factory/" + str(stationName, 'utf-8') + "/TaskInProgress")
    print("Sent on: SIP40_Factory/" + str(stationName, 'utf-8') + "/TaskInProgress")
    
def InitMOR(client, robotName):
    publish.single("SIP40_Factory/MOR_General/TaskQueue", taskQueueMOR, hostname="localhost")
    client.subscribe("SIP40_Factory/" + str(robotName, 'utf-8') + "/TakeTask")
    print("Sent on: SIP40_Factory/MOR_General/TaskQueue")
    
def AddTaskInMORQueue(client, task):
    global taskQueueMOR
    if(taskQueueMOR):
        taskQueueMOR = taskQueueMOR + ";"
    
    taskQueueMOR = taskQueueMOR + str(task, 'utf-8')
    print(taskQueueMOR)
    publish.single("SIP40_Factory/MOR_General/TaskQueue", taskQueueMOR, hostname="localhost")
    
def RemoveTaskFromMORQueue(client, taskNumber):
    global taskQueueMOR
    
    tempList = taskQueueMOR.split(";")
    tempTaskQueue = str("")
    
    for x in range(len(tempList)):
        print(str(x) + " " + str(taskNumber, 'utf-8'))
        if(x != int(taskNumber)):
              tempTaskQueue = tempTaskQueue + tempList[int(x)] + ";"
        else:
            continue
    
    tempTaskQueue = tempTaskQueue[:-1]
    taskQueueMOR = tempTaskQueue
    print(taskQueueMOR)
    publish.single("SIP40_Factory/MOR_General/TaskQueue", taskQueueMOR, hostname="localhost")
    
def SendNewTaskToStation(client, station, msg):
    if(str(msg, 'utf-8') == str("0")):
        print("Send to: " + station)
        publish.single("SIP40_Factory/" + station +"/TaskInProgress", 1, hostname="localhost")

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("SIP40_Factory/MOR_General/GetInitTaskListFromRobot")
    client.subscribe("SIP40_Factory/Anmeldung/Station")
    client.subscribe("SIP40_Factory/Anmeldung/MOR")

def on_message(client, userdata, msg):
    print("Received on : " + msg.topic)
        
    if msg.topic == str("SIP40_Factory/Anmeldung/Station"):
        InitStation(client, msg.payload)
        
    elif msg.topic == str("SIP40_Factory/Anmeldung/MOR"):
        InitMOR(client, msg.payload)
        
    elif(msg.topic.split("/")[2] == "NextTaskForMOR"):
        AddTaskInMORQueue(client, msg.payload)
        
    elif(msg.topic.split("/")[2] == "TakeTask"):
        RemoveTaskFromMORQueue(client, msg.payload)
        
    elif(msg.topic.split("/")[2] == "TaskInProgress"):
        SendNewTaskToStation(client, msg.topic.split("/")[1], msg.payload)

#client.loop_stop

try:
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.loop_forever()
    #client.loop_start()

    #while True:
        #def AddTaskForMOR(client, task):
        
        #publish.single("SIP40_Factory/Thing_MOR_1/BATTERY", x, hostname="192.168.1.96")
        #time.sleep(0.25)
        
        #if x > 99:
        #    x = 0
        #else:
        #    x = x + 1
            
# Aufraeumarbeiten nachdem das Programm beendet wurde
except KeyboardInterrupt:
    print("Programm wurde geschlossen!")
    #GPIO.cleanup()