#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::istringstream

const char *ssid = "Livebox-dev-";
const char *password = "79221FC83787F575CE711C1668";

SoftwareSerial Uart2(0, 2); // (rx, tx)

int a = 0;

//**************** Prototypes ****************
bool connectToWIFI(int tryConnect);
String getDataFromSTM();
bool checkData(String data);



//****************setup*****************
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600); // start serial port at 9600 bps
    Serial.println(" ---------= Program Start =--------- ");

    Uart2.begin(9600);
    delay(1000);

    while (!connectToWIFI(20)) {}       // connect to WIFI try 20 times
}

//****************loop*****************
void loop() {
    // put your main code here, to run repeatedly:

    if (Serial.available() > 0) {
    }


    String sensor = getDataFromSensor();
    
    if (strlen(sensor.c_str()) > 0){
        Serial.println(sensor);
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
    Serial.println("Connected");
    return true;
}

String getDataFromSTM() {
    String dataFromSTM = "";

    while(Uart2.available() > 0) {
        dataFromSTM += (char)Uart2.read(); //read() read from RX_PIN  character by character
    }

    return dataFromSTM;
}

bool checkData(String data) {
    typedef enum {
        LABEL,
        VALUE,
        SUM
    } parser_t;

    parser_t parser = LABEL;

    String label, value, sum;

    for (unsigned int i = 0; i < data.length(); i++){
        if (data[i] == 0x0a){
            parser = LABEL;

        } else if (data[i] == 0x0d){
            /* code */
        } else if (data[i] == 0x09){
            switch (parser)
            {
            case LABEL:
                parser = VALUE;
            break;
            case VALUE:
                parser = SUM;
            break;
            default:
            break;
            }

        } else {
            
            switch (parser)
            {
            case LABEL:
                label += data[i];

                break;
            case VALUE:
                value += data[i];
                break;

            case SUM:
                sum +=  data[i];
                break;
            default:
            break;
            }
        }
    }

    String toSum = label + "\t" + value + "\t";

    uint8_t crc = 0xff;
    size_t i, j;
    for (i = 0; i < toSum.length(); i++) {
        crc ^= toSum[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }
    
    char chr[1];
    sprintf(chr, "%c", crc);

    if (sum == chr){
        return true;
    }
    
    return false;
}