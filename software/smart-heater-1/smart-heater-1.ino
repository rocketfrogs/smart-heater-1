/************************* WiFi Access Point *********************************/

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include "credentials.h"  "     // Include Credentials (you need to create that file if you cloned it from git)

/*
Content of "credentials.h" that matters for this section

// WIFI Credentials

#define WIFI_SSID        "[REPLACE BY YOUR WIFI SSID (2G)]"     // The SSID (name) of the Wi-Fi network you want to connect to
#define WIFI_PASSWORD    "[REPLACE BY YOUR WIFI PASSWORD]"      // The password of the Wi-Fi 
*/

const char* ssid     = WIFI_SSID;         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = WIFI_PASSWORD;     // The password of the Wi-Fi 

/************************* MQTT Setup *********************************/

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "credentials.h

/*
// MQTT Credentials

Content of "credentials.h" that matters for this section

#define AIO_SERVER      "[REPLACE BY YOUR MQTT SERVER IP ADDRESS OR ITS FQDN]"
#define AIO_SERVERPORT  [REPLACE BY THE PORT NUMBER USED FOR THE MQTT SERVICE ON YOUR MQTT SERVEUR (DEFAULT IS 1883)]       // use 8883 for SSL"
#define AIO_USERNAME    ""  // USE THIS IF YOU HAVE USERNAME AND PASSWORD ENABLED ON YOUR MQTT SERVER
#define AIO_KEY         ""  // USE THIS IF YOU HAVE USERNAME AND PASSWORD ENABLED ON YOUR MQTT SERVER
*/

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish stat_current_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/stat/heater_1/current_temperature");
Adafruit_MQTT_Publish stat_target_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/stat/heater_1/target_temperature");
Adafruit_MQTT_Publish stat_heating = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/stat/heater_1/heating");
Adafruit_MQTT_Publish stat_status = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/stat/heater_1/status");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe cmnd_target_temperature = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/cmnd/heater_1/target_temperature");
Adafruit_MQTT_Subscribe cmnd_status = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/cmnd/heater_1/status");

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

#define TMPPin A0//TMP36 attached to ESP8266 ESP-12E ADC

String temperatureString = "";      //variable to hold the temperature reading
int BUTTONc = 0;         // variable for reading the pushbutton status
int BUTTONh = 0;         // variable for reading the pushbutton status
int BUTTONonoff = 0;         // variable for reading the pushbutton status

int onoff_status = LOW;
int previous_BUTTONonoff_status = LOW;

float temperature_target = 25.00;
int previous_BUTTONc_status = LOW;
int previous_BUTTONh_status = LOW;

int flag = 0;

void setup() {
Serial.begin(115200); // Start the Serial communication to send messages to the computer

  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&cmnd_target_temperature);
  mqtt.subscribe(&cmnd_status);

}


void loop() {

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here


  int tmpValue = analogRead(TMPPin);
  float voltage = tmpValue * 3.3;// converting that reading to voltage
  voltage /= 1024.0;
  float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
  //to degrees ((voltage - 500mV) times 100)
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;  //now convert to Fahrenheit
  temperatureString = " " + String(temperatureC) + " C  " + String(temperatureF) + " F ";
 // Serial.println(temperatureString);

pinMode(D3, OUTPUT); 
pinMode(D4, OUTPUT); 
pinMode(D5, OUTPUT); 
pinMode(D6, OUTPUT); 
pinMode(D7, OUTPUT); 

pinMode(D0, INPUT); 

  BUTTONc = digitalRead(D0);
  BUTTONh = digitalRead(D1);
  BUTTONonoff = digitalRead(D2);

    if (onoff_status == HIGH) { 
    // turn LED on:
    digitalWrite(D4, HIGH);
  } else {
    // turn LED off:
    digitalWrite(D4, LOW);
  }

    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (BUTTONh == HIGH) {
    // turn LED on:
    digitalWrite(D5, HIGH);
  } else {
    // turn LED off:
    digitalWrite(D5, LOW);
  }

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (BUTTONc == HIGH) {
    // turn LED on:
    digitalWrite(D7, HIGH);
  } else {
    // turn LED off:
    digitalWrite(D7, LOW);
  }

////////////// COLDER BUTTON PROCCESSING ///////////////
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (BUTTONc == HIGH) {
      ////Serial.println("BUTTONonoff == HIGH");
   
      if (previous_BUTTONc_status == LOW) {
          ////Serial.println("previous_BUTTONonoff_status == LOW");
      
          temperature_target -= 0.5;
          Serial.print("temperature_target : ");
          Serial.println(temperature_target);
          previous_BUTTONc_status = HIGH; // this so that it change state once despite the button behing pressed during multiple loops
      
      } // else it was high before and we don't do anythink this time 

  } else {

  // Serial.println("BUTTONonoff == NOT HIGH");
  previous_BUTTONc_status = LOW; // this so that it change state once despite the button behing pressed during multiple l
    
  }
  ////////////// END COLDER BUTTON PROCCESSING ///////////////

////////////// HOTTER BUTTON PROCCESSING ///////////////
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (BUTTONh == HIGH) {
      ////Serial.println("BUTTONonoff == HIGH");
   
      if (previous_BUTTONh_status == LOW) {
          ////Serial.println("previous_BUTTONonoff_status == LOW");
      
          temperature_target += 0.5;
          Serial.print("temperature_target : ");
          Serial.println(temperature_target);
          previous_BUTTONh_status = HIGH; // this so that it change state once despite the button behing pressed during multiple loops
      
      } // else it was high before and we don't do anythink this time 

  } else {

  // Serial.println("BUTTONonoff == NOT HIGH");
  previous_BUTTONh_status = LOW; // this so that it change state once despite the button behing pressed during multiple l
    
  }
  ////////////// END HOTTER BUTTON PROCCESSING ///////////////


  ////////////// ON OFF BUTTON PROCCESSING ///////////////
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (BUTTONonoff == HIGH) {
      ////Serial.println("BUTTONonoff == HIGH");
   
      if (previous_BUTTONonoff_status == LOW) {
          ////Serial.println("previous_BUTTONonoff_status == LOW");
      
          if (onoff_status == LOW) {
               Serial.println("Device set to ON");
               onoff_status = HIGH;          
          } else {
              Serial.println("Device set to OFF");
              onoff_status = LOW;
          }
          previous_BUTTONonoff_status = HIGH; // this so that it change state once despite the button behing pressed during multiple loops
      
      } // else it was high before and we don't do anythink this time 

  } else {

  // Serial.println("BUTTONonoff == NOT HIGH");
  previous_BUTTONonoff_status = LOW; // this so that it change state once despite the button behing pressed during multiple l
    
  }
  ////////////// END ON OFF BUTTON PROCCESSING ///////////////


////////// THERMOSTAT //////////

if (onoff_status == HIGH) {

    if (flag == 0){

        Serial.print("current temperature : ");
        Serial.println(temperatureC);
        flag ++;
      
    } else {

        flag ++;
        if(flag == 20) {
      
            flag = 0;
        }
      
    }

    if (temperatureC < temperature_target) {

         digitalWrite(D3, HIGH);
         digitalWrite(D6, LOW);
      
    } else {

         digitalWrite(D3, LOW);
         digitalWrite(D6, HIGH);
      
    }
  
} else {

    digitalWrite(D6, LOW);
    digitalWrite(D3, LOW); // we turn off the heating
  
}




/*
digitalWrite(D3, HIGH);
//  digitalWrite(D4, HIGH);
//  digitalWrite(D5, HIGH);
digitalWrite(D6, HIGH);
//  digitalWrite(D7, HIGH);


  delay(1000);
  

digitalWrite(D3, LOW);
//  digitalWrite(D4, LOW);
//  digitalWrite(D5, LOW);
digitalWrite(D6, LOW);
//  digitalWrite(D7, LOW);

  
  delay(1000);
*/

  delay(50);


  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(250))) {
    
    if (subscription == &cmnd_target_temperature) {
      Serial.print(F("Got: "));
      Serial.println((char *)cmnd_target_temperature.lastread);
      int temp_temperature_target = atof((char *)cmnd_target_temperature.lastread);
      if (temp_temperature_target >= 0 && temp_temperature_target < 40) {
      temperature_target = temp_temperature_target;
      Serial.print("Received New temperature Target : ");
      Serial.println(temperature_target);
      } else {
      Serial.print("Received UNVALID New temperature Target : ");
      Serial.println(temp_temperature_target);
      }
    }
    
    if (subscription == &cmnd_status) {
      Serial.print(F("Got: "));
      Serial.println((char *)cmnd_status.lastread);
      int temp_onoff_status = atof((char *)cmnd_status.lastread);
      if (temp_onoff_status == 0 || temp_onoff_status == 1) {
      onoff_status = temp_onoff_status;
      Serial.print("Received New onoff status : ");
      Serial.println(onoff_status);
      } else {
      Serial.print("Received UNVALID New onoff status : ");
      Serial.println(temp_onoff_status);
      }
      
    }

 }
    
  

  // Now we can publish stuff!
  Serial.print(F("\nSending stat_current_temperature val "));
  Serial.print(temperatureC);
  Serial.print("...");
  if (! stat_current_temperature.publish(temperatureC)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  Serial.print(F("\nSending stat_target_temperature val "));
  Serial.print(temperature_target);
  Serial.print("...");
  if (! stat_target_temperature.publish(temperature_target)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  Serial.print(F("\nSending stat_status val "));
  Serial.print(onoff_status);
  Serial.print("...");
  if (! stat_status.publish(onoff_status)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
int heating = digitalRead(D3);

  Serial.print(F("\nSending stat_heating val "));
  Serial.print(heating);
  Serial.print("...");
  if (! stat_heating.publish(heating)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }



// stat_target_temperature
// stat_status

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */


}





// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(250);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
