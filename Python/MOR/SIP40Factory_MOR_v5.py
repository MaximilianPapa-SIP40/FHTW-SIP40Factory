#!/usr/bin/env python
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import time
import serial

taskQueueMOR = str("")
morIdentity = -1
connected = False
stateMachineState = 0
lastBatteryInformationsSend = 0
stationToggle = False

# Digitale Repräsentation der Fabrik mittels 2D Matrix
w, h = 5, 5;
Matrix = [[99999 for x in range(w)] for y in range(h)]
Matrix[0][0] = 10001
Matrix[0][1] = 31010
Matrix[0][2] = 21110
Matrix[0][3] = 31010
Matrix[0][4] = 10003
Matrix[1][2] = 30101
Matrix[2][0] = 10000
Matrix[2][1] = 31010
Matrix[2][2] = 21111
Matrix[2][3] = 31010
Matrix[2][4] = 10004
Matrix[3][2] = 30101
Matrix[4][0] = 10002
Matrix[4][1] = 31010
Matrix[4][2] = 21100

def PrintFactoryMap():
    print("")
    print("Map of the Factory: ")
    for y in range(0, 5):
        for x in range(0, 5):
            print(Matrix[x][y], end='', flush=True)
            if x != 4:
                print(" ", end='', flush=True)
        print("")

#serial = serial.Serial('/dev/ttyACM0',9600)
#time.sleep(5) # der Arduino resettet nach einer seriellen Verbindung, daher muss kurz gewartet werden.
    
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
    
    PrintFactoryMap()
    
    while True:
        if ((stateMachineState) == 0 and (morIdentity == -1)):
            print("Retry to connect in 5 seconds!")
            time.sleep(5)
            client.connect("192.168.1.96", 1883, 60)
            
        if ((stateMachineState == 1) and (taskQueueMOR)):
            TakeTask(0)
            stateMachineState = 2;
            
        if (stateMachineState == 2):            
            if(stationToggle == False):
                #serial.write(str("F").encode())
                stationToggle = not stationToggle
            else:
                print("Turn")
                #serial.write(str("T").encode())
                
            robotAtNextCross = False;
            while not robotAtNextCross:
                #serial.reset_input_buffer()
                #zumoState = serial.readline()
                zumoState = str(zumoState, 'utf-8')
                zumoState = zumoState[:-2]
                if (zumoState == "N"):
                    robotAtNextCross = True
                    print("Received from Zumo: Next Cross Arrived")
                
            time.sleep(5) # work at task
            stateMachineState = 1;
            print("StateMachine 1")
            
        # Update battery informations every 5 seconds
        #if ((time.time() - lastBatteryInformationsSend) > 5):
            #serial.reset_input_buffer()
            #lastBatteryInformationsSend = time.time()
            #batteryLevel = serial.readline()
            #batteryLevel = str(batteryLevel, 'utf-8')
            #batteryLevel = batteryLevel[:-2]
            #publish.single("SIP40_Factory/MOR_" + str(morIdentity) + "/Battery", batteryLevel, hostname="192.168.1.96")
            #print(batteryLevel)
            
# Aufraeumarbeiten nachdem das Programm beendet wurde
except KeyboardInterrupt:
    print("Programm wurde geschlossen!")
    #serial.close()
    #GPIO.cleanup()
