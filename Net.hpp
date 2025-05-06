#include <WiFi.h>
#include <WiFiUdp.h>
#define WIFI_SSID "kvtoMobilePhone"
#define WIFI_PASS "k88666666"
#define UDP_PORT 9527
#define MAX_PACKET_SIZE 255

#define SERVER_IP IPAddress(192, 168, 43, 1)  // 替换为你想发送的目标 IP 地址
#define CLIENT_IP IPAddress(192, 168, 43, 2)

WiFiUDP UDP;


void setup_net() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);
}

void send_to(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, IPAddress targetIP) {
  uint8_t payload[4] = { b1, b2, b3, b4 };
  UDP.beginPacket(targetIP, UDP_PORT);
  UDP.write(payload, 4);  // 发送原始字节数组
  UDP.endPacket();
}
void send_to_client(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
  send_to(b1, b2, b3, b4, CLIENT_IP);
}
void send_to_server(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
  send_to(b1, b2, b3, b4, SERVER_IP);
}
bool try_receive(uint8_t& b1, uint8_t& b2, uint8_t& b3, uint8_t& b4) {
  static uint8_t packet[4];
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    int len = UDP.read(packet, MAX_PACKET_SIZE);
    if (len >= 4) {
      b1 = packet[0];
      b2 = packet[1];
      b3 = packet[2];
      b4 = packet[3];

      Serial.print("Received 4 bytes: ");
      Serial.print(b1, HEX);
      Serial.print(" ");
      Serial.print(b2, HEX);
      Serial.print(" ");
      Serial.print(b3, HEX);
      Serial.print(" ");
      Serial.println(b4, HEX);
      return true;
    } else {
      Serial.println("Received packet too small.");
      return false;
    }
  }
  return false;
}
void receive(uint8_t& b1, uint8_t& b2, uint8_t& b3, uint8_t& b4) {
  while (true) {
    if (try_receive(b1, b2, b3, b4)) {
      break;
    }
  }
}
bool received(const uint8_t equal_b1, const uint8_t equal_b2, const uint8_t equal_b3, const uint8_t equal_b4) {

  uint8_t b1, b2, b3, b4;
  return try_receive(b1, b2, b3, b4) && equal_b1 == b1 && equal_b2 == b2 && equal_b3 == b3 && equal_b4 == b4;
}