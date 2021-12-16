#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <LittleFS.h>
String ip;
//自定义主页访问处理函数
void homepage() {
  server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\"/><title>WHD</title></head><body><form action=\"/setwifi\" method=\"POST\"><h1>YOYO</h1>device id:" + unionid + "<br/><br/>ssid:<br/><input type=\"text\" name=\"ssid\"><br/><br/>password:<br/><input type=\"text\" name=\"pwd\"><br/><br/><input type=\"submit\" value=\"配置Wifi\"></form></body></html>");
  Serial.println("用户访问了主页");
}
void homepageLogined() {
  server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\"/><title>WHD</title></head><body><h1>YOYO</h1>device id:" + unionid + "<br/><br/>ip:" + ip + "<br/><br/></body></html>");
  Serial.println("用户访问了主页");
}
/**
   写入配置文件
*/
bool saveConfig(char *key, char *value, char*filename) {
  StaticJsonDocument<200> doc;
  doc[key] = value;
  //  doc["serverName"] = "api.example.com";
  //  doc["accessToken"] = "128du9as8du12eoue8da98h123ueh9h98";

  File configFile = LittleFS.open(filename, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    Serial.println(configFile);
    return false;
  }

  serializeJson(doc, configFile);
  delay(200);
  configFile.close();
  return true;
}
/**
   重置wifi信息
*/
void resetWifiConfig() {
  Serial.println("longclick");
  //  重置
  StaticJsonDocument<200> doc;
  doc["wificonfig"] = "";
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
  }

  serializeJson(doc, configFile);
  configFile.close();
  //删除本地ip
  doc["url"] = "";
  configFile = LittleFS.open("/localip.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
  }
  serializeJson(doc, configFile);
  configFile.close();
  ESP.restart();

}
/**
   读取配置文件
*/
const char* loadConfig(char *key, char*file) {
  File configFile = LittleFS.open(file, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return "";
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return "";
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return "";
  }
  configFile.close();
  return doc[key];
}

/**
   初始化文件系统
*/
void initFileSystem() {
  //  LittleFS.format();
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }
}

void setwifi() {
  if (server.arg("ssid") != NULL || server.arg("pwd") != NULL) {
    server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\"/><title>WHD</title></head><body><h1>设置成功，设备已重启</h1></body></html>");
    String wifiConfig = server.arg("ssid") + "|" + server.arg("pwd");
    saveConfig("wificonfig", (char*)wifiConfig.c_str(), "/config.json");
    Serial.println("load");
    Serial.println(loadConfig("wificonfig", "/config.json"));
    delay(1000);
    ESP.restart();
  } else {
    server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\"/><title>WHD</title></head><body><h1>请正确输入wifi名和密码</h1></body></html>");

  }
}

void wifiInit() {
  String wifiConfig = loadConfig("wificonfig", "/config.json");
  Serial.println(wifiConfig);
  if (wifiConfig == "") {
    showLight(1);
    Serial.print("Setting soft-AP ... ");
    String ssid = "yoyo_" + unionid;
    boolean result = WiFi.softAP(ssid, "12345678");
    if (result == true)
    {
      Serial.println("Ready");
      //初始化WebServer
      server.on("/", homepage);
      server.on("/setwifi", HTTP_POST, setwifi);
      server.begin();
      wifiLooping();
      Serial.println("HTTP server started");
    }
    else
    {
      Serial.println("Failed!");
    }
  } else {
    Serial.println(getValue(wifiConfig, '|', 0));
    Serial.println(getValue(wifiConfig, '|', 1));
    //初始化网络
    WiFi.mode(WIFI_STA);
    WiFi.begin(getValue(wifiConfig, '|', 0), getValue(wifiConfig, '|', 1));
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    ip = WiFi.localIP().toString();
    server.on("/", homepageLogined);
    server.begin();
    Serial.println("");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

  }

}
void wifiLooping() {
  while (1) {
    server.handleClient();
  }
}
void wifiLoop() {
  server.handleClient();
}
//拆分字符
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
