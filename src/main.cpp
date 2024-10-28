#include <Arduino.h>
#include <WiFi.h>
#include "DHT.h"
#include "Firebase_ESP_Client.h"
#include "addons/TokenHelper.h"
#define DATABASE_URL "https://iot-2024-16551-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyAhWDXMuBHeiBTR6TbElGQHncMZRQ9UdY4"
#define PATIENT1_LED_PIN 15
#define PATIENT2_LED_PIN 2

int DHT_PIN = 17;
char ssid[] = "DIRECT-19879533";
char password[] = "tinggalmasukaja";
DHT dht(DHT_PIN, DHT22);
int lamps_pin[4] = {15, 2, 4, 16};

FirebaseData firebaseData;
FirebaseData ledData;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void sensorupdate(){
    int h = dht.readHumidity();
    int t = dht.readTemperature();
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("% Temperature: "));
    Serial.print(t);
    Serial.print(F("°C ,"));
    if (Firebase.RTDB.setInt(&firebaseData, "iot-db/suhu", t))
        {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
        Serial.println("ETag: " + firebaseData.ETag());
        Serial.println("------------------------------------");
        Serial.println();
        }
    else {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
    }
    if (Firebase.RTDB.setInt(&firebaseData, "iot-db/kelembapan", h))
        {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
        Serial.println("ETag: " + firebaseData.ETag());
        Serial.println("------------------------------------");
        Serial.println();
        }
    else {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
    }

}

void blinkLed(int ledPin, int times, int interval) {

    for (int i = 0; i < times; i++) {

        digitalWrite(ledPin, HIGH);

        delay(interval);

        digitalWrite(ledPin, LOW);

        delay(interval);

    }

}

void setup(){
    Serial.begin(9600);
    dht.begin();
    for (int i : lamps_pin) pinMode(i, OUTPUT);
    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the WiFi network");
    Serial.print("API URL: http://");
    Serial.println(WiFi.localIP());
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
        Serial.println("ok");
        signupOK = true;
    }
    else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}
void loop(){
    if (Firebase.ready()) {

        Firebase.RTDB.getInt(&firebaseData, "patient-nurse-db/patientsatu");

        int patient1Count = firebaseData.intData();


        Firebase.RTDB.getInt(&firebaseData, "patient-nurse-db/patientdua");

        int patient2Count = firebaseData.intData();
        if (patient1Count > 0) {

            blinkLed(PATIENT1_LED_PIN, 5, 500);

        }


        if (patient2Count > 0) {

            blinkLed(PATIENT2_LED_PIN, 5, 500);

        }

    }

}
