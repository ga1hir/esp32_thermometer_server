#include <WiFi.h>           // Wifi接続用ライブラリ
#include <WebServer.h>      // Webサーバ用ライブラリ
#include <Wire.h>           // I2C(センサーとの通信用）ライブラリ
#include <SparkFunBME280.h> // 温度センサ（BME280）用ライブラリ

WebServer server(80);     // Webサーバオブジェクトの生成
const char ssid[] = "WIFITEST";   //Wifi接続SSID
const char pass[] = "******";     //Wifi接続パスワード

BME280 sensor;          //温湿度センサーオブジェクトの生成

uint32_t chipId = 0;    //chipID(個体識別番号）
char chipIdHex[6];

void setup() {          // デバイス起動時の処理
  Serial.begin(115200);

  // chipID(個体識別番号）の取得
  // スケッチ例>ESP32>ChipId>GetChipID.inoから一部をコピペ
  // MACアドレスの下6桁を取得します。
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  // chipID(個体識別番号）を16進に変換して表示  
  sprintf( chipIdHex , "%06X",chipId);
  Serial.print("Chip ID: "); 
  Serial.println(chipIdHex); 

  // WiFi接続
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting WiFi...");
      delay(500);
  }
  
  // WiFiのIPアドレスをシリアルに出力
  Serial.println("WiFi Connected.");
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());

  // 温湿度センサー（BME280）とのI2C接続
  Wire.begin();
  sensor.setI2CAddress(0x76); 
  while( sensor.beginI2C() == false ) {  // Wire を用いて I2C 接続開始
    Serial.println("Connecting I2C Sensor...");
    delay(500);
  }
  Serial.println("I2C Sensor Connected.");

  // Webサーバの起動
  server.on("/", handleRoot);
  server.onNotFound(notFound);
  server.begin();
}

void loop() {     //デバイス起動後の処理（無限ループ）
  server.handleClient();
}


void handleRoot() // "/"にアクセスされた時の処理
{
  String msg="{";             //センサーのデータを取得し、JSONぽく編集
  msg += "\"DeviceID\":\"";   // chipID(個体識別番号）
  msg += chipIdHex;
  //msg += chipId;  
  msg += "\",\"Temp\":\"";    // 温度　C
  msg += sensor.readTempC();
  msg += "\",\"Humidity\":\"";// 湿度　%
  msg += sensor.readFloatHumidity();
  msg += "\",\"Pressure\":\"";// 気圧　hPa
  msg += sensor.readFloatPressure() / 100.0;
  msg += "\"}";  

  server.send(200,"application/json",msg);  //生成したJSONをContent-Type:JSONとしてレスポンス

  Serial.println("Connect From");
}

void notFound() //  "/"以外のURLへアクセスされた場合
{
  String message = "Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message); //404（NotFound）をレスポンス
}
