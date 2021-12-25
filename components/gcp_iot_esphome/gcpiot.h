/**
 * gcpiot.h
 *
 * Header file for gcp_iot_esphome 
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

#define USE_CALLBACKS

#include "esphome.h"
#include "esphome/core/preferences.h"

// You need to set certificates to All SSL cyphers and you may need to
// increase memory settings in Arduino/cores/esp8266/StackThunk.cpp:
//   https://github.com/esp8266/Arduino/issues/6811
#include "WiFiClientSecureBearSSL.h"
#include <time.h>

#include <MQTT.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>

using namespace esphome;

static const char* TAG = "GCPIoTCoreESPHome"; // Logging tag

static const char* GCPIOT_VERSION = "0.1.0";


/* If polling interval is greater than 9 seconds, the GCP_IoT
library reconnects, but doesn't then follow up with our data request.*/
static const uint32_t GCPIOT_POLL_INTERVAL_DEFAULT = 500; // in milliseconds,
                                                           // 0 < X <= 9000

// Time (seconds) to expire token += 20 minutes for drift
const int jwt_exp_secs = 3600; // Maximum 24H (3600*24)

class GCPIoTEsp : public PollingComponent {

    public:

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
        GCPIoTEsp(
            char* project_id,
            char* location,
            char* registry_id,
            char* device_id,
            unsigned char private_key[],
            uint32_t poll_interval,
            const char* primary_ca,
            const char* backup_ca
        );

        // Print a banner with library information.
        void banner() {
            ESP_LOGI(TAG, "GCP IoT ESPHome version %s",
                    GCPIOT_VERSION);
        }

        void GCPIoTEsp::control(const char* &call);

        // Set up the component, initializing the HeatPump object.
        void setup() override;

        // This is called every poll_interval.
        void update() override;

        // Dump the config state to log
        void dump_config() override;

        // Dump the state to log
        void dump_state() override;

        // The MQTT callback function for commands and configuration updates
        // Place your message handler code here.
        void messageReceivedAdvanced(MQTTClient *client, char topic[], char bytes[], int length)

        // Get a default sensor value to send as telemetry
        String getDefaultSensor()

        // Generate JWT
        String getJwt()

        // Retrieve certs from files in spifs
        static void readDerCert(const char *filename)

        // Set CA cert on wifi client        
        static void setupCertAndPrivateKey()

        // Publish telemetry as a string
        bool publishTelemetry(const String &data)

        // Publish telemetry as a char array
        bool publishTelemetry(const char *data, int length)

        // Setup IoT Core certs and MQTT connection
        void setupCloudIoT()

    protected:

};

