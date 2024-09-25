#include <WiFiS3.h>
#include <PubSubClient.h>

const char* ssid = "iptime";         // WiFi 네트워크 이름
const char* mqtt_server = "192.168.0.78"; // MQTT 브로커 IP 주소

WiFiClient espClient;
PubSubClient client(espClient);

const int stepPin = 5;    // 스텝 모터 STEP 핀
const int dirPin = 6;     // 스텝 모터 DIR 핀
const int enablePin = 7;  // A4988 ENABLE 핀 (옵션)

void setup() {
  Serial.begin(9600);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  digitalWrite(enablePin, LOW); // 모터 활성화

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (message == "clockwise") {
    rotateMotor(true, 200); // 시계 방향으로 회전
  } else if (message == "counterclockwise") {
    rotateMotor(false, 200); // 반시계 방향으로 회전
  }
}

void rotateMotor(bool clockwise, int steps) {
  digitalWrite(dirPin, clockwise ? HIGH : LOW);
  
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800); // 펄스 폭 조정
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800); // 펄스 간격 조정
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      client.subscribe("arduino/motor/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
}