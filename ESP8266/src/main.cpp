#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::istringstream

#include "crc32.hpp"

const char *ssid = "Livebox-dev-";
const char *password = "79221FC83787F575CE711C1668";

SoftwareSerial Uart2(0, 2); // (rx, tx)

String dataFromSTM = "";

int a = 0;

//**************** Prototypes ****************
bool connectToWIFI(int tryConnect, bool debug);
String getDataFromSTM();
bool checkCRC(String sensor);
String getData(String sensor);



//****************setup*****************
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600); // start serial port at 9600 bps
    Serial.println(" ---------= Program Start =--------- ");

    Uart2.begin(9600);

    delay(1000);

    //while (!connectToWIFI(20, true)) {}       // connect to WIFI try 20 times
}

//****************loop*****************
void loop() {
    // put your main code here, to run repeatedly:

    if (Serial.available() > 0) {   
    }
    String sensor = getDataFromSTM();

    if (sensor.length() > 0){
        Serial.println(sensor);


        if (checkCRC(sensor)) {
            String data = getData(sensor);
            Serial.print("sensor : ");
            Serial.println(data);

            StaticJsonDocument<128> json;

            deserializeJson(json, data);

            String tof = json["ToF"];

            if (tof == "triggered") {
                Serial.println(tof);
            }
        }
        
    }
    
    delay(100);

/*
    String dataFromSensor = Uart2.readString();
    Serial.println(dataFromSensor);
    delay(100);
*/
}

//****************Functions*****************

//***Function for connect to internet 
bool connectToWIFI(int tryConnect, bool debug) {
    if (debug){
        Serial.print("Waiting for connection to WiFi to : ");
        Serial.println(ssid);
    }

    WiFi.begin(ssid, password);   //WiFi connection
    
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFi connection completion
        if (i <= tryConnect) {
            delay(500);
            
            if (debug) {
                Serial.print('.');
            }
            
            i++;
        } else {  //delay passed
            if (debug) {
                Serial.println("Delay passed");
            }
            
            return false;
        }
    }
    Serial.println("Connected");
    return true;
}

String getDataFromSTM() {
    char c_uart2 = '\0';

    //Serial.println("getData");

    while(Uart2.available() > 0) {
        c_uart2 = (char)Uart2.read();
        dataFromSTM += c_uart2; //read() read from RX_PIN  character by character
    }

    if (c_uart2 == '\r') {
        c_uart2 = '\0';

        String data = dataFromSTM;
        dataFromSTM = "";

        return data;
    } else {
        return "";
    }
}

bool checkCRC(String sensor) {

    StaticJsonDocument<128> json;
    deserializeJson(json, sensor);

    const char* c_crc = json["crc"];
    uint32_t uint_crc = (int)strtol(c_crc, NULL, 16);

    String data;
    serializeJson(json["data"][0], data);
    char * c_data = (char *)data.c_str();
    uint32_t crc = StringToCRC32(c_data, CRC32mpeg2);

    if (crc == uint_crc) {
        return true;
    }

    return false;
}