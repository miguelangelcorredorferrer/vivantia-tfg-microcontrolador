#include <Arduino.h>
#include <MKRWAN.h>
#include <credenciales.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

/* ───── Config ───── */
#define REGION            EU868
#define LED_PIN           7     // Control bomba (ON/OFF)
#define PUMP_CTRL_PIN     8     // Control absorción de agua
#define CTRL_FPORT        3
#define DATA_FPORT        2      // Telemetría en puerto 2
#define HEARTBEAT_MS      10000UL
#define DATA_INTERVAL_MS  60000UL

// DHT11
#define DHTPIN   6
#define DHTTYPE  DHT11
DHT dht(DHTPIN, DHTTYPE);

// Humedad de suelo
#define SOIL_MOISTURE_PIN A0
#define SOIL_MOISTURE_DRY 880
#define SOIL_MOISTURE_WET 530

// Módem LoRa interno del MKR WAN
LoRaModem modem;

// Tiempos
unsigned long lastDataTx = 0;
unsigned long lastHeartbeat = 0;

/* Espera opcional al puerto serie (solo en DEBUG, con timeout) */
static void waitForSerial(unsigned long ms = 1500) {
#ifdef SERIAL_DEBUG
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0) < ms) { /* nop */ }
#endif
}

/* Uplink corto de control (ack tras downlink) */
void sendUplinkStatus() {
  modem.setPort(CTRL_FPORT);
  modem.beginPacket();
  modem.write(0x01);     // 1 byte (estado/ack)
  modem.endPacket(false);
}

void sendTelemetry() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  int rawSoil = analogRead(SOIL_MOISTURE_PIN);
  float soilPct = map(rawSoil, SOIL_MOISTURE_DRY, SOIL_MOISTURE_WET, 0, 1000) / 10.0;
  soilPct = constrain(soilPct, 0, 100);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer el sensor DHT11");
    return;
  }

  // Conversión x10
  int16_t temp_int = (int16_t)(temperature * 10);
  int16_t hum_int  = (int16_t)(humidity * 10);
  int16_t soil_int = (int16_t)(soilPct * 10);

  uint8_t payload[6];
  payload[0] = (uint8_t)(temp_int >> 8);
  payload[1] = (uint8_t)(temp_int & 0xFF);
  payload[2] = (uint8_t)(hum_int  >> 8);
  payload[3] = (uint8_t)(hum_int  & 0xFF);
  payload[4] = (uint8_t)(soil_int >> 8);
  payload[5] = (uint8_t)(soil_int & 0xFF);

  // Debug
  Serial.print("T: "); Serial.print(temperature, 1); Serial.print(" °C  ");
  Serial.print("H: "); Serial.print(humidity, 1);    Serial.print(" %  ");
  Serial.print("Suelo: "); Serial.print(soilPct, 1); Serial.println(" %");

  Serial.print("Payload (hex): ");
  for (int i = 0; i < 6; i++) {
    if (payload[i] < 16) Serial.print("0");
    Serial.print(payload[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Envío en FPort 2 (Uplink)
  modem.setPort(DATA_FPORT);
  modem.beginPacket();
  modem.write(payload, 6);
  int err = modem.endPacket(false);
  if (err > 0) Serial.println("Uplink enviado (FPort 2)");
  else         Serial.println("❌ Error al enviar telemetría");

  modem.setPort(CTRL_FPORT);
}

void setup() {
  Serial.begin(115200);
  waitForSerial(); // no bloquea si no hay monitor

  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_CTRL_PIN, OUTPUT);

  if (!modem.begin(REGION)) {
    Serial.println("No se pudo iniciar el módulo LoRa");
    while (1) { delay(1000); }
  }

  Serial.print("DevEUI: ");
  Serial.println(modem.deviceEUI());

  // Join OTAA con reintentos
  bool joined = false;
  for (int i = 0; i < 5 && !joined; i++) {
    if (modem.joinOTAA(appEui, appKey)) {
      joined = true;
      Serial.println("¡Unido a la red!");
    } else {
      Serial.println("Falló el join OTAA. Reintentando en 5 s...");
      delay(5000);
    }
  }
  if (!joined) {
    Serial.println("No se pudo unir a la red. Revisa AppEUI/AppKey/gateway.");
    // Si quieres, reintenta en loop() periódicamente.
  }

  modem.setADR(true);
  modem.dataRate(5);
  modem.setPort(CTRL_FPORT);

  // Uplink inmediato al arrancar
  sendTelemetry();
  lastDataTx = millis();
  lastHeartbeat = millis();
}

void loop() {
  modem.poll(); // Gestiona RX/ventanas

  // Telemetría periódica
  if (millis() - lastDataTx >= DATA_INTERVAL_MS) {
    lastDataTx = millis();
    sendTelemetry();
  }

  // Heartbeats
  if (millis() - lastHeartbeat >= HEARTBEAT_MS) {
    lastHeartbeat = millis();
    modem.setPort(CTRL_FPORT);
    modem.beginPacket();
    modem.write(0x00); // payload dummy
    modem.endPacket(false);
    Serial.println("⏱ Uplink periódico (heartbeat)");
  }

  // Downlink: 1 byte (0x01 ON, 0x00 OFF)
  if (modem.available()) {
    uint8_t b = modem.read();
    Serial.print("⬇ Downlink recibido: 0x");
    Serial.println(b, HEX);

    if (b == 0x01) {
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(PUMP_CTRL_PIN, HIGH);
      Serial.println("Bomba ENCENDIDA!!");
    } else {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(PUMP_CTRL_PIN, LOW);
      Serial.println("Bomba APAGADA!!");
    }

    delay(500);
    sendUplinkStatus();
  }
}
