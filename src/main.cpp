#include "WiFi.h"
#include "PubSubClient.h"
#include "Wire.h"
#include "DHT.h"
#include "Arduino.h"
#include "ArduinoJson.h"

int AO_PIN = 34;
int DHT_PIN = 17;
int lamps_pin[4] = {15, 2, 4, 16};

JsonDocument jsonDocument;
char buffer[250];




char ssid[] = "DIRECT-19876533";
char password[] = "tinggalmasukaja";

const char* mqtt_server = "mqtt.rekycode.id";  // e.g., "192.168.1.10" or domain like "broker.example.com"
const int mqtt_port = 1883;                        // Default MQTT port is 1883
const char* mqtt_user = "reky";      // Leave empty if not using authentication
const char* mqtt_password = "reky.iot";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;



DHT dht(DHT_PIN, DHT22);


void turnOffLamp(int pin) {
    digitalWrite(pin, LOW);
}

void turnOnLamp(int pin) {
    digitalWrite(pin, HIGH);
}

int getLumen() {
    int lumen = analogRead(AO_PIN);
    return lumen;
}

void turnOffAllLamp() {
    for (int i : lamps_pin) {
        digitalWrite(i, LOW);
    }
}

void turnOnAllLamp() {
    for (int i : lamps_pin) {
        digitalWrite(i, HIGH);
    }
}

float getHumidity(){
    float humidity = dht.readHumidity();
    return humidity;
}

float getTemperature(){
    float temperature = dht.readTemperature(false);
    return temperature;
}
//void lampGetState() {
//    int id_lamp = server.pathArg(0).toInt();
//    int state = digitalRead(lamps_pin[id_lamp - 1]);
//    if (state == HIGH) {
//        set_message("lamp", false, "ON");
//    } else {
//        set_message("lamp", false, "OFF");
//    }
//    server.send(200, "application/json", buffer);
//}

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

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

//void callback(char* topic, byte* message, unsigned int length) {
//    Serial.print("Message arrived on topic: ");
//    Serial.print(topic);
//    Serial.print(". Message: ");
//    char messageTemp[50];
//
//    for (int i = 0; i < length; i++) {
//        Serial.print((char)message[i]);
//        messageTemp += (char)message[i];
//    }
//    Serial.println();
//
//    if (String(topic) == "esp32/setLampState") {
//        Serial.print("Changing output to ");
//        if(messageTemp == "on"){
//            Serial.println("on");
//            turnOnAllLamp();
//        }
//        else if(messageTemp == "off"){
//            Serial.println("off");
//            turnOffAllLamp();
//        }
//    }
//}

void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");

    // Create a buffer to store the message
    char messageTemp[256];
    int i;
    for (i = 0; i < length; i++) {
        messageTemp[i] = (char)message[i];
    }
    messageTemp[i] = '\0'; // Null-terminate the string

    Serial.println(messageTemp);

    // Parse the JSON message
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, messageTemp);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract the "action" key value
    const char* action = doc["action"];

    if(String(topic) == "esp32/setLampState"){
        if (strcmp(action, "on") == 0) {
            turnOnAllLamp();
        }else if (strcmp(action, "off") == 0){
            turnOffAllLamp();
        }
    }

}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("connected");
            // Subscribe
            client.subscribe("esp32/#");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
void setup() {
    Serial.begin(9600);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    for (int i : lamps_pin) {
        pinMode(i, OUTPUT);
    }
    dht.begin();
}

void loop(){
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 5000){
        lastMsg = now;

        int lumenValue = getLumen();
        float humidityValue = dht.readHumidity();
        float temperatureValue = dht.readTemperature();

        char humidityString[8];
        dtostrf(humidityValue, 1, 2, humidityString);
        client.publish("esp32/humidity", humidityString);

        char temperatureString[8];
        dtostrf(temperatureValue, 1, 2, temperatureString);
        client.publish("esp32/temperature", temperatureString);

        char lumenString[8];
        itoa(lumenValue, lumenString, 10);
        Serial.print("Lumen = ");
        Serial.println(getLumen());
        client.publish("esp32/lumen", lumenString);

    }
}
