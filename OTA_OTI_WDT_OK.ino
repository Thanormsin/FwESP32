// Credit to : Farrukh
// Link to : https://www.youtube.com/watch?v=qCmdUtguwPw&t=321s
// Link to : https://www.youtube.com/watch?v=FuY6BobS-1k&t=0s

// Modify by : Thanormsin.M
// Modify Date : 11-Jul-21

#include "5Relay.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

//### OTA ##################
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>
//### OTA ##################

//### OTA WebUpdate ##################
#include <WiFi.h>
//#include <WiFiClient.h>
#include <WebServer.h>
//#include <ESPmDNS.h>
#include <Update.h>

const char* host = "Compackset";
//const char* ssid = "Tha2-2.4G";
//const char* password = "0819065291";

WebServer server(80);
/*
 * Login page
 */
//### OTA WebUpdate ##################

const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

/*
 * setup function
 */
//### OTA WebUpdate ##################

//### LCD ##################
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
//LiquidCrystal_I2C lcd(0x27, 16, 2); // ถ้าไม่ได้ให้ลอง 0x27
LiquidCrystal_I2C lcd(0x27, 20, 4); // ถ้าไม่ได้ให้ลอง 0x27

//const char* host = "Compackset";
const char * ssid = "Tha2-2.4G";
const char * password = "0819065291";

String FirmwareVer = {"2.0"};
#define URL_fw_Version "https://raw.githubusercontent.com/Thanormsin/FwESP32/main/bin_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/Thanormsin/FwESP32/main/Fw.bin"
//#define URL_fw_Bin "https://raw.githubusercontent.com/Thanormsin/FwESP32/main/Fw123.bin"

void connect_wifi();
void firmwareUpdate();
int FirmwareVersionCheck();

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis_2 = 0;
const long interval = 20000;
const long mini_interval = 1000;
void repeatedCall() {
  static int num=0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
  }
  if ((currentMillis - previousMillis_2) >= mini_interval) {
    previousMillis_2 = currentMillis;
    Serial.print("idle loop...");
    Serial.print(num++);
    Serial.print(" Active fw version : ");
    Serial.println(FirmwareVer);
   if(WiFi.status() == WL_CONNECTED) 
   {
     Serial.println("wifi connected");
   }
   else
   {
    connect_wifi();
   }
  }
}

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button_boot = {
  0,
  0,
  false
};

void IRAM_ATTR isr() {
  button_boot.numberKeyPresses += 1;
  button_boot.pressed = true;
}

void setup() { 
  pinMode(button_boot.PIN, INPUT);
  attachInterrupt(button_boot.PIN, isr, RISING);
  Serial.begin(115200);
  Serial.print("Current firmware version:");
  Serial.println(FirmwareVer);
  pinMode(LED_BUILTIN, OUTPUT);
  
  _setup();

  //#### LCD 2004 ##########################
  lcd.begin();
  lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
  lcd.clear();
  
  lcd.home (); 
  
  lcd.setCursor(0,0); 
  lcd.print("CompackSet Ver. 3");

  lcd.setCursor(0,1); 
  lcd.print("Update Over Internet");

  lcd.setCursor(0,2); 
  lcd.print("Are you ready");

  lcd.setCursor(0,3); 
  lcd.print("Let's Start it."); 
  //#### LCD 2004 ##########################

  delay(2000);

  connect_wifi(); 

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("Compackset");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();

  //### OTA WebUpdate ##################
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  //### OTA WebUpdate ##################

  //#### Connect Wifi ##########################
  //lcd.begin();
  //lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
  lcd.clear();
  
  lcd.home (); 
  
  lcd.setCursor(0,0); 
  lcd.print("Exsample Code");

  lcd.setCursor(0,1); 
  lcd.print("OTA,OTI,Web Update");

  lcd.setCursor(0,3); 
  lcd.print("By:Thanormsin.M");
  //#### Connect Wifi ##########################

  delay(2500);
}
void loop() {
  //### OTA ##################
  ArduinoOTA.handle();  

  //### OTA WebUpdate ##################
  server.handleClient();
  //delay(1);
  
  //#### LCD 2004 ##########################
  //lcd.begin();
  lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
  lcd.clear();
  
  lcd.home (); 

  lcd.setCursor(0,0); 
  lcd.print("Board:");
  lcd.print(host);
  
  lcd.setCursor(0,1); 
  lcd.print("SSID:");
  //lcd.setCursor(5,1); 
  lcd.print(ssid);

  lcd.setCursor(0,2); 
  lcd.print("IP:");
  lcd.print(WiFi.localIP());

  lcd.setCursor(0,3); 
  lcd.print("Current Fw. V. ");
  lcd.print(FirmwareVer);
  
  if (button_boot.pressed) { //to connect wifi via Android esp touch app 
    Serial.println("Starting Manual Firmware update..");

    //#### LCD 2004 ##########################
    //lcd.begin();
    lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
    lcd.clear();
    
    lcd.home (); 

    lcd.setCursor(0,0); 
    lcd.print("    -->>  <<--");
  
    lcd.setCursor(0,1); 
    lcd.print("Manual update...");
    
    lcd.setCursor(0,2); 
    lcd.print("Pls. wait amoment");            

    lcd.setCursor(0,3); 
    lcd.print("Update progress ....");
    //#### LCD 2004 ##########################
    
    firmwareUpdate();
    button_boot.pressed = false;
  }
  repeatedCall();
  
  //Blinking
  digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
  delay(500);

  digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
  delay(200);

  digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
  delay(500);

  digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
  delay(200);
  
  digitalWrite(LED_BUILTIN,HIGH);
}

void connect_wifi() {
  Serial.println("Waiting for WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void firmwareUpdate(void) {
  WiFiClientSecure client;
  //client.setCACert(rootCACertificate);
  client.setInsecure();
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);
  
  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());

    //#### LCD 2004 ##########################
    //lcd.begin();
    lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
    lcd.clear();
    
    lcd.home (); 

    lcd.setCursor(0,0); 
    lcd.print("    -->>  <<--");
  
    lcd.setCursor(0,1); 
    lcd.print("Found new firmware");

    lcd.setCursor(0,2); 
    lcd.print("But Update Failer...");
    delay(2000);
    
    //#### LCD 2004 ##########################
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}
int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;
  //Serial.println("Hi");
  //Serial.println(https);

  if (client) 
  {
    //client -> setCACert(rootCACertificate);
    client->setInsecure();

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;
    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);

      Serial.println("http code : "+String(httpCode));
      
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("Device already on latest firmware version : %s\n", FirmwareVer);
      Serial.println("");
      return 0;
    } 
    else 
    {
      //#### LCD 2004 ##########################
      //lcd.begin();
      lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
      lcd.clear();
      
      lcd.home (); 

      lcd.setCursor(0,0); 
      lcd.print("    -->>  <<--");
    
      lcd.setCursor(0,1); 
      lcd.print("Check new firmware");
      
      lcd.setCursor(0,2); 
      lcd.print("Found Fw. ver : ");            
      lcd.print(payload);

      lcd.setCursor(0,3); 
      lcd.print("Update firmware ....");
      //#### LCD 2004 ##########################
      
      Serial.print("New firmware detected ver : ");
      Serial.println(payload);
      Serial.println("Update firmware .....");
      
      return 1;
    }
  } 
  return 0;  
}
