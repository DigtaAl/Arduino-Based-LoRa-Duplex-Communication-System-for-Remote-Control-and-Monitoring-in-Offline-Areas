#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

// Mengatur parameter tampilan OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 1
Adafruit_SH1106 display(OLED_RESET);

// Mengatur pin tombol
#define BUTTON_PIN 3
#define BUTTON_PIN2 4
#define BUTTON_PIN3 5
#define BUTTON_PIN4 6

// Mengatur pin LED
#define LED_PIN  A0
#define LED_PIN2 A1
#define LED_PIN3 A2
#define LED_PIN4 A3
const int txLed = 7;
const int rxLed = 8;

// Mengatur pin relay
const int relayPin1 = 4;
const int relayPin2 = 5;
const int relayPin3 = 6;
const int relayPin4 = 7;

// Mengatur pin LoRa
const int csPin = 10;
const int resetPin = 9;
const int irqPin = 2;

String outgoing;
String data;

byte msgCount = 0;  // Jumlah pesan yang dikirim
byte localAddress = 0xFF;  // Alamat lokal perangkat ini
byte destination = 0xBB;  // Alamat tujuan pesan
long lastSendTime = 0;  // Waktu terakhir pengiriman pesan
int interval = 2000;  // Interval waktu antar pengiriman pesan

void setup() {
  Serial.begin(9600);
  while (!Serial);
  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(433E6)) {  // Menginisialisasi modul LoRa
    Serial.println("Starting LoRa Failed!");
    while (true);
  }
  Serial.println("LoRa Initialized");

  // Menginisialisasi tampilan OLED
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);
  pinMode(txLed, OUTPUT);
  pinMode(rxLed, OUTPUT);
  display.clearDisplay();
}

void loop() {
  // Membaca keadaan tombol
  byte buttonState = digitalRead(BUTTON_PIN);
  byte buttonState2 = digitalRead(BUTTON_PIN2);
  byte buttonState3 = digitalRead(BUTTON_PIN3);
  byte buttonState4 = digitalRead(BUTTON_PIN4);

  // Memproses data dari Serial
  if (Serial.available()) {
    data = Serial.readStringUntil('\n');
    processSerialCommand(data);
  }

  // Pengiriman pesan berdasarkan tombol yang ditekan
  if (buttonState == LOW) {
    Serial.println("Send 1");
    String message = "1";
    sendMessage(message);
    static bool sendA = true;
    sendMessage(sendA ? "A" : "a");
    sendA = !sendA;
    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  if (buttonState2 == LOW) {
    Serial.println("Send 2");
    String message = "2";
    sendMessage(message);
    static bool sendB = true;
    sendMessage(sendB ? "B" : "b");
    sendB = !sendB;
    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  if (buttonState3 == LOW) {
    Serial.println("Send 3");
    String message = "3";
    sendMessage(message);
    static bool sendC = true;
    sendMessage(sendC ? "C" : "c");
    sendC = !sendC;
    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  if (buttonState4 == LOW) {
    Serial.println("Send 4");
    String message = "4";
    sendMessage(message);
    static bool sendD = true;
    sendMessage(sendD ? "D" : "d");
    sendD = !sendD;
    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  // Memanggil fungsi untuk menerima pesan
  onReceive(LoRa.parsePacket());
}



void sendMessage(String outgoing) {
  digitalWrite(txLed, HIGH);
  LoRa.beginPacket();
  LoRa.write(destination);
  LoRa.write(localAddress);
  LoRa.write(msgCount);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();
  msgCount++;
  digitalWrite(txLed, LOW);
}

void onReceive(int packetSize) {
  digitalWrite(rxLed, HIGH);
  if (packetSize == 0) return;
  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingMsgId = LoRa.read();
  byte incomingLength = LoRa.read();
  String incoming = "";
  digitalWrite(rxLed, LOW);

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  processCommand(incoming);

  if (incomingLength != incoming.length()) {
    return;
  }

  // Parsing data yang diterima
  int firstCommaIndex = incoming.indexOf(',');
  float temperature = incoming.substring(0, firstCommaIndex).toFloat();

  int secondCommaIndex = incoming.indexOf(',', firstCommaIndex + 1);
  float humidity = incoming.substring(firstCommaIndex + 1, secondCommaIndex).toFloat();

  int thirdCommaIndex = incoming.indexOf(',', secondCommaIndex + 1);
  int flameValue = incoming.substring(secondCommaIndex + 1, thirdCommaIndex).toInt();

  int fourCommaIndex = incoming.indexOf(',', thirdCommaIndex + 1);
  int lightValue = incoming.substring(thirdCommaIndex + 1).toInt();

  int stat_r1 = incoming.substring(fourCommaIndex + 1).toInt();

  // Menampilkan data ke OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Kontrol & Monitoring");

  display.setCursor(0, 20);
  display.print("Temp : ");
  display.print(temperature);
  display.print(" C");

  display.setCursor(0, 30);
  display.print("Hum  : ");
  display.print(humidity);
  display.print(" %");

  display.setCursor(0, 40);
  display.print("Flame: ");
  display.print(flameValue);
  display.print(" %");

  display.setCursor(0, 50);
  display.print("Light: ");
  display.print(lightValue);
  display.print(" %");
  display.display();

  // Mengirim data ke Serial
  Serial.print("<A0"); Serial.print(temperature); Serial.print("A0>");
  Serial.print("<A1"); Serial.print(humidity); Serial.print("A1>");
  Serial.print("<A2"); Serial.print(flameValue); Serial.print("A2>");
  Serial.print("<A3"); Serial.print(lightValue); Serial.print("A3>");

  // Mengontrol LED berdasarkan status
  if (stat_r1 == 0000) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 0001) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 0010) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 0011) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 0100) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 0101) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 0110) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 0111) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 1000) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 1001) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 1010) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 1011) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 1100) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 1101) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, HIGH);
  } else if (stat_r1 == 1110) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, LOW);
  } else if (stat_r1 == 1111) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN4, HIGH);
  }
}

// Fungsi untuk memproses perintah dari Serial
void processSerialCommand(String command) {
  processCommand(command);
  sendMessage(command);
}

// Fungsi untuk memproses perintah
void processCommand(String command) {
  if (command == "A") {
    digitalWrite(relayPin1, LOW);
  } else if (command == "a") {
    digitalWrite(relayPin1, HIGH);
  }
  if (command == "B") {
    digitalWrite(relayPin2, LOW);
  } else if (command == "b") {
    digitalWrite(relayPin2, HIGH);
  }
  if (command == "C") {
    digitalWrite(relayPin3, LOW);
  } else if (command == "c") {
    digitalWrite(relayPin3, HIGH);
  }

  if (command == "D") {
    digitalWrite(relayPin4, LOW);
  } else if (command == "d") {
    digitalWrite(relayPin4, HIGH);
  }
}
