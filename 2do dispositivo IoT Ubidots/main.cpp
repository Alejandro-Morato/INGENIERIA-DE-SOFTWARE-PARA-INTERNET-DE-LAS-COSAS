#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include "Ubidots.h"

// ==================================================
// CREDENCIALES
// ==================================================

const char* UBIDOTS_TOKEN = "BBUS-04d234U72bW00Wg3AXulz9z30xzDVi";
const char* WIFI_SSID = "UAM-ROBOTICA";
const char* WIFI_PASS = "m4nt32024uat";

// ==================================================
// CONFIGURACION DE UBIDOTS
// ==================================================

// La guia oficial utiliza un puntero Ubidots*
// y ubidots->send() sin argumentos.
Ubidots* ubidots = nullptr;

// ==================================================
// PINES
// ==================================================

// DHT22
#define DHT_PIN 23
#define DHT_TYPE DHT22

// BMP280 I2C
// GPIO34 es solamente de entrada; no se recomienda como SCL.
// Se conserva SDA en GPIO25 y se usa GPIO26 como SCL.
#define BMP_SDA 21
#define BMP_SCL 22

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp;

bool bmpDisponible = false;

// ==================================================
// TIEMPOS
// ==================================================

const unsigned long INTERVALO_ENVIO = 10000;
unsigned long ultimoEnvio = 0;

// ==================================================
// FUNCIONES AUXILIARES
// ==================================================

void imprimirEstadoWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi conectado.");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("Direccion IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println();
    Serial.println("No se pudo conectar a WiFi.");
  }
}

bool conectarWiFi() {
  Serial.print("Conectando a WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - inicio < 20000) {
    delay(500);
    Serial.print(".");
  }

  imprimirEstadoWiFi();

  return WiFi.status() == WL_CONNECTED;
}

// ==================================================
// SETUP
// ==================================================

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("ESP32 + DHT22 + BMP280 + Ubidots");
  Serial.println("=================================");

  // Iniciar DHT22
  dht.begin();
  Serial.println("DHT22 iniciado.");
  Serial.println("DATA: GPIO22");

  // Iniciar I2C
  Wire.begin(BMP_SDA, BMP_SCL);
  Serial.println("I2C iniciado.");
  Serial.println("SDA: GPIO25");
  Serial.println("SCL: GPIO26");

  // Buscar BMP280 en direccion 0x76
  bmpDisponible = bmp.begin(0x76);

  // Buscar BMP280 en direccion 0x77
  if (!bmpDisponible) {
    Serial.println("BMP280 no encontrado en 0x76.");
    Serial.println("Intentando direccion 0x77...");
    bmpDisponible = bmp.begin(0x77);
  }

  if (bmpDisponible) {
    Serial.println("BMP280 detectado correctamente.");

    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X16,
      Adafruit_BMP280::FILTER_X16,
      Adafruit_BMP280::STANDBY_MS_500
    );
  } else {
    Serial.println("No se detecto el BMP280.");
    Serial.println("El sistema continuara con el DHT22.");
  }

  // Conectar WiFi usando la funcion indicada por Ubidots
  if (conectarWiFi()) {
    Ubidots::wifiConnect(WIFI_SSID, WIFI_PASS);

    // Crear la instancia despues de conectarse al WiFi
    ubidots = new Ubidots(UBIDOTS_TOKEN, UBI_HTTP);

    // Activa mensajes internos de depuracion si los necesitas.
    // ubidots->setDebug(true);

    Serial.println("Ubidots inicializado.");
  } else {
    Serial.println("Ubidots no se inicializo porque no hay WiFi.");
  }

  Serial.println("Setup terminado.");
}

// ==================================================
// LOOP
// ==================================================

void loop() {
  if (millis() - ultimoEnvio < INTERVALO_ENVIO) {
    return;
  }

  ultimoEnvio = millis();

  // Reconectar si se pierde el WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado.");
    conectarWiFi();
    return;
  }

  // Verificar que Ubidots exista
  if (ubidots == nullptr) {
    Serial.println("Ubidots no esta inicializado.");
    return;
  }

  // ================================================
  // LECTURA DEL DHT22
  // ================================================

  float temperaturaDHT = dht.readTemperature();
  float humedadDHT = dht.readHumidity();

  if (isnan(temperaturaDHT) || isnan(humedadDHT)) {
    Serial.println("Error al leer el DHT22.");
    return;
  }

  // ================================================
  // LECTURA DEL BMP280
  // ================================================

  float temperaturaBMP = 0.0F;
  float presionBMP = 0.0F;
  float altitudBMP = 0.0F;

  if (bmpDisponible) {
    temperaturaBMP = bmp.readTemperature();
    presionBMP = bmp.readPressure() / 100.0F;
    altitudBMP = bmp.readAltitude(1013.25F);

    if (isnan(temperaturaBMP) || isnan(presionBMP)) {
      Serial.println("Error al leer el BMP280.");
      bmpDisponible = false;
    }
  }

  // ================================================
  // MOSTRAR LECTURAS
  // ================================================

  Serial.println();
  Serial.println("========== LECTURAS ==========");

  Serial.print("Temperatura DHT22: ");
  Serial.print(temperaturaDHT, 2);
  Serial.println(" °C");

  Serial.print("Humedad DHT22: ");
  Serial.print(humedadDHT, 2);
  Serial.println(" %");

  if (bmpDisponible) {
    Serial.print("Temperatura BMP280: ");
    Serial.print(temperaturaBMP, 2);
    Serial.println(" °C");

    Serial.print("Presion BMP280: ");
    Serial.print(presionBMP, 2);
    Serial.println(" hPa");

    Serial.print("Altitud aproximada: ");
    Serial.print(altitudBMP, 2);
    Serial.println(" m");
  } else {
    Serial.println("BMP280 no disponible.");
  }

  // ================================================
  // ENVIAR A UBIDOTS
  // ================================================

  ubidots->add("temperatura_dht22", temperaturaDHT);
  ubidots->add("humedad_dht22", humedadDHT);

  if (bmpDisponible) {
    ubidots->add("temperatura_bmp280", temperaturaBMP);
    ubidots->add("presion_bmp280", presionBMP);
    ubidots->add("altitud_bmp280", altitudBMP);
  }

  // La guia oficial utiliza send() sin DEVICE_LABEL.
  bool enviado = ubidots->send();

  if (enviado) {
    Serial.println("Valores enviados correctamente a Ubidots.");
  } else {
    Serial.println("Error al enviar valores a Ubidots.");
  }

  Serial.println("==============================");
}