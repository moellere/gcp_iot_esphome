/**
 * gcpiot.cpp
 *
 * Implementation of gcp_iot_esphome
 *
 * Author: Enoch Moeller <enoch@tweektech.com>
 * Last Updated: 2021-12-18
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Requirements:
 * - https://github.com/moellere/gcp_iot_esphome
 * - ESPHome 1.19.1 or greater
 */

#include "gcpiot.h"
using namespace esphome;

/**
 * Create a new GCPIoT Esphome object
 *
 * Args:
 *   project_id: id of the GCP project where the registry exists
 *   location: 
 *   registry_id: the id of the IoT Core registry
 *   device_id: the id of the device itself
 *   private_key: 
 *     To get the private key run (where private-key.pem is the ec private key
 *     used to create the certificate uploaded to google cloud iot):
 *     openssl ec -in <private-key.pem> -noout -text
 *     and copy priv: part.
 *     The key length should be exactly the same as the key length bellow (32 pairs
 *     of hex digits). If it's bigger and it starts with "00:" delete the "00:". If
 *     it's smaller add "00:" to the start. If it's too big or too small something
 *     is probably wrong with your key.
 *   poll_interval: polling interval in milliseconds
 *   primary_ca: certificate for primary GCP CA
 *   backup_ca: certificate for backup GCP CA
 */
GCPIoTEsp::GCPIoTEsp(
        char* project_id,
        char* location,
        char* registry_id,
        char* device_id,
        unsigned char private_key[],
        uint32_t poll_interval,
        const char* primary_ca,
        const char* backup_ca
) :
    PollingComponent{poll_interval}, // member initializers list
    hw_serial_{hw_serial}
{
    this->traits_.set_supports_action(true);
    this->traits_.set_supports_current_temperature(true);
}


void GCPIoTEsp::update() {
    // This will be called every "update_interval" milliseconds.
    //this->dump_config();
    this->hp->sync();
#ifndef USE_CALLBACKS
    this->hpSettingsChanged();
    heatpumpStatus currentStatus = hp->getStatus();
    this->hpStatusChanged(currentStatus);
#endif
}

/**
 * Implement control of a GCP IoT Esphome device.
 *
 * Maps HomeAssistant/ESPHome modes
 */
void GCPIoTEsp::control(const char* &call) {
    ESP_LOGV(TAG, "Control called.");

    ESP_LOGD(TAG, "control - Was IoT Esp device updated? %s", YESNO(updated));

    // send the update back to esphome:
    this->publish_state();
}

void GCPIoTEsp::setup() {
    // This will be called by App.setup()
    this->banner();

    ESP_LOGCONFIG(TAG, "Intializing new IoT Core connection object.");
    this->hp = new HeatPump();
    this->current_temperature = NAN;
    this->target_temperature = NAN;
    this->fan_mode = climate::CLIMATE_FAN_OFF;
    this->swing_mode = climate::CLIMATE_SWING_OFF;

#ifdef USE_CALLBACKS
    hp->setSettingsChangedCallback(
            [this]() {
                this->hpSettingsChanged();
            }
    );

    hp->setStatusChangedCallback(
            [this](heatpumpStatus currentStatus) {
                this->hpStatusChanged(currentStatus);
            }
    );
#endif

    ESP_LOGCONFIG(
            TAG,
            "hw_serial(%p) is &Serial(%p)? %s",
            this->get_hw_serial_(),
            &Serial,
            YESNO(this->get_hw_serial_() == &Serial)
    );

    ESP_LOGCONFIG(TAG, "Calling hp->connect(%p)", this->get_hw_serial_());

    if (hp->connect(this->get_hw_serial_(), this->baud_)) {
        hp->sync();
    }
    else {
        ESP_LOGCONFIG(
                TAG,
                "Connection to HeatPump failed."
                " Marking MitsubishiHeatPump component as failed."
        );
        this->mark_failed();
    }

    // create various setpoint persistence:
    cool_storage = global_preferences->make_preference<uint8_t>(this->get_object_id_hash() + 1);
    heat_storage = global_preferences->make_preference<uint8_t>(this->get_object_id_hash() + 2);
    auto_storage = global_preferences->make_preference<uint8_t>(this->get_object_id_hash() + 3);

    // load values from storage:
    cool_setpoint = load(cool_storage);
    heat_setpoint = load(heat_storage);
    auto_setpoint = load(auto_storage);

    this->dump_config();
}

void GCPIoTEsp::dump_config() {
    this->banner();
    ESP_LOGI(TAG, "  Supports HEAT: %s", YESNO(true));
    ESP_LOGI(TAG, "  Supports COOL: %s", YESNO(true));
    ESP_LOGI(TAG, "  Supports AWAY mode: %s", YESNO(false));
    ESP_LOGI(TAG, "  Saved heat: %.1f", heat_setpoint.value_or(-1));
    ESP_LOGI(TAG, "  Saved cool: %.1f", cool_setpoint.value_or(-1));
    ESP_LOGI(TAG, "  Saved auto: %.1f", auto_setpoint.value_or(-1));
}

void GCPIoTEsp::dump_state() {
    ESP_LOGI(TAG, "HELLO");
}

// !!REPLACEME!!
// The MQTT callback function for commands and configuration updates
// Place your message handler code here.
void messageReceivedAdvanced(MQTTClient *client, char topic[], char bytes[], int length)
{
  ESP_LOGI(TAG, " Incoming Topic: %s", topic);
  if (length > 0)// On message
  {
    ESP_LOGI(TAG, " \n\r   Data: %s", bytes);
  }
}
///////////////////////////////

// Initialize WiFi and MQTT for this board
static MQTTClient *mqttClient;
static BearSSL::WiFiClientSecure netClient;
static BearSSL::X509List certList;
static CloudIoTCoreDevice device(project_id, location, registry_id, device_id);
CloudIoTCoreMqtt *mqtt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////
String getDefaultSensor()
{
  return "Wifi: " + String(WiFi.RSSI()) + "db";
}

String getJwt()
{
  // Disable software watchdog as these operations can take a while.
  ESP.wdtDisable();
  time_t iat = time(nullptr);
  ESP_LOGI(TAG, "Refreshing JWT");
  String jwt = device.createJWT(iat, jwt_exp_secs);
  ESP.wdtEnable(0);
  return jwt;
}

static void readDerCert(const char *filename) {
  File ca = SPIFFS.open(filename, "r");
  if (ca)
  {
    size_t size = ca.size();
    uint8_t cert[size];
    ca.read(cert, size);
    certList.append(cert, size);
    ca.close();

    ESP_LOGI(TAG, " Success to open ca file ");
  }
  else
  {
    ESP_LOGE(TAG, " Failed to open ca file ");
  }
  ESP_LOGI(TAG, " Filename: %s", filename);
}

static void setupCertAndPrivateKey()
{
  // Set CA cert on wifi client
  // If using a static (pem) cert, uncomment in ciotc_config.h:
  certList.append(primary_ca);
  certList.append(backup_ca);
  netClient.setTrustAnchors(&certList);

  device.setPrivateKey(private_key);
  return;

  // If using the (preferred) method with the cert and private key in /data (SPIFFS)
  // To get the private key run
  // openssl ec -in <private-key.pem> -outform DER -out private-key.der

  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, " Failed to mount file system");
    return;
  }

  readDerCert("/gtsltsr.crt"); // primary_ca.pem
  readDerCert("/GSR4.crt"); // backup_ca.pem
  netClient.setTrustAnchors(&certList);


  File f = SPIFFS.open("/private-key.der", "r");
  if (f) {
    size_t size = f.size();
    uint8_t data[size];
    f.read(data, size);
    f.close();

    BearSSL::PrivateKey pk(data, size);
    device.setPrivateKey(pk.getEC()->x);

    ESP_LOGI(TAG, " Success to open private-key.der");
  } else {
    ESP_LOGE(TAG, " Failed to open private-key.der");
  }

  SPIFFS.end();
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(const String &data)
{
  ESP_LOGI(TAG, " Outcoming: %s", data);
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length)
{
  return mqtt->publishTelemetry(data, length);
}

// TODO: fix globals
void setupCloudIoT()
{

  // ESP8266 WiFi secure initialization and device private key
  setupCertAndPrivateKey();

  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, &netClient, &device);
  mqtt->setUseLts(true);
  mqtt->startMQTTAdvanced(); // Opens connection using advanced callback
}