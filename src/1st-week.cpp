#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <ArduinoJson.h>
#include "DHT.h"

int AO_PIN = 34;
int DHT_PIN = 17;
int lamps_pin[4] = {15, 2, 4, 16};
char ssid[] = "SSID";
char password[] = "PASSWORD";

DHT dht(DHT_PIN, DHT22);
WebServer server(80);
JsonDocument jsonDocument;
char buffer[250];

void get_sensor_json(char *tag, int value, char *unit) {
    jsonDocument.clear();
    jsonDocument["type"] = tag;
    jsonDocument["value"] = value;
    jsonDocument["unit"] = unit;
    serializeJson(jsonDocument, buffer);
}

void set_message(char *tag, bool error, char *message) {
    jsonDocument.clear();
    jsonDocument["type"] = tag;
    jsonDocument["error"] = error;
    jsonDocument["message"] = message;
    serializeJson(jsonDocument, buffer);
}

void turnOffLamp(int pin) {
    digitalWrite(pin, LOW);
}

void turnOnLamp(int pin) {
    digitalWrite(pin, HIGH);
}

void getLumen() {
    int lumen = analogRead(AO_PIN);
    get_sensor_json("lumen", lumen, "lumen");
    server.send(200, "application/json", buffer);
}

void getHumidity() {
    float humidity = dht.readHumidity();
    get_sensor_json("humidity", humidity, "%");
    server.send(200, "application/json", buffer);
}

void getTemperature() {
    float temperature = dht.readTemperature();
    get_sensor_json("temperature", temperature, "C");
    server.send(200, "application/json", buffer);
}

void lampHandler() {
    int id_lamp = server.pathArg(0).toInt();
    int state = server.pathArg(1).toInt();

    int lamps_id[4] = {1, 2, 3, 4};
    if (std::find(std::begin(lamps_id), std::end(lamps_id), id_lamp) != std::end(lamps_id)) {
        if (state == 0) {
            turnOffLamp(lamps_pin[id_lamp - 1]);
            set_message("lamp", false, "Lamp is off");
            server.send(200, "application/json", buffer);
        } else if (state == 1) {
            turnOnLamp(lamps_pin[id_lamp - 1]);
            set_message("lamp", false, "Lamp is on");
            server.send(200, "application/json", buffer);
        } else {
            set_message("lamp", true, "Invalid state");
            server.send(400, "application/json", buffer);
        }
    } else {
        set_message("lamp", true, "Invalid lamp id");
        server.send(400, "application/json", buffer);
    }
}

void turnOffAllLamp() {
    for (int i : lamps_pin) {
        digitalWrite(i, LOW);
    }
    set_message("lamp", false, "All lamp is off");
    server.send(200, "application/json", buffer);
}

void turnOnAllLamp() {
    for (int i : lamps_pin) {
        digitalWrite(i, HIGH);
    }

    set_message("lamp", false, "All lamp is on");
    server.send(200, "application/json", buffer);
}

void strobeLamp(){
    for (int strobe_time = 0; strobe_time < 6; strobe_time++) {
        for (int i : lamps_pin) {
            digitalWrite(i, HIGH);
            delay(500);
            digitalWrite(i, LOW);
            delay(500);
        }
    }
}

void lampGetState() {
    int id_lamp = server.pathArg(0).toInt();
    int state = digitalRead(lamps_pin[id_lamp - 1]);
    if (state == HIGH) {
        set_message("lamp", false, "ON");
    } else {
        set_message("lamp", false, "OFF");
    }
    server.send(200, "application/json", buffer);
}

void router(){
    server.on("/getLumen", getLumen);
    server.on("/getHumidity", getHumidity);
    server.on("/getTemperature", getTemperature);
    server.on(UriBraces("/lamp/{}/{}"), lampHandler);
    server.on(UriBraces("/lamp_state/{}"), lampGetState);
    server.on("/lamp/turnOnAll", turnOnAllLamp);
    server.on("/lamp/turnOffAll", turnOffAllLamp);
    server.on("/lamp/strobe", strobeLamp);

    server.begin();
}

void setup() {
    Serial.begin(9600);
    dht.begin();

    pinMode(AO_PIN, INPUT);
    for (int i : lamps_pin) {
        pinMode(i, OUTPUT);
    }

    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    router();

    Serial.println("Connected to the WiFi network");
    Serial.print("API URL: http://");
    Serial.println(WiFi.localIP());
}

void loop() {
    server.handleClient();
}