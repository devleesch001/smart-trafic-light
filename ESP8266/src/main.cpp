#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

const char *ssid = "Livebox-dev-";
const char *password = "79221FC83787F575CE711C1668";

int a = 0;

//****************Functions*****************

//***Function for connect to internet 
// try during tpssec seconds
bool connectToWIFI(int tryConnect) {
    
    Serial.print("Waiting for connection to WiFi to : ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);   //WiFi connection
    
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFi connection completion
        if (i <= tryConnect) {
            delay(500);
            Serial.print('.');
            i++;
        } else {  //delay passed
            Serial.println("Delay passed");
            return false;
        }
    }
    Serial.println("OK");
    return true;
}

//***Function for recover data.. but as we can't plug elements on our board, this will be fake data:
// temp is random, mac is write in the function, always the same, et serial number is in the parameter
//The function update the global array variable data


//***Function wich send data to the server

//****************setup*****************
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);                   // start serial port at 9600 bps
    delay(1000);
    Serial.println("---------- Program Start ----------");
    while (!connectToWIFI(20)) {}       // connect to WIFI

    
}

//****************loop*****************
void loop() {
    // put your main code here, to run repeatedly:

    if (Serial.available() > 0) {
        // read the incoming byte:
        String data = Serial.readString();
        Serial.println(data);
    }
}