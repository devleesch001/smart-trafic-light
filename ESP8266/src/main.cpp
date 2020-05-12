#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

const char *ssid = "Livebox-dev-";
const char *password = "79221FC83787F575CE711C1668";

SoftwareSerial Uart2(0, 2);

int a = 0;

//**************** Prototypes ****************
bool connectToWIFI(int tryConnect);
String getData(const char * data);


//****************setup*****************
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600); // start serial port at 9600 bps
    Serial.println("---------- Program Start ----------");

    Uart2.begin(9600);
    delay(1000);

    while (!connectToWIFI(20)) {}       // connect to WIFI try 20
}

//****************loop*****************
void loop() {
    // put your main code here, to run repeatedly:

    if (Serial.available() > 0) {
        // read the incoming byte:
        Serial.println(getDataFromSensor());
    }

    
}

//****************Functions*****************

//***Function for connect to internet 
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

String getDataFromSensor() {
    String dataFromSensor = "";
    
    while(Uart2.available() > 0) {
        dataFromSensor = Uart2.readString(); //read() read from RX_PIN  character by character 
    }
    Serial.println(dataFromSensor);

    return dataFromSensor;
}