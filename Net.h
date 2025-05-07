#include <WiFi.h>
#include <WiFiUdp.h>
//修改说明:手机开热点(频段2.4G)设为你的热点名和密码
#define WIFI_SSID "kvtoMobilePhone"
#define WIFI_PASS "k88666666"
#define UDP_PORT 9527
#define MAX_PACKET_SIZE 255

//设置方法:先编译上传服务端代码到一个板子,然后看你热点已连接,看ip地址改下面
#define SERVER_IP IPAddress(192, 168, 43, 63)
//设置方法:你把服务端的断电(防止两个设备你分不清),编译上传客户端代码到另一个板子,然后看你热点已连接,看ip地址改下面
#define CLIENT_IP IPAddress(192, 168, 43, 4)

WiFiUDP UDP;
#define PACKET_SIZE 6
#define VOID_VALUE 255
uint8_t packet[PACKET_SIZE];
bool valid_digit(int a) {
  return packet[a] != VOID_VALUE;
}
bool valid_digit(int a, int b) {
  return packet[a] != VOID_VALUE && packet[b] != VOID_VALUE;
}
void clear_packet() {
  memset(packet, 255, sizeof(packet));
}
void setup_net() {
  clear_packet();
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

void send_to(IPAddress targetIP) {

  UDP.beginPacket(targetIP, UDP_PORT);
  UDP.write(packet, PACKET_SIZE);  // 发送原始字节数组
  UDP.endPacket();
  clear_packet();
}
void send_to_client() {
  send_to(CLIENT_IP);
}
void send_to_server() {
  send_to(SERVER_IP);
}
bool try_receive() {

  int packetSize = UDP.parsePacket();
  if (packetSize) {
    int len = UDP.read(packet, MAX_PACKET_SIZE);
    if (len >= PACKET_SIZE) {
      for (int i = 0; i < PACKET_SIZE; i++) {
        Serial.print(" ");
        Serial.println(packet[i], HEX);
      }
      return true;
    } else {
      Serial.println("Received packet too small.");
      return false;
    }
  }
  return false;
}
void receive() {
  while (true) {
    if (try_receive()) {
      break;
    }
  }
}
