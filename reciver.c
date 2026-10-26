#include <SPI.h>
#include <RadioLib.h>

// ESP32 + RA-02 SX1278
// SCK=GPIO18, MISO=GPIO19, MOSI=GPIO23
// CS=GPIO5, RESET=GPIO14, DIO0=GPIO26

SX1278 radio = new Module(5, 26, 14, RADIOLIB_NC);

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("ESP32 LORA RECEIVER");

  SPI.begin(18, 19, 23, 5);

  int state = radio.begin(
    433.0, 125.0, 9, 7, 0x12, 10, 8
  );

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("LoRa init failed: ");
    Serial.println(state);
    while (true) delay(1000);
  }

  radio.setCRC(true);

  Serial.println("LoRa initialized successfully");
  Serial.println("Waiting for messages...");
}

void loop() {
  String packet;

  // DIO0 must be connected to GPIO26
  int rxState = radio.receive(packet);

  if (rxState == RADIOLIB_ERR_NONE) {
    Serial.println();
    Serial.println("===== PACKET RECEIVED =====");

    Serial.print("Raw packet: [");
    Serial.print(packet);
    Serial.println("]");

    Serial.print("Packet length: ");
    Serial.println(packet.length());

    Serial.print("RSSI: ");
    Serial.print(radio.getRSSI());
    Serial.println(" dBm");

    if (packet.startsWith("MSG|")) {
      int separator1 = packet.indexOf('|');
      int separator2 =
        packet.indexOf('|', separator1 + 1);

      if (separator1 >= 0 && separator2 > separator1) {
        String id =
          packet.substring(separator1 + 1, separator2);

        String message =
          packet.substring(separator2 + 1);

        Serial.print("Message ID: ");
        Serial.println(id);

        Serial.print("RECEIVED MESSAGE: ");
        Serial.println(message);

        // Acknowledge only a valid MSG packet
        String ack = "ACK|" + id;

        Serial.print("Sending ACK: ");
        Serial.println(ack);

        int txState = radio.transmit(ack);

        if (txState == RADIOLIB_ERR_NONE) {
          Serial.println("ACK transmission completed");
        } else {
          Serial.print("ACK transmit error: ");
          Serial.println(txState);
        }
      } else {
        Serial.println("Invalid packet format");
      }
    } else {
      Serial.println("Unexpected packet format");
    }

    Serial.println("===========================");
  } else if (rxState != RADIOLIB_ERR_RX_TIMEOUT) {
    Serial.print("Receive error: ");
    Serial.println(rxState);
    delay(100);
  }
}
