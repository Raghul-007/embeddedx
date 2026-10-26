#include <SPI.h>
#include <RadioLib.h>

// NodeMCU ESP8266 V3 + RA-02 SX1278
// SCK=D5, MISO=D6, MOSI=D7
// CS=D8, RESET=D0, DIO0=D2, DIO1=D1

SX1278 radio = new Module(15, 4, 16, 5);

unsigned long messageID = 0;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  delay(1000);

  Serial.println();
  Serial.println("ESP8266 LORA SENDER");

  SPI.begin();

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
  Serial.println("Type a message and press Send.");
}

void loop() {
  if (!Serial.available()) {
    delay(5);
    return;
  }

  String message = Serial.readStringUntil('\n');
  message.trim();

  if (message.length() == 0) return;

  if (message.length() > 150) {
    Serial.println("Message too long. Maximum 150 characters.");
    return;
  }

  messageID++;

  String packet =
    "MSG|" + String(messageID) + "|" + message;

  String expectedACK = "ACK|" + String(messageID);
  bool delivered = false;

  Serial.println();
  Serial.print("Original message: ");
  Serial.println(message);

  Serial.print("Complete packet: ");
  Serial.println(packet);

  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("Attempt: ");
    Serial.println(attempt);

    int txState = radio.transmit(packet);

    if (txState != RADIOLIB_ERR_NONE) {
      Serial.print("Transmit error: ");
      Serial.println(txState);
      continue;
    }

    Serial.println("Packet transmitted. Waiting for ACK...");

    String response;
    int rxState = radio.receive(response, 3000);

    if (rxState == RADIOLIB_ERR_NONE) {
      response.trim();

      Serial.print("ACK response: [");
      Serial.print(response);
      Serial.println("]");

      if (response == expectedACK) {
        delivered = true;
        break;
      }

      Serial.println("ACK did not match the message ID.");
    } else {
      Serial.print("ACK receive error: ");
      Serial.println(rxState);
    }

    delay(200);
  }

  if (delivered) {
    Serial.println("DELIVERY CONFIRMED");
  } else {
    Serial.println("DELIVERY NOT CONFIRMED");
  }

  Serial.println("Ready for next message.");
}
