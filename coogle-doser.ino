#include <CoogleIOT.h>
#include "Stepper2.h"

#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif

#ifndef STEPPER_RPMS
#define STEPPER_RPMS 15
#endif

#define ACTION_TOPIC "/peristalic/freshwater/1"
#define STATE_TOPIC "/peristalic/freshwater/1/state"

CoogleIOT *iot;
PubSubClient *mqtt;

char msg[150];

int pinOuts[4] = { D1, D2, D5, D6 };

Stepper2 dosingStepper(pinOuts);

void setup() {

  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD);
  iot->initialize();
   
  iot->info("CoogleDoser Initializing...");
  iot->info("-=-=-=-=--=--=-=-=-=-=-=-=-=-=-=-=-=-");
  iot->logPrintf(INFO, "Doser RPM: %d\n", STEPPER_RPMS);
  iot->logPrintf(INFO, "MQTT Action Topic: %s", ACTION_TOPIC);
  iot->logPrintf(INFO, "MQTT State Topic: %s", STATE_TOPIC);
  iot->info("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

  dosingStepper.setSpeed(STEPPER_RPMS);
  
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
  int turnsToExecute = input.toInt();
  
  iot->logPrintf(DEBUG, "MQTT Callback Triggered. Topic: %s\n", topic);
  iot->logPrintf(DEBUG, "Turns to execute: %d", turnsToExecute);

  if(turnsToExecute > 0) {
    
    iot->info("Executing clock-wise");
    
    dosingStepper.setDirection(0);

    for(int i = turnsToExecute; i > 0; i--) {
      mqtt->publish(STATE_TOPIC, String(i).c_str(), true);
      yield();
      iot->loop();
      dosingStepper.turn();
    }
    
  } else if(turnsToExecute < 0) {
    
    iot->info("Executing counter-clockwise");
    dosingStepper.setDirection(1);
    
    for(int i = turnsToExecute; i < 0; i++) {
      mqtt->publish(STATE_TOPIC, String(i).c_str(), true);
      yield();
      iot->loop();
      dosingStepper.turn();
    }
        
  } 

  mqtt->publish(STATE_TOPIC, "0", true);
  dosingStepper.stop();
  
  iot->info("Action Complete!");
}

void loop() {
  iot->loop();
}
