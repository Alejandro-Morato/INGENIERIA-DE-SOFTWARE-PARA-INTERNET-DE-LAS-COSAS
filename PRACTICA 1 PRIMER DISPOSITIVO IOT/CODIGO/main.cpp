/*
 * El ejemplo de esta semana es una conexión hacia la plataforma ThingSpeak para proyectos 
 * de Internet de las Cosas (IoT)
 * Realizado por: Sebastian Carranza - Equipo TodoMaker
 * Propiedad y Derechos: Equipo TodoMaker
 * Para esta semana, necesitamos lo siguiente:
 * ESP32 Dev Kit
 * Sensor DHT11
 * Cables Jumper Male - Female
 * Cable USB a MicroUSB
 * Una red WiFi existente en nuestro hogar, lugar de trabajo, etc.
 * 
 * No habrá una conexión física con algún componente extra.
 */
/*Incluimos primero las librerías*/
#include <WiFi.h>
#include <ThingSpeak.h>
#include "DHT.h"
/*Definimos que el Pin de Datos del sensor estará conectado al pin 2 del Arduino UNO*/
#define DHTPIN  23      
/*Definimos que el tipo de sensor que estamos utilizando es un DHT11*/
#define DHTTYPE DHT11// DHT 11
/*Se actualizan algunos parámetros del DHT11 con los puntos definidos anteriormente*/ 
DHT dht(DHTPIN, DHTTYPE);
/*Definimos como constantes Char las credenciales de acceso a la red WiFi*/
const char* ssid="UAM-ROBOTICA";
const char* password="m4nt32024uat";
/*Definimos las credenciales para la conexión a la plataforma*/
unsigned long channelID = 3459947;
const char* WriteAPIKey ="TLSR7FS2RS3Y5FKS";
/*Definimos el cliente WiFi que usaremos*/
WiFiClient cliente;

void medicion(){
  /*Realizamos la lectura de Temperatura y Humedad del sensor*/
  float temperatura= dht.readTemperature();
  float humedad = dht.readHumidity();
  /*Imprimimos los valores obtenidos en el terminal Serial*/
  Serial.print("Temperatura registrada: ");
  Serial.print(temperatura);
  Serial.println("°C");
  Serial.print("Humedad registrada: ");
  Serial.print(humedad);
  Serial.println("%");
  Serial.println("-----------------------------------------");
  /*Indicamos el orden de envío por campos o Field, en el orden definido de la plataforma, junto a los valores del sensor*/
  ThingSpeak.setField(1,temperatura);
  ThingSpeak.setField(2,humedad);
}


/*Iniciamos la función Setup()*/
void setup() {
  /*Iniciamos el terminal Serial a una velocidad de 115200, junto a un retardo de 1 segundo*/
  Serial.begin(115200);
  delay(1000);
  /*Imprimimos una frase, e iniciamos nuestro sensor DHT*/
  Serial.println("Sensores Instalados y listos");
  dht.begin();
  /*Iniciamos la conexión a la red WiFi, y se imprimirán caracteres indicando el tiempo que tarda la conexión*/
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  /*Una vez conextado, se imprimirá una frase y se iniciará la conexión a la Plataforma usando el cliente definido anteriormente*/
  Serial.println("Conectado al WiFi");
  ThingSpeak.begin(cliente);
  delay(5000);
}

/*Iniciamos la función Loop*/
void loop() {
  /*Usamos un retardo de 5 segundos, y utilizamos la función Medición para la lectura de los sensores*/
  delay(5000);
  medicion();
  /*Hacemos la conexión y envío de datos a la plataforma, utilizando las credenciales definidas anteriormente*/
  ThingSpeak.writeFields(channelID,WriteAPIKey);
  /*Imprimimos una frase indicando el envío, y agregamos un retardo de 10 segundos*/
  Serial.println("Datos enviados a ThingSpeak!");
  delay(10000);
  
}
/*Definimos la función Medición*/
