#!/usr/bin/env python
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import time

taskID = 0
taskQueueMOR = str("empty")
morIdentity = 0

def InitStation(client, stationName):
    publish.single("SIP40_Factory/" + str(stationName, 'utf-8') + "/TaskForStation", 1, hostname="localhost")
    client.subscribe("SIP40_Factory/" + str(stationName, 'utf-8') + "/NextTaskForMOR")
    client.subscribe("SIP40_Factory/" + str(stationName, 'utf-8') + "/TaskInProgress")
    print("Sent on: SIP40_Factory/" + str(stationName, 'utf-8') + "/TaskInProgress")
    
def InitMOR(client):
    global morIdentity
    morIdentity = morIdentity + 1
    publish.single("SIP40_Factory/Anmeldung/MOR/Identity", str(morIdentity), hostname="localhost")
    print("Gave MOR the identity: " + str(morIdentity))
    
    publish.single("SIP40_Factory/MOR_General/TaskQueue", taskQueueMOR, hostname="localhost")
    client.subscribe("SIP40_Factory/MOR_" + str(morIdentity) + "/TakeTask")
    print("Sent on: SIP40_Factory/MOR_General/TaskQueue")
    
def AddTaskInMORQueue(client, task):
    global taskID
    global taskQueueMOR
    if(taskQueueMOR):
        taskQueueMOR = taskQueueMOR + ";"
    
    taskQueueMOR = taskQueueMOR + str(taskID) + "-" + str(task, 'utf-8')
    taskID = taskID + 1
    print(taskQueueMOR)
    publish.single("SIP40_Factory/MOR_General/TaskQueue", taskQueueMOR, hostname="localhost")
    
def RemoveTaskFromMORQueue(client, taskNumber):
    global taskQueueMOR
    
    tempList = taskQueueMOR.split(";")
    tempTaskQueue = str("")
    
    for x in range(len(tempList)):
        if (tempList[int(x)] != "empty"):
            tempIDList = tempList[int(x)].split("-")
            print(tempIDList[0])
            print(tempIDList[0] + " " + str(taskNumber, 'utf-8'))
            if(int(tempIDList[0]) != int(taskNumber)):
                  tempTaskQueue = tempTaskQueue + tempList[int(x)] + ";"
            else:
                continue
        else:
            tempTaskQueue = tempTaskQueue + tempList[int(x)] + ";"
    
    tempTaskQueue = tempTaskQueue[:-1]
    taskQueueMOR = tempTaskQueue
    print(taskQueueMOR)
    publish.single("SIP40_Factory/MOR_General/TaskQueue", taskQueueMOR, hostname="localhost")
    
def SendNewTaskToStation(client, station, msg):
    if(str(msg, 'utf-8') == str("0")):
        print("Send to: " + station)
        publish.single("SIP40_Factory/" + station +"/TaskForStation", 1, hostname="localhost")

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
        InitMOR(client)
        
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