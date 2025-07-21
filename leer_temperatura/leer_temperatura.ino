#include <MKRWAN.h>
#include "leer_temperatura.h" 

#include <Adafruit_Sensor.h>
#include <DHT.h>

// Selecciona tu región: AS923, AU915, EU868, KR920, IN865, US915, US915_HYBRID
_lora_band region = EU868;

// Inicializa el módem LoRa (usando Serial1, que es el puerto del módulo LoRa en las placas MKR WAN)
LoRaModem modem(Serial1);

// Configuración del sensor DHT11:
#define DHTPIN 2         // Ajusta el pin según tu conexión
#define DHTTYPE DHT11    // Usamos DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuración del sensor de humedad del suelo capacitivo:
#define SOIL_MOISTURE_PIN A0  // Pin analógico para el sensor de humedad del suelo

// Valores de calibración del sensor de humedad del suelo (según tus mediciones)
#define SOIL_MOISTURE_DRY 700   // Valor en suelo seco
#define SOIL_MOISTURE_WET 300   // Valor en suelo húmedo

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  // Inicializa el sensor DHT
  dht.begin();
  
  // Configura el pin del sensor de humedad del suelo como entrada
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  
  // Inicializa el módem LoRa
  if (!modem.begin(region)) {
    Serial.println("Error: No se pudo iniciar el módulo LoRa");
    while (1) {}
  }
  
  Serial.print("Device EUI: ");
  Serial.println(modem.deviceEUI());

  // Realiza el join OTAA utilizando las credenciales definidas en leer_temperatura.h
  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Error en el join OTAA; revisa tus credenciales y ubicación");
    while (1) {}
  }
  Serial.println("¡Conectado a la red!");

  // Configura parámetros adicionales: activa ADR y establece data rate bajo
  modem.setADR(true);
  modem.dataRate(5);
}

void loop() {
  // Lee la temperatura del sensor DHT11
  float temperature = dht.readTemperature();
  
  // Lee el valor analógico del sensor de humedad del suelo
  int rawSoilMoisture = analogRead(SOIL_MOISTURE_PIN);
  
  // Convierte el valor bruto a porcentaje (0-100%)
  // Nota: Como los valores están invertidos (mayor = más seco),
  // invertimos los parámetros en el mapeo
  float soilMoisturePercent = map(rawSoilMoisture, SOIL_MOISTURE_DRY, SOIL_MOISTURE_WET, 0, 1000) / 10.0;
  
  // Asegúrate de que los valores están dentro del rango 0-100%
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  
  // Verifica errores de lectura
  if (isnan(temperature)) {
    Serial.println("Error al leer la temperatura del sensor DHT11");
    delay(2000);
    return;
  }
  
  // Convierte los valores a enteros multiplicando por 10 (por ejemplo, 25.3 °C → 253)
  int16_t temp_int = (int16_t)(temperature * 10);
  int16_t soil_int = (int16_t)(soilMoisturePercent * 10);
  
  // Imprime los valores en el monitor serie
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humedad del suelo (raw): ");
  Serial.println(rawSoilMoisture);
  
  Serial.print("Humedad del suelo: ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");
  
  // Crea un buffer de 4 bytes:
  // - Bytes 0 y 1: temperatura (alto y bajo)
  // - Bytes 2 y 3: humedad del suelo (alto y bajo)
  uint8_t payload[4];
  payload[0] = (uint8_t)(temp_int >> 8);
  payload[1] = (uint8_t)(temp_int & 0xFF);
  payload[2] = (uint8_t)(soil_int >> 8);
  payload[3] = (uint8_t)(soil_int & 0xFF);

  // Imprime en el monitor serie el payload en formato hexadecimal para depuración
  Serial.print("Payload (hex): ");
  for (int i = 0; i < 4; i++) {
    if (payload[i] < 16) Serial.print("0");
    Serial.print(payload[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Envía el payload en formato binario a la red LoRaWAN
  modem.beginPacket();
  for (int i = 0; i < 4; i++) {
    modem.write(payload[i]);
  }
  int err = modem.endPacket(false);
  
  if (err > 0) {
    Serial.println("Paquete enviado correctamente.");
  } else {
    Serial.println("Error al enviar el paquete.");
  }
  
  // Espera 1 minuto antes de enviar la siguiente lectura
  delay(60000);
}