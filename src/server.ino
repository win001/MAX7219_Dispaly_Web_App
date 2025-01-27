#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

extern char newMessage[75];
extern bool newMessageAvailable;

extern uint8_t scrollSpeed;
extern textEffect_t scrollinEffect;
extern textEffect_t scrolloutEffect;
extern textPosition_t scrollAlign;
extern uint16_t scrollPause;

// Fixed IP configuration
IPAddress local_IP(192, 168, 1, 45);  // Change to your desired IP
IPAddress gateway(192, 168, 29, 1);      // Your router's IP
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);             // Google DNS

WebServer server(80);

void serverSetup() {
//   Serial.begin(115200);
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Configure static IP
  // if (!WiFi.config(local_IP, gateway, subnet, dns)) {
  //   Serial.println("STA Failed to configure");
  // }

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize mDNS
  if (!MDNS.begin("ledmatrix")) {   // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");  

  // Route for root index.html
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Handle form submission
  server.on("/submit", HTTP_POST, []() {
    String jsonData = server.arg("plain");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonData);
    newMessageAvailable = true;
    // Process the received data
    String response = "Received inputs:\n";
    JsonObject obj = doc.as<JsonObject>();
    for(JsonPair p : obj) {

        response += String(p.key().c_str()) + ": " + String(p.value().as<const char*>()) + "\n";

        if (String(p.key().c_str()) == "alignment") {
            scrollAlign = (textPosition_t)p.value().as<int>();
        } else if (String(p.key().c_str()) == "inEffect") {
            scrollinEffect = (textEffect_t)p.value().as<int>();
        } else if (String(p.key().c_str()) == "outEffect") {
            scrolloutEffect = (textEffect_t)p.value().as<int>();            
        } else if (String(p.key().c_str()) == "scrollDelay") {
            scrollSpeed = p.value().as<int>();
        } else if (String(p.key().c_str()) == "pauseDuration") {
            scrollPause = p.value().as<int>();
        } else if (String(p.key().c_str()) == "displayText") {
            strlcpy(newMessage, (char*)p.value().as<const char*>(), sizeof(newMessage)); // Update the message
        } else {
            response += "Invalid key\n";
            newMessageAvailable = false;
        }

    //   strlcpy(newMessage, (char*)p.value().as<const char*>(), sizeof(newMessage)); // Update the message     
        Serial.println(response);
    }
    
    server.send(200, "text/plain", response);
  });

  server.begin();
}

void serverLoop() {
  server.handleClient();
}