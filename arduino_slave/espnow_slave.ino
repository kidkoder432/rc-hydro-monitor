#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 2

#define SELF "Car2_S"
#define MASTER "Car2_M"

typedef struct Packet {
  float voltage;
  float amperage;
} Packet;

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = SELF;
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial && millis() < 5000)
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Car 1 S");

  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}

Packet readings[10];

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  digitalWrite(LED_GREEN, LOW);
  memcpy(&readings, data, data_len);
  Serial.print("##  ");

  for (int i = 0; i < 10; i++) {
    Serial.print(readings[i].voltage);
    Serial.print("  ");
    Serial.print(readings[i].amperage);
    Serial.print("\t");

  }
  Serial.println(); 
  delay(20);
  digitalWrite(LED_GREEN, HIGH);

  
}

void loop() {
  // Chill
}
