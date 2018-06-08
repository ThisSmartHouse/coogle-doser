/*
  +----------------------------------------------------------------------+
  | CoogleDoser for ESP8266                                              |
  +----------------------------------------------------------------------+
  | Copyright (c) 2017-2018 John Coggeshall                              |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");      |
  | you may not use this file except in compliance with the License. You |
  | may obtain a copy of the License at:                                 |
  |                                                                      |
  | http://www.apache.org/licenses/LICENSE-2.0                           |
  |                                                                      |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied. See the License for the specific language governing         |
  | permissions and limitations under the License.                       |
  +----------------------------------------------------------------------+
  | Authors: John Coggeshall <john@thissmarthouse.com>                   |
  +----------------------------------------------------------------------+
*/

#include <CoogleIOT.h>
#include <AccelStepper.h>

#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif

#ifndef STEPPER_SPEED
#define STEPPER_SPEED 1000
#endif

#ifndef STEPPER_ACCEL
#define STEPPER_ACCEL 500
#endif

#define ACTION_TOPIC "/peristalic/freshwater/1"
#define STATE_TOPIC "/peristalic/freshwater/1/state"

CoogleIOT *iot;
PubSubClient *mqtt;

AccelStepper stepper(AccelStepper::DRIVER, D1, D2);

char msg[150];
int turnsToExecute = 0;

void setup() {

  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD);
  iot->initialize();
   
  iot->info("CoogleDoser Initializing...");
  iot->info("-=-=-=-=--=--=-=-=-=-=-=-=-=-=-=-=-=-");
  iot->logPrintf(INFO, "MQTT Action Topic: %s", ACTION_TOPIC);
  iot->logPrintf(INFO, "MQTT State Topic: %s", STATE_TOPIC);
  iot->info("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

  stepper.setMaxSpeed(STEPPER_SPEED);
  stepper.setAcceleration(STEPPER_ACCEL);
  stepper.setSpeed(STEPPER_SPEED);
    
  iot->info("");

  if(iot->mqttActive()) {
      mqtt = iot->getMQTTClient();
    
      mqtt->setCallback(mqttCallback);

      mqtt->publish(STATE_TOPIC, "0", true);
      mqtt->subscribe(ACTION_TOPIC);
           
      iot->info("Coogle Doser Initialized!");
  } else {
    iot->error("Initialization failure, invalid MQTT Server connection.");
  }
   
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{ 
  String input((char *)payload);
  
  turnsToExecute = input.toInt();
  
  iot->logPrintf(DEBUG, "MQTT Callback Triggered. Topic: %s\n", topic);
  iot->logPrintf(DEBUG, "Turns to execute: %d", turnsToExecute);

  iot->info("Action Complete!");
}

void loop() {
  iot->loop();
  
  if(stepper.distanceToGo() == 0) {
    if(turnsToExecute > 0) {
      stepper.moveTo(stepper.currentPosition() + (1600 * turnsToExecute));
      turnsToExecute = 0;

      //mqtt->publish(STATE_TOPIC, String(turnsToExecute).c_str(), true);
       
    }
  }
  
  stepper.run();
}
