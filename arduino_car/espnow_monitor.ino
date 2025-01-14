#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>  // only for esp_wifi_set_channel()

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float voltage, amperage;

// Global copy of slave
esp_now_peer_info_t slave;
#define CHANNEL 2
#define PRINTSCANRESULTS 0

#define SELF "Car2_M"   // Update
#define SLAVE "Car2_S"  // Update

typedef struct Packet {
  float voltage;
  float amperage;
  
  
} Packet;

long long AcsValue, Samples,VoltageS,SamplesS;
double AvgAcs,AvgV;

struct Packet getReadings() {
  AcsValue = Samples = AvgAcs=VoltageS=SamplesS=AvgV=0;
  for (int x = 0; x < 5000; x++) {  //Get 5000 samples
    AcsValue = analogRead(A0);//Read current sensor values
    VoltageS=analogRead(A1);
    Samples = Samples + AcsValue;
    SamplesS=SamplesS+VoltageS;
    delay(3);                      // let ADC settle before next sample 3ms
  }
  float AvgAcs = Samples / 5000.0;
  float AvgV= SamplesS/5000.0;
  struct Packet readings;

  Serial.println(AvgAcs);
  readings.amperage = fmap(AvgAcs, 3103, 4021, 0.0, 4.0);
  Serial.println(amperage);
  // Serial.print("\t");
  Serial.println(AvgV);
  readings.voltage = fmap(AvgV, 0.0, 4096.0 * (8.6 / (5.0 * 3.3)), 0.0, 8.6);
  Serial.println(voltage);

  return readings;
}


// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// Scan for slaves in AP mode
void ScanForSlave() {
  int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, CHANNEL);  // Scan only on one channel
  // reset on each scan
  bool slaveFound = 0;
  memset(&slave, 0, sizeof(slave));

  Serial.println("");
  if (scanResults == 0) {
    Serial.println("No WiFi devices in AP Mode found");

  } else {
    digitalWrite(LED_RED, HIGH);
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      if (PRINTSCANRESULTS) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      }
      delay(10);
      // Check if the current device starts with `Slave`
      if (SSID.indexOf(SLAVE) == 0) {
        // SSID of interest
        Serial.println("Found a Slave.");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" [");
        Serial.print(BSSIDstr);
        Serial.print("]");
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
        // Get BSSID => Mac Address of the Slave
        int mac[6];
        if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])) {
          for (int ii = 0; ii < 6; ++ii) {
            slave.peer_addr[ii] = (uint8_t)mac[ii];
          }
        }

        slave.channel = CHANNEL;  // pick a channel
        slave.encrypt = 0;        // no encryption

        slaveFound = 1;
        // we are planning to have only one slave in this example;
        // Hence, break after we find one, to be a bit efficient
        break;
      }
    }
  }

  if (slaveFound) {
    Serial.println("Slave Found, processing..");
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_RED, 1);
  } else {
    digitalWrite(LED_BLUE, 1);
    digitalWrite(LED_RED, LOW);
    Serial.println("Slave Not Found, trying again.");
  }

  // clean up ram
  WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool manageSlave() {
  if (slave.channel == CHANNEL) {

    Serial.print("Slave Status: ");
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(slave.peer_addr);
    if (exists) {
      // Slave already paired.
      Serial.println("Already Paired");
      return true;
    } else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&slave);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
      } else {
        Serial.println("Not sure what wwwhappened");
        return false;
      }
    }
  } else {
    // No slave found to process
    Serial.println("No Slave found to process");
    return false;
  }
}

Packet toSend[10];
int idx = 0;

// send data
void sendData() {

  if (idx >= 10) { idx = 0; }
  digitalWrite(LED_GREEN, LOW);
  Packet readings = getReadings();
  digitalWrite(LED_GREEN, HIGH);

  toSend[idx] = readings;

  const uint8_t *peer_addr = slave.peer_addr;
  esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&toSend, sizeof(toSend));
  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success");
    idx = 0;
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  } else {
  }
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  delay(1000);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.print(SELF);
  Serial.println("aster");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL ");
  Serial.println(WiFi.channel());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  // In the loop we scan for slave
  ScanForSlave();
  // If Slave is found, it would be populate in `slave` variable
  // We will check if `slave` is defined and then we proceed further
  if (slave.channel == CHANNEL) {  // check if slave channel is defined
    // `slave` is defined
    // Add slave as peer if it has not been added already
    bool isPaired = manageSlave();
    if (isPaired) {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_BUILTIN, LOW);
      // pair success or already paired
      // Send data to device
      sendData();
    } else {
      idx += 1;
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_BLUE, HIGH);

      // slave pair failed
      Serial.println("Slave pair failed!");
    }
  } else {
    // No slave found to process
  }

  // wait for 3seconds to run the logic again
  delay(1000);
}

