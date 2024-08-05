#include "WiFiS3.h"
#include <AccelStepper.h>

char ssid[] = "iptimeSmart";        // Network SSID (name)
char pass[] = "12345678";    // Network password (use for WPA, or use as key for WEP)
int keyIndex = 0;              // Network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// 스테퍼 모터 설정
const int dirPin = 2;  // Direction 핀
const int stepPin = 3; // Step 핀
const int stepsPerRevolution = 200; // 17HS3401 모터의 한 바퀴 회전에 필요한 스텝 수

// A4988 드라이버를 위한 AccelStepper 설정
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup() {
  Serial.begin(9600);      // Initialize serial communication
  
  // Set stepper motor speed (RPM)
  stepper.setMaxSpeed(100);  // Set to 1000 steps per second
  stepper.setAcceleration(50);  // Set acceleration

  // Check for WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // Attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // Print the network name (SSID)

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    
    // Wait 10 seconds for connection:
    delay(10000);
    
    // Check connection status
    if (status == WL_CONNECTED) {
      Serial.println("Connected to WiFi network.");
    } else {
      Serial.println("Failed to connect to WiFi network.");
    }
  }

  server.begin();                           // Start web server on port 80
  printWifiStatus();                        // Print status as we're now connected
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New client");           // Print a message out the serial port
    String currentLine = "";                // Make a String to hold incoming data from the client
    while (client.connected()) {            // Loop while the client's connected
      if (client.available()) {             // If there's bytes to read from the client,
        char c = client.read();             // Read a byte, then
        Serial.write(c);                    // Print it out the serial monitor
        if (c == '\n') {                    // If the byte is a newline character

          // If the current line is blank, you got two newline characters in a row.
          // That's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // The content of the HTTP response follows the header:
            client.println("<html><head><meta http-equiv='refresh' content='5'></head><body>");
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/CW\">here</a> to rotate the motor clockwise<br></p>");
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/CCW\">here</a> to rotate the motor counterclockwise<br></p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line:
            client.println();
            // Break out of the while loop:
            break;
          } else {    // If you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // If you got anything else but a carriage return character,
          currentLine += c;      // Add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /CW" or "GET /CCW":
        if (currentLine.indexOf("GET /CW") >= 0) {
          Serial.println("Rotating Clockwise");
          rotateMotor(stepsPerRevolution);
          client.println("HTTP/1.1 303 See Other");
          client.println("Location: /");
          client.println();
          break;
        } else if (currentLine.indexOf("GET /CCW") >= 0) {
          Serial.println("Rotating Counter-Clockwise");
          rotateMotor(-stepsPerRevolution);
          client.println("HTTP/1.1 303 See Other");
          client.println("Location: /");
          client.println();
          break;
        }
      }
    }
    // Close the connection:
    client.stop();
    Serial.println("Client disconnected");
  }
}

void rotateMotor(int steps) {
  stepper.move(steps);  // Move the specified number of steps
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void printWifiStatus() {
  // Print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // Print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}