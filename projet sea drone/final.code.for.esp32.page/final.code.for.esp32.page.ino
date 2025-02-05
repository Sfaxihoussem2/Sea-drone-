#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>
#include <ArduinoJson.h>


//Camera related constants
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

const char *ssid = "MyWiFiCar";
const char *password = "12345678";

float speed = 0.0;
String latitude = "Unknown";
String longitude = "Unknown";
String position = "Unknown";
String heading = "Unknown";
AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
AsyncWebSocket wsCarInput("/CarInput");
uint32_t cameraClientId = 0;

const char *htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<style>
  body, html {
    height: 100%;
    margin: 0;
    font-family: Arial, sans-serif;
   background-color: #F5F5DC;
  
  }
  .slidecontainer {
    position: fixed; /* Changed from absolute to fixed to maintain position regardless of scrolling */
    top: 50%; /* Center vertically */
    transform: translateY(-50%);
    width: 50%; /* Take half of the screen width */
  }
  #leftPanel {
    left: 00%;
    top: 30%;

  }
  #rightPanel {
    right: 0;
    left: 80%;
    top: 30%;
  }
  .slider {
    -webkit-appearance: none;
    width: 35%;
    height: 8px;
    border-radius: 5px;
    background: #87CEEB;
    outline: none;
    opacity: 0.7;
    -webkit-transition: .2s;
    transition: opacity .2s;
transform: rotate(270deg);
  }
  .slider:hover {
    opacity: 1;
  }
  .slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 25px;
    height: 25px;
    border-radius: 50%;
    background: red;
    cursor: pointer;
  }
  .slider::-moz-range-thumb {
    width: 25px;
    height: 25px;
    border-radius: 50%;
    background: red;
    cursor: pointer;
  }
  table {
    width: 100%;
    border-collapse: collapse;
    margin-top: 20px; /* Adds space between the image and the table */
  }
</style>
</head>
<body class="noselect">
<span style="margin-left: 35%;
    font-family: 'Bookman-Demi';
    font-weight: bold;
    font-size: 40px;color:#000080;">Welcome to your drone USV</span>

<img id="cameraImage" src="" style="width: 40%;
    height: 35%; margin-left: 30%;
    margin-top: 7%;"></td>
 
  </style>
</head>
<body class="noselect">
  <div id="leftPanel" class="slidecontainer">
    <input type="range" min="0" max="255" value="0" class="slider" id="Light" oninput='sendButtonInput("Light", value)'>
  </div>
  <div id="rightPanel" class="slidecontainer" >
    <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendButtonInput("Speed", value)'>
  </div>
 
  </table>
  <!-- New table added below for displaying Speed, Heading, and Position -->
  <!-- New Table for Speed, Heading, and Position -->

  <div class="container">
        <span style="float:left;margin-left: 7%;font-family: 'Verdana';font-weight: bold;font-size: 19px;">Babord</span>
        <span style="float:right;margin-right: 8%;font-family: 'Verdana';font-weight: bold;font-size: 19px;">Tribord</span>
    </div>
  <table style="width: 60%;
    margin-top: 10%;
    margin-left: 20%; border-collapse: collapse; border-spacing: 0;">
    <tr>
      <td style="text-align: center; border: 1px solid black;padding: 15px;font-weight: bold;background-color: #f2f2f2;">SPEED</td>
      <td style="text-align: center; border: 1px solid black;padding: 15px;font-weight: bold;background-color: #f2f2f2;">HEADING</td>
      <td style="text-align: center; border: 1px solid black;padding: 15px;font-weight: bold;background-color: #f2f2f2;">POSITION</td>
    </tr>
    <tr>
      <td style="text-align: center; border: 1px solid black;padding: 10px;" id="speedValue">0Nds</td>
      <td style="text-align: center; border: 1px solid black;padding: 10px;" id="headingValue">0degrees</td>
      <td style="text-align: center; border: 1px solid black;padding: 10px;" id="positionValue">Unknown</td>
    </tr>
  </table>
  
    <script>
      var webSocketCameraUrl = "ws:\/\/" + window.location.hostname + "/Camera";
      var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput";      
      var websocketCamera;
      var websocketCarInput;
      
      function initCameraWebSocket() 
      {
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = 'blob';
        websocketCamera.onopen    = function(event){};
        websocketCamera.onclose   = function(event){setTimeout(initCameraWebSocket, 2000);};
        websocketCamera.onmessage = function(event)
        {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
      }
      
      function initCarInputWebSocket() 
      {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onopen    = function(event)
        {
          var speedButton = document.getElementById("Speed");
          sendButtonInput("Speed", speedButton.value);
          var lightButton = document.getElementById("Light");
          sendButtonInput("Light", lightButton.value);
        };
        websocketCarInput.onclose   = function(event){setTimeout(initCarInputWebSocket, 2000);};
        websocketCarInput.onmessage = function(event)
        {
          var data = JSON.parse(event.data);
          if (data.type == "update") {
            document.getElementById("speedValue").innerText = data.speed + " Nds";
            document.getElementById("positionValue").innerText = data.position;
          }
        };        
      }
      
      function initWebSocket() 
      {
        initCameraWebSocket();
        initCarInputWebSocket();
      }

      function sendButtonInput(key, value) 
      {
        var data = key + "," + value;
        websocketCarInput.send(data);
      }
    
      window.onload = initWebSocket;
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";




void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "File Not Found");
}

void updateClients() {
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["type"] = "update";
  jsonDoc["speed"] = speed;
  jsonDoc["heading"] = 0;  // Placeholder for heading
  jsonDoc["position"] = position;

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  wsCarInput.textAll(jsonString);
}


void onCarInputWebSocketEvent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str());
        int valueInt = atoi(value.c_str());
        if (key == "Right") {
          //Serial.println("right: " +valueInt);
          //ledcWrite(PWMSpeedChannel, valueInt);
        } else if (key == "Left") {
          //Serial.println("left: " +valueInt);
          //ledcWrite(PWMLightChannel, valueInt);
        }
        updateClients();
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}

void onCameraWebSocketEvent(AsyncWebSocket *server,
                            AsyncWebSocketClient *client,
                            AwsEventType type,
                            void *arg,
                            uint8_t *data,
                            size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      cameraClientId = 0;
      break;
    case WS_EVT_DATA:
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  if (psramFound()) {
    heap_caps_malloc_extmem_enable(20000);
    Serial.printf("PSRAM initialized. malloc to take memory from psram above this size");
  }
}

void sendCameraPicture() {
  if (cameraClientId == 0) {
    return;
  }
  unsigned long startTime1 = millis();
  //capture a frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Frame buffer could not be acquired");
    return;
  }

  unsigned long startTime2 = millis();
  wsCamera.binary(cameraClientId, fb->buf, fb->len);
  esp_camera_fb_return(fb);

  //Wait for message to be delivered
  while (true) {
    AsyncWebSocketClient *clientPointer = wsCamera.client(cameraClientId);
    if (!clientPointer || !(clientPointer->queueIsFull())) {
      break;
    }
    delay(1);
  }

  unsigned long startTime3 = millis();
  //Serial.printf("Time taken Total: %d|%d|%d\n",startTime3 - startTime1, startTime2 - startTime1, startTime3-startTime2 );
}


// Constantly reads incoming data via Serial2 and updates the values accordingly
void parseIncomingData(String message) {
  if (message.startsWith("speed:")) {
    // Extract speed value
    String speedString = message.substring(6);
    speed = speedString.toFloat();  // Assuming speed is in knots
  } else if (message.startsWith("LAT:")) {
    // Extract latitude
    latitude = message.substring(4);
    updatePosition();
  } else if (message.startsWith("LON:")) {
    // Extract longitude
    longitude = message.substring(4);
    updatePosition();
  }
  updateClients();
}

void updatePosition() {
  position = latitude + ", " + longitude;
}


void setup(void) {

  //Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX (GPIO16), TX (GPIO17)
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  wsCamera.onEvent(onCameraWebSocketEvent);
  server.addHandler(&wsCamera);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.println("HTTP server started");

  setupCamera();
}



void loop() {
  wsCamera.cleanupClients();
  wsCarInput.cleanupClients();
  sendCameraPicture();
  if (Serial.available() > 0) {
    // Read the data sent from Arduino MEGA until a newline character
    String message = Serial.readStringUntil('\n');

    // Parse the incoming data and update the global variables
    parseIncomingData(message);
  }
  //Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
}