#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Mendefinisikan pin DHT dan tipe sensor yang digunakan
#define DHTPIN 3
#define DHTTYPE DHT22

// Membuat objek dht
DHT dht(DHTPIN, DHTTYPE);

// Mendefinisikan pin sensor cahaya, api, dan LED untuk TX dan RX
const int lightPin = A0;
const int flamePin = A1;
const int txLed = A2;
const int rxLed = A3;

// Mendefinisikan pin relay
const int relayPin1 = 4;
const int relayPin2 = 5;
const int relayPin3 = 6;
const int relayPin4 = 7;

// Mendefinisikan status relay
bool relayOn = false;
bool relayStatus1 = false;
bool relayStatus2 = false;
bool relayStatus3 = false;
bool relayStatus4 = false;

// Mendefinisikan pin untuk modul LoRa
const int csPin = 10;
const int resetPin = 9;
const int irqPin = 2;

String outgoing; // Variabel untuk menyimpan pesan yang akan dikirim
byte msgCount = 0; // Menghitung jumlah pesan
byte localAddress = 0xBB; // Alamat lokal
byte destination = 0xFF; // Alamat tujuan
long lastSendTime = 0; // Waktu terakhir mengirim pesan
int interval = 2000; // Interval pengiriman pesan

void setup() {
  Serial.begin(9600); // Memulai komunikasi serial dengan baud rate 9600
  while (!Serial);
  
  // Mengatur mode pin untuk relay dan LED
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  pinMode(txLed, OUTPUT);
  pinMode(rxLed, OUTPUT);

  // Mengatur pin untuk modul LoRa
  LoRa.setPins(csPin, resetPin, irqPin);

  // Memulai modul LoRa dengan frekuensi 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  Serial.println("LoRa init succeeded.");
  dht.begin(); // Memulai sensor DHT
}

// Fungsi untuk mengontrol relay
void controlRelay(int relayPin, bool on) {
  digitalWrite(relayPin, on ? HIGH : LOW);
  relayOn = on;
}

void loop() {
  // Membaca nilai dari sensor DHT
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Membaca nilai dari sensor api dan cahaya
  int flameValue = analogRead(flamePin);
  flameValue = map(flameValue, 0, 1024, 100, 0);
  int lightValue = analogRead(lightPin);
  lightValue = map(lightValue, 0, 1024, 100, 0);

  // Membaca status relay
  int nilai_r1;
  int stat_r1 = digitalRead(relayPin1);
  int stat_r2 = digitalRead(relayPin2);
  int stat_r3 = digitalRead(relayPin3);
  int stat_r4 = digitalRead(relayPin4);



  // Menentukan nilai relay berdasarkan status masing-masing relay
  if ((stat_r1 == HIGH) && (stat_r2 == HIGH) && (stat_r3 == HIGH) && (stat_r4 == HIGH)) {
    nilai_r1 = 0000;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == HIGH) && (stat_r3 == HIGH) && (stat_r4 == LOW)) {
    nilai_r1 = 0001;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == HIGH) && (stat_r3 == LOW) && (stat_r4 == HIGH)) {
    nilai_r1 = 0010;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == HIGH) && (stat_r3 == LOW) && (stat_r4 == LOW)) {
    nilai_r1 = 0011;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == LOW) && (stat_r3 == HIGH) && (stat_r4 == HIGH)) {
    nilai_r1 = 0100;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == LOW) && (stat_r3 == HIGH) && (stat_r4 == LOW)) {
    nilai_r1 = 0101;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == LOW) && (stat_r3 == LOW) && (stat_r4 == HIGH)) {
    nilai_r1 = 0110;
  }
  else if ((stat_r1 == HIGH) && (stat_r2 == LOW) && (stat_r3 == LOW) && (stat_r4 == LOW)) {
    nilai_r1 = 0111;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == HIGH) && (stat_r3 == HIGH) && (stat_r4 == HIGH)) {
    nilai_r1 = 1000;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == HIGH) && (stat_r3 == HIGH) && (stat_r4 == LOW)) {
    nilai_r1 = 1001;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == HIGH) && (stat_r3 == LOW) && (stat_r4 == HIGH)) {
    nilai_r1 = 1010;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == HIGH) && (stat_r3 == LOW) && (stat_r4 == LOW)) {
    nilai_r1 = 1011;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == LOW) && (stat_r3 == HIGH) && (stat_r4 == HIGH)) {
    nilai_r1 = 1100;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == LOW) && (stat_r3 == HIGH) && (stat_r4 == LOW)) {
    nilai_r1 = 1101;
  }
  else if ((stat_r1 == LOW) && (stat_r2 == LOW) && (stat_r3 == LOW) && (stat_r4 == HIGH)) {
    nilai_r1 = 1110;
  }
  else {
    nilai_r1 = 1111;
  }

  // Membuat string data dari nilai sensor
  String data = String(temperature) + "," + String(humidity) + "," + String(flameValue) + "," + String(lightValue) + "," + String(nilai_r1);

  // Mengirim pesan jika interval waktu telah tercapai
  if (millis() - lastSendTime > interval) {
    sendMessage(data);
    lastSendTime = millis();
    interval = random(2000) + 4000;
  }

  // Menerima pesan dari LoRa
  onReceive(LoRa.parsePacket());
}

// Fungsi untuk mengirim pesan
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

// Fungsi untuk menerima pesan
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

  if (incomingLength != incoming.length()) {
    Serial.println("error: message length does not match length");
    return;
  }

  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;
  }

  // Mengontrol relay berdasarkan pesan yang diterima
  if (incoming == "1") {
    if (!relayOn) {
      controlRelay(relayPin1, true);
      delay(1000);
    } else {
      controlRelay(relayPin1, false);
      delay(1000);
    }
  }
  if (incoming == "2") {
    if (!relayOn) {
      controlRelay(relayPin2, true);
      delay(1000);
    } else {
      controlRelay(relayPin2, false);
      delay(1000);
    }
  }
  if (incoming == "3") {
    if (!relayOn) {
      controlRelay(relayPin3, true);
      delay(1000);
    } else {
      controlRelay(relayPin3, false);
      delay(1000);
    }
  }
  if (incoming == "4") {
    if (!relayOn) {
      controlRelay(relayPin4, true);
      delay(1000);
    } else {
      controlRelay(relayPin4, false);
      delay(1000);
    }
  }

  // Mengontrol relay berdasarkan pesan yang diterima (varian lain)
  if (incoming == "A") {
    controlRelay(relayPin1, true);
    Serial.println("Relay ON");
  } else if (incoming == "a") {
    controlRelay(relayPin1, false);
    Serial.println("Relay OFF");
  }
  if (incoming == "B") {
    controlRelay(relayPin2, true);
    Serial.println("Relay ON");
  } else if (incoming == "b") {
    controlRelay(relayPin2, false);
    Serial.println("Relay OFF");
  }
  if (incoming == "C") {
    controlRelay(relayPin3, true);
    Serial.println("Relay ON");
  } else if (incoming == "c") {
    controlRelay(relayPin3, false);
    Serial.println("Relay OFF");
  }
  if (incoming == "D") {
    controlRelay(relayPin4, true);
    Serial.println("Relay ON");
  } else if (incoming == "d") {
    controlRelay(relayPin4, false);
    Serial.println("Relay OFF");
  }
}
