// Credit to : Farrukh
// Link to : https://www.youtube.com/watch?v=qCmdUtguwPw&t=321s
// Link to : https://www.youtube.com/watch?v=FuY6BobS-1k&t=0s

// Modify by : Thanormsin.M
// Modify Date : 11-Jul-21

//### WDT ##################
#include "esp_task_wdt.h"
//3 seconds WDT
#define WDT_TIMEOUT 3000
//if 1 core doesn't work, try with 2
#define CONFIG_FREERTOS_NUMBER_OF_CORES 1 
esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WDT_TIMEOUT,
        .idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,    // Bitmask of all cores
        .trigger_panic = true,
    };
    
//### WDT ##################

#include "Beginner.h"

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
//#include <WiFi.h>
//#include <WiFiClient.h>
#include <WebServer.h>
//#include <ESPmDNS.h>
#include <Update.h>

//OTAWiFi
const char* host = "Beginner";

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

//### WiFi Connect ##################
const char * ssid = "Tha2-2.4G";
const char * password = "0819065291";
//### WiFi Connect ##################

//### Auto WiFi2Connect ##################
const char* ssidNm    = "";

const char* ssid1     = "Tha2-2.4G";        //<-- ใส่ชือ Wifi รอง ตรงนี ถ้าใช้ WiFi ตัวเดียวก็ให้ใส่เหมือนกันกับ ssid1
const char* password1 = "0819065291";         //<-- ใส่รหัส Wifi รอง ตรงนี

const char* ssid2     = "Tha3-2.4G";        //<-- ใส่ชือ Wifi หลัก
const char* password2 = "0819065291";         //<-- ใส่รหัส Wifi หลัก ตรงนี

//### Other ##################
int lastWiFi = 0;
//int ReCnctWiFi = 1;
//### Auto WiFi2Connect ##################

String FirmwareVer = {"3.2"};
#define URL_fw_Version "https://raw.githubusercontent.com/Thanormsin/FwESP32/main/BeginnerFw_Ver.txt"
#define URL_fw_Bin     "https://raw.githubusercontent.com/Thanormsin/FwESP32/main/Beginner_Bin.bin"

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
    //Connect2WiFi();
    
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
  
  Serial.print("Current firmware version:");
  Serial.println(FirmwareVer);
  pinMode(LED_BUILTIN, OUTPUT);
 
  _setup();

  connect_wifi(); 
  //Connect2WiFi();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  //ArduinoOTA.setHostname("BeginnersetOTA");
  ArduinoOTA.setHostname(host);

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

  ShowWiFiConnect();
  delay(2000);

  //### Easy Schdule
//  Task1.start(); // สั่งให้งาน Task เริ่มทำงาน
  //Task2.start();
  //Task3.start();
  //### Easy Schdule

  //### WDT
  vTaskDelay(200);
  Serial.println("Starting");

  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = WDT_TIMEOUT,                             //กำหนดเวลา WDT ถ้าไม่มีการสั่ง ResetWDT ก็จะสั่ง Reboot
      .idle_core_mask = (1 << configNUM_CORES) - 1,    // Bitmask of all cores,
      .trigger_panic = true,
  };
  ESP_ERROR_CHECK(esp_task_wdt_reconfigure(&twdt_config));
  xTaskCreatePinnedToCore(testTask, "Test Task", 2000, NULL, 5, NULL, ARDUINO_RUNNING_CORE);
  //### WDT
}

int i = 0;
int last = millis();
void loop() {
  //vTaskDelete(NULL);
  
  //### OTA ##################
  ArduinoOTA.handle();  

  //### OTA WebUpdate ##################
  server.handleClient();

//  //### WDT ##################
//  // resetting WDT every 2s, 5 times only
//  if (millis() - last >= 2000 && i < 5) {
//      Serial.println("Resetting WDT...");
//      esp_task_wdt_reset();
//      last = millis();
//      i++;
//      if (i == 5) {
//        Serial.println("Stopping WDT reset. CPU should reboot in 3s");
//      }
//
//  }else
//  {
//    Serial.println("Last "+last);
//  }
  //### WDT ##################

  //ทดสอบ Relay On Board
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(Relay[i], HIGH);
    delay(200);
  }
  
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(Relay[i], LOW);
    delay(200);
  }

  //###  ทดสอบ MCP0  ###
  //สั่งเดินหน้า MCP0,MCP1,MCP2,MCP3
  for (int i = 8; i < 16; i++)
  {
    //digitalWrite(MCP0_B[i], HIGH);
    mcp.digitalWrite(MCP0[i], HIGH);
    delay(200);
  }

  delay(1500);
  
  for (int i = 8; i < 16; i++)
  {
    //digitalWrite(MCP0_B[i], HIGH);
    mcp.digitalWrite(MCP0[i], LOW);
    delay(200);
  }
  
  delay(1500);

  //นับถอยหลัง 0
  for (int i = 15; i > 7; i--)
  {
    mcp.digitalWrite(MCP0[i], HIGH);
    delay(200);
  }

  //นับถอยหลังจ 0
  for (int i = 15; i > 7; i--)
  {
    mcp.digitalWrite(MCP0[i], LOW);
    delay(200);
  }
  //###  ทดสอบ MCP0  ###

  //#### RTC DS3231 ##########################ฃ
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.println();
  Serial.print("Tempeature = ");
  Serial.print(rtc.getTemperature()); // คำสั่งดึงอุณหภูมิออกมาแสดง
  Serial.println(" C");

  Serial.println("By ArduinoALL");
  Serial.println();
  //delay(500);

/*
  //### สั่ง Scan MCP0 Input  ###
  for (int i = 0; i < 8; i++)
  {
    mcp.digitalWrite((MCP0[i])+8, mcp.digitalRead(MCP0[i])); // Scan In0
    delay(200);
   
  }
*/

  //### DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  delay(200);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  //### DHT11

  //LDR
  int val = analogRead(A0);
  //LDR
  
  //#### LCD 2004 ##########################
  lcd.begin();
  lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
  lcd.clear();
  
  lcd.home (); 

  lcd.setCursor(3,0); 
  lcd.print("Condition Room");

  ShowTime();
 
  lcd.setCursor(0,2); 
  lcd.print("Temp.");

  lcd.setCursor(0,3); 
  lcd.print(t); // คำสั่งดึงอุณหภูมิออกมาแสดง

  lcd.print((char)223);
  //lcd.print("C");

  lcd.setCursor(8,2); 
  lcd.print("Humi.");

  lcd.setCursor(8,3); 
  lcd.print(h);

  lcd.setCursor(15,2); 
  lcd.print("Lux.");

  lcd.setCursor(15,3); 
  lcd.print(val);

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

////// ตั้งค่า 2 WiFi //////
void Connect2WiFi()
{
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_OFF);

      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      
      WiFi.mode(WIFI_STA);
      if (lastWiFi == 1) {
        Serial.print("Connecting WiFi#2 :");
        Serial.println(ssid2);
        WiFi.begin(ssid2, password2);
        
        lastWiFi = 2;
        ssid = ssid2;   
    
      } else {        
        Serial.print("Connecting WiFi#1 :");
        //WiFi.begin(ssid1, password1);
        
        Serial.println("Start Connect to WiFi"); 
        Serial.println(ssid1);
        WiFi.begin(ssid1, password1);
        
        lastWiFi = 1;
        ssid = ssid1;  
      } 
    
      unsigned long timeout = millis();
      while (WiFi.status() != WL_CONNECTED) {
        if (millis() - timeout > 20000) {
          Serial.println("");
          Serial.println(">>> Connecting Wait Timeout !");
          break;
        }
         
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
      }
    }
    Serial.println("");
    Serial.print("Connected to WIFi: ");
//    Serial.print("SSID: ");
    Serial.println(ssidNm);
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP());     
    Serial.println(""); 

    delay(5000);
}

void firmwareUpdate(void) {
  WiFiClientSecure client;
  //client.setCACert(rootCACertificate);   // ไม่ใช้แล้ว
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
      lcd.print("Update In Progress..");
      //#### LCD 2004 ##########################
      
      Serial.print("New firmware detected ver : ");
      Serial.println(payload);
      Serial.println("Update firmware .....");
      
      return 1;
    }
  } 
  return 0;  
}

void ShowWiFiConnect()
{  
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
  lcd.print("Fw.");
  lcd.print(FirmwareVer);

  lcd.setCursor(12,3); 
  lcd.print("WDT:");
  lcd.print(last);
}

void ShowTime()
{
  //#### RTC DS3231 ##########################ฃ
  DateTime now = rtc.now();
  //#### RTC Show Clcok ##########################
  //lcd.begin();
  //lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
  //lcd.clear();
  //lcd.home (); 
  //### วัน ###
  if (now.month()<10)
  {
    lcd.setCursor(0,1); 
    lcd.print("0");
    lcd.print(now.day(), DEC);
    }
  else {
    lcd.setCursor(0,1); 
    lcd.print(now.day(), DEC);
  }

  lcd.setCursor(2,1); 
  lcd.print("/"); 
  
  //### เดือน ###
  if (now.month()<10)
  {
    lcd.setCursor(3,1); 
    lcd.print("0");
    lcd.print(now.month(), DEC);
    }
  else {
    lcd.setCursor(3,1); 
    lcd.print(now.month(), DEC);
  }

  lcd.setCursor(5,1); 
  lcd.print("/");  

  //### ปี ###
  lcd.setCursor(6,1); 
  lcd.print(now.year(), DEC);

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //### ชั่วโมง ###
  if (now.hour()<10)
  {
    lcd.setCursor(11,1); 
    lcd.print("0");
//    lcd.setCursor(14,0); 
    lcd.print(now.hour(), DEC);
    }
  else {
    lcd.setCursor(11,1); 
    lcd.print(now.hour(), DEC);
  }

  lcd.setCursor(13,1); 
  lcd.print(":");  

  //### นาที ###
  if (now.minute()<10)
  {
    lcd.setCursor(14,1); 
    lcd.print("0");
//    lcd.setCursor(4,1); 
    lcd.print(now.minute(), DEC);
    }
  else {
    lcd.setCursor(14,1); 
    lcd.print(now.minute(), DEC);
  }

  lcd.setCursor(16,1); 
  lcd.print(":");  
   
  //### วินาที ###
  if (now.second()<10)
  {
    lcd.setCursor(17,1); 
    lcd.print("0");
 //   lcd.setCursor(7,1); 
    lcd.print(now.second(), DEC);
    }
  else {
    lcd.setCursor(17,1); 
    lcd.print(now.second(), DEC);
  }
 
}

void SenserShw1()
{   
  ShowTime();

  //#### LCD 2004 ##########################
  //lcd.begin();
  //lcd.backlight(); //คำสั่งเปิดไฟแบล็คไลท์ 
  //lcd.clear();
  
  lcd.home (); 

  lcd.setCursor(3,0); 
  lcd.print("Sensor Show");
  
  lcd.setCursor(0,2); 
  lcd.print("WtrTmp.");

//  //### DS12B80
//  OneWire32 ds(26); //gpio pin
//  
//  uint64_t addr[MaxDevs];
//  
//  //uint64_t addr[] = {
//  //  0x183c01f09506f428,
//  //  0xf33c01e07683de28,
//  //};
  
//  //to find addresses
//  uint8_t devices = ds.search(addr, MaxDevs);
//  for (uint8_t i = 0; i < devices; i += 1) {
//    //Serial.printf("%d: 0x%llx,\n", i, addr[i]);
//    char buf[20]; snprintf( buf, 20, "0x%llx,", addr[i] ); Serial.println(buf);
//  }
//  //end
//
//  //for(;;){
//    ds.request();
//    //vTaskDelay(750 / portTICK_PERIOD_MS);
//    for(byte i = 0; i < MaxDevs; i++){
//      uint8_t err = ds.getTemp(addr[i], currTemp[i]);
//      if(err){
//        const char *errt[] = {"", "CRC", "BAD","DC","DRV"};
//        Serial.print(i); Serial.print(": "); Serial.println(errt[err]);
//      }else{
//        char buf[20];
//        snprintf( buf, 20, "0x%llx,", addr[i] );
//        Serial.print("Device: " + String(i) + " " + buf + " "); Serial.println(currTemp[i]);
//      }
//    }
//    //vTaskDelay(1000 / portTICK_PERIOD_MS);
//  //}
//  //### DS12B80
  
  lcd.setCursor(0,3); 
//  lcd.print(currTemp[0]); // คำสั่งดึงอุณหภูมิออกมาแสดง

  lcd.print((char)223);
  //lcd.print("C");

  lcd.setCursor(8,2); 
  lcd.print("TDS.");

  lcd.setCursor(8,3); 
//  lcd.print(tdsValue,2);

  lcd.setCursor(15,2); 
  lcd.print("PH.");

//  lcd.setCursor(15,3); 
//  lcd.print(val);
  delay(500);
}
