
/*--------------------------------------------------/
 DESIGNSYNTHESIS Inc.  5-4-2023                 /
 BiFold relay control                               /
 for capacitive switched bifolding doors            /
 Switch activation requires three(3)sequencial taps /
 Work in conjunction with Progressive Automation    /
 Actuator controller.                               /
 v-5.0a  r.young 2-5-2019 --   
 v-5.1   r.young 1-4-2023 -- 
 v-5.2   r.young 5-4-2023 --                        /
 revised 12-20-2022 r. young --                     /
 removed all wifi and MQTT Capacities               /
---------------------------------------------------*/
#include <FS.h>
#include <ArduinoJson.h>    
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <AceButton.h>
using namespace ace_button;

//#include <ButtonConfig.h>
#include <Atm_esp8266.h>
 
long lastMsg = 0;
char msg[50];
int value = 0;
unsigned long   BEACON_DELAY = 2000;
unsigned long   INTERVAL_DURATION = 500;
unsigned long   RELAY_FIRED;
unsigned long   ONLINE_START;
unsigned long   TOP_RANGE;
const int       BUTTON_PIN = D0;



Atm_led led;
Atm_timer timer;
int DOOR_STATE = 0;
int LAST_STATE = 0;
int LAST_COMMAND = 0;


bool STARTUP= false;

AceButton button(BUTTON_PIN);

void handleEvent(AceButton*, uint8_t, uint8_t);
void handleTestEvent(AceButton*, uint8_t, uint8_t);

//flag for saving data
bool shouldSaveConfig = false;

//define your default values here, if there are different values in config.json, they are overwritten.
//=====================================================================================================

//====================================================================================================

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void OnStateChanged()
{
  
  if(DOOR_STATE==1 && LAST_STATE==2){
      digitalWrite(D5,LOW);
      RELAY_FIRED = millis();
      Serial.println("HIGH");
      LAST_STATE=1;
      EEPROM.write(1,LAST_STATE);
      EEPROM.commit();
    }
  if(DOOR_STATE==2 && LAST_STATE==1){
      digitalWrite(D6,LOW);
      RELAY_FIRED = millis();
      Serial.println("LOW");
      LAST_STATE=2;
      EEPROM.write(1,LAST_STATE);
      EEPROM.commit();
    }
   
}
/*
Main button handlers set to 3 taps to activate
*/
void handleTestEvent(AceButton* /*button*/, uint8_t eventType,
    uint8_t /* buttonState */) {
 switch (eventType) {
    case AceButton::kEventClicked:
      break;
    case AceButton::kEventDoubleClicked:
      Serial.print("--(0)");
      if(DOOR_STATE==1){
        DOOR_STATE=2;
        LAST_STATE=1;     
        Serial.print("Door State=");
        Serial.println(DOOR_STATE);
      }else{
        DOOR_STATE=1;
        LAST_STATE=2;
        Serial.print("Door State=");
        Serial.println(DOOR_STATE);        
      }
     break; 
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(9600);

  //clean FS, for testing
//SPIFFS.format();

 //--------------PINS-------------------------------
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  digitalWrite(D5,HIGH);
  digitalWrite(D6,HIGH);

  

 
  //-------------------------------------------------
  EEPROM.begin(512); 
  LAST_STATE = EEPROM.read(0);
  LAST_COMMAND=EEPROM.read(1);
  Serial.print("LAST_STATE=");
  Serial.println(LAST_STATE);
  Serial.print("LAST_COMMAND=");
  Serial.println(LAST_COMMAND);
  

// Override the System ButtonConfig with our own ButtonConfig
// instance instead.
// Configure the ButtonConfig with the event handler and enable LongPress.
// SupressAfterLongPress is optional since we don't do anything if we get it.
//---------------------------------------------------------------------------------

//_buttonConfig.setEventHandler(handleEvent);
ButtonConfig* buttonConfig = button.getButtonConfig();

buttonConfig->setEventHandler(handleTestEvent);

buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
buttonConfig->setFeature(
      ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);

led.begin(LED_BUILTIN).blink(1500,40);
led.trigger(led.EVT_BLINK);

ONLINE_START = millis();
}


/********  MAIN LOOP ********/
void loop() {

 // if (!client.connected()) {
 //   reconnect();
 // }
 // client.loop();
  button.check();
  //OnSwitchChanged();
  OnStateChanged();
  //digitalWrite(D5,LOW);-removed, not sure why it was here
  
    
  long now = millis();
  
  if (now - RELAY_FIRED > INTERVAL_DURATION) {
      digitalWrite(D5,HIGH);
      digitalWrite(D6,HIGH);
  }
 
  //snprintf (msg, 75, "door state=%ld", DOOR_STATE);
   automaton.run();
}

//-------------------------------------------------------------------------------------------------------------------
// Events
//-------------------------------------------------------------------------------------------------------------------

void OnSwitchChanged()
{
  if((DOOR_STATE==0 || DOOR_STATE==2) && digitalRead(D0)==HIGH){
    DOOR_STATE=1;
    LAST_COMMAND=1;
    EEPROM.write(1,LAST_COMMAND);
    EEPROM.commit();
    Serial.println("SWITCH_OPEN");
    //led.begin(LED_BUILTIN).blink(500,200);
    //led.trigger(led.EVT_BLINK);
  }
  
  if(DOOR_STATE==1 && digitalRead(D0)==LOW){
    DOOR_STATE=2;
    LAST_COMMAND=2;
    Serial.println("SWITCH_CLOSE");
    EEPROM.write(1,LAST_COMMAND);
    EEPROM.commit();
   // led.begin(LED_BUILTIN).blink(1000,20);
    //led.trigger(led.EVT_BLINK);
  }
  
  
}

void handleEvent(AceButton*  button , uint8_t eventType,
    uint8_t  buttonState ) {

  Serial.println(eventType);
  Serial.println(buttonState);
  
  switch (eventType) {
   
    //case AceButton::kEventClicked:
    //case AceButton::kEventReleased:
    case AceButton::kEventLongPressed:{
      if(DOOR_STATE==1){
        DOOR_STATE=2;
        LAST_STATE=1;     
        Serial.print("Door State=");
        Serial.println(DOOR_STATE);
      }else{
        DOOR_STATE=1;
        LAST_STATE=2;
        Serial.print("Door State=");
        Serial.println(DOOR_STATE);        
      }
     // break;
    }
 
     break;

  }
}


