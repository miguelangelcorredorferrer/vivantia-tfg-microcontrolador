#include <MKRWAN.h>

/* ───── Claves OTAA como cadenas HEX ─────
 * Copia los valores exactamente como aparecen
 * en la consola TTN (sin “0x”, sin espacios).
 */
const char  appEui[] = "2DCF462904B478D8";                 // 8  bytes ⇒ 16 dígitos
const char  appKey[] = "F3E58E7A7D7B16491F74C653108A5714"; // 16 bytes ⇒ 32 dígitos

/* ───────── Parámetros de la demo ───────── */
#define REGION        EU868      // eu868, us915, …
#define LED_PIN       13         // LED interno
#define CTRL_FPORT    1         // Puerto para comandos
#define HEARTBEAT_MS  60000UL    // 1 minuto

LoRaModem modem(Serial1);
unsigned long lastTx = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(LED_PIN, OUTPUT);

  if (!modem.begin(REGION)) {
    Serial.println("❌  No se pudo iniciar el módulo LoRa");
    while (1);
  }

  Serial.print("DevEUI: "); Serial.println(modem.deviceEUI());

  /* Join OTAA (cadenas HEX) */
  if (!modem.joinOTAA(appEui, appKey)) {
    Serial.println("❌  Falló el join OTAA");
    while (1);
  }
  Serial.println("✅  ¡Uni­do a la red!");

  modem.setPort(CTRL_FPORT);  // Puerto por defecto para TX/RX
  modem.setADR(true);
  modem.dataRate(5);          // SF7 en EU868
}

void loop() {
  // 1) Mantén a la librería atendida:
  modem.poll();                     // <— importante en MKRWAN

  // 2) Heartbeat
  if (millis() - lastTx >= HEARTBEAT_MS) {
    lastTx = millis();
    modem.beginPacket();
    int err = modem.endPacket(false);   // unconfirmed
    Serial.println(err > 0 ? "Heartbeat enviado" : "Error TX");
  }

  // 3) Forzar uplink desde consola (para test rápido)
  if (Serial.available() && Serial.read() == 't') {
    modem.beginPacket();
    modem.write(0x42);                  // dummy byte
    modem.endPacket(false);
    Serial.println("Uplink forzado");
  }

  // 4) Procesar downlink
  if (modem.available()) {
    uint8_t b = modem.read();           // solo necesitamos el 1.er byte
    Serial.print("⬇  Downlink byte 0x"); Serial.println(b, HEX);
    digitalWrite(LED_PIN, b == 0x01 ? HIGH : LOW);
    Serial.println(b == 0x01 ? "💡 LED encendido" : "💡 LED apagado");
  }
}

