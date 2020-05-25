#include <Arduino.h>
#include <ArduinoJson.h>

#include <SoftwareSerial.h>

#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <time.h>

#include "crc32.h"
#include "keys.h"

extern "C" {
#include "libb64/cdecode.h"
}


//**************** Prototypes ****************
bool connectToWIFI(int tryConnect, bool debug);
String getDataFromSTM();
bool checkCRC(String sensor, bool debug);
String getData(String sensor);

bool connectToAws();
void callback(char* topic, byte* payload, unsigned int length);
int b64decode(String b64Text, uint8_t* output);
void setCurrentTime();
void sendDatatoAws(String jsonData);

// Private Variable
WiFiClientSecure wiFiClient;
PubSubClient pubSubClient(AWS_IOT_ENDPOINT, 8883, callback, wiFiClient); //MQTT Client

StaticJsonDocument<512> json;
SoftwareSerial Uart2(0, 2); // (rx, tx)

String dataFromSTM = "";
int a = 0;


//****************setup*****************
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600); // start serial port at 9600 bps
    Serial.println(" ---------= Program Start =--------- ");

    Uart2.begin(9600);

    delay(1000);

    while (!connectToWIFI(20, true)) {}       // connect to WIFI try 20 times

    setCurrentTime();

    uint8_t binaryCert[AWS_CERT_CRT.length() * 3 / 4];
    int len = b64decode(AWS_CERT_CRT, binaryCert);
    wiFiClient.setCertificate(binaryCert, len);

    uint8_t binaryPrivate[AWS_CERT_PRIVATE.length() * 3 / 4];
    len = b64decode(AWS_CERT_PRIVATE, binaryPrivate);
    wiFiClient.setPrivateKey(binaryPrivate, len);

    uint8_t binaryCA[AWS_CERT_CA.length() * 3 / 4];
    len = b64decode(AWS_CERT_CA, binaryCA);
    wiFiClient.setCACert(binaryCA, len);
}

//****************loop*****************
void loop() {
    // put your main code here, to run repeatedly:

    if (WiFi.status() == WL_CONNECTED){
        connectToAws();
    }

    //Serial.println(time(nullptr));
    //Serial.println(ctime(&now));
    

    String sensor = getDataFromSTM();

    if (sensor.length() > 0){
        Serial.println(sensor);

        if (checkCRC(sensor, false)) {

            String data = getData(sensor);

            sendDatatoAws(data);

/*
            deserializeJson(json, data);
                json["Date"] = time(nullptr);

                //time_t now = time(nullptr);

                data = "";
            serializeJson(json, data);

            Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);


            Serial.println(data);

            if (!pubSubClient.publish(MQTT_PUB_TOPIC, data.c_str(), false))// send json data to dynamoDbB topic
            Serial.println("ERROR??? :"); Serial.println(pubSubClient.state()); //Connected '0'
            */
        }
        
    }
    
    delay(100);
}

//****************Functions*****************

//***Function for connect to internet 
bool connectToWIFI(int tryConnect, bool debug) {

    Serial.print("Waiting for connection to WiFi to : "); Serial.print(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //WiFi connection

    if (debug){
        int i = 0;
        while (WiFi.status() != WL_CONNECTED) {
        
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

    }

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

void sendDatatoAws(String jsonData)
{ 
    //const char * data = "{\"posix\":856,\"phase\":\"green\",\"ToF\":\"triggered\"}";

    deserializeJson(json, jsonData);
    json["Date"] = time(nullptr);

    //time_t now = time(nullptr);

    String data;
    serializeJson(json, data);

    Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);


    Serial.println(data);

    if (!pubSubClient.publish(MQTT_PUB_TOPIC, data.c_str(), false))// send json data to dynamoDbB topic
    Serial.println("ERROR??? :"); Serial.println(pubSubClient.state()); //Connected '0'

}

bool checkCRC(String sensor, bool debug) {
    String data;

    deserializeJson(json, sensor);
        String CRC_Json = json["crc"];

    serializeJson(json["data"], data);

    char * c_data = (char *)data.c_str();
    uint32_t crc = StringToCRC32(c_data, CRC32mpeg2);

    String CRC_Calc = String(crc, HEX); 

    if (debug) {
        Serial.print("crc calculated : "); Serial.println(CRC_Calc);
        Serial.print("crc read from json : "); Serial.println(CRC_Json);
    }


    if (CRC_Calc == CRC_Json) {
        if (debug) {
            Serial.println("Success is : CRC_Calc == CRC_Json");
        }

        return true;
    }

    if (debug) {
        Serial.println("Fail is : CRC_Calc != CRC_Json");
    }

    return false;
}

String getData(String sensor) {
    deserializeJson(json, sensor);    
    String data;
    serializeJson(json["data"], data);

    return data;
}

bool connectToAws() {
    if (!pubSubClient.connected()) {
        Serial.print("PubSubClient connecting to : "); Serial.print(AWS_IOT_ENDPOINT);
        while (!pubSubClient.connected()) {
            Serial.print(pubSubClient.state());
            delay(100);
            Serial.print(".");
            pubSubClient.connect("smart-trafic-light");
        }

        Serial.println(" connected");
        pubSubClient.subscribe(MQTT_PUB_TOPIC);
    }

    pubSubClient.loop();
    return true;
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

int b64decode(String b64Text, uint8_t* output) {
    base64_decodestate b64ds;
    base64_init_decodestate(&b64ds);
    int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(), (char*)output, &b64ds);
    return cnt;
}

void setCurrentTime() {
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: "); Serial.print(asctime(&timeinfo));
}
