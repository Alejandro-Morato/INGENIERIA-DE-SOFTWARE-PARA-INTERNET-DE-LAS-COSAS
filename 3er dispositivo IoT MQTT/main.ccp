/******************************************
 *
 * ESP32 + DHT22 + Ubidots MQTT
 * Basado en el ejemplo oficial de Ubidots
 *
 ******************************************/

#include "UbidotsEsp32Mqtt.h"
#include "DHT.h"

/****************************************
 * Definir constantes
 ****************************************/
const char *UBIDOTS_TOKEN = "BBUS-04d234U72bW00Wg3AXulz9z30xzDVi"; // Token de Ubidots
const char *WIFI_SSID = "GalaxyS24Ultra";                           // SSID WiFi
const char *WIFI_PASS = "123456789";                            // Password WiFi
const char *DEVICE_LABEL = "publicador";                           // Dispositivo en Ubidots

const char *VARIABLE_LABEL_TEMP = "temperatura";                   // Variable 1
const char *VARIABLE_LABEL_HUM = "humedad";                        // Variable 2

const int PUBLISH_FREQUENCY = 5000;                                // Cada 5 segundos

#define DHTPIN 4
#define DHTTYPE DHT22

unsigned long timer;
Ubidots ubidots(UBIDOTS_TOKEN);
DHT dht(DHTPIN, DHTTYPE);

/****************************************
 * Funciones auxiliares
 ****************************************/
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

/****************************************
 * Setup
 ****************************************/
void setup() {
  Serial.begin(115200);

  dht.begin();

  ubidots.setDebug(true);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();
}

/****************************************
 * Loop principal
 ****************************************/
void loop() {
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  if ((millis() - timer) > PUBLISH_FREQUENCY) {
    float temperatura = dht.readTemperature();
    float humedad = dht.readHumidity();

    if (isnan(temperatura) || isnan(humedad)) {
      Serial.println("Error al leer el sensor DHT22");
    } else {
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println(" °C");

      Serial.print("Humedad: ");
      Serial.print(humedad);
      Serial.println(" %");

      ubidots.add(VARIABLE_LABEL_TEMP, temperatura);
      ubidots.add(VARIABLE_LABEL_HUM, humedad);
      ubidots.publish(DEVICE_LABEL);
    }

    timer = millis();
  }

  ubidots.loop();
}