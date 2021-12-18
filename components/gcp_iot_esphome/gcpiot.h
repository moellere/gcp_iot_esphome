/**
 * gcpiot.h
 *
 * Header file for gcp_iot_esphome 
 *
 * Author: Enoch Moeller <enoch@tweektech.com>
 * Last Updated: 2021-12-18
 * License: MIT
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

static const char* GCPIOT_VERSION = "1.0.0";

/* If polling interval is greater than 9 seconds, the GCP_IoT
library reconnects, but doesn't then follow up with our data request.*/
static const uint32_t GCPIOT_POLL_INTERVAL_DEFAULT = 500; // in milliseconds,
                                                           // 0 < X <= 9000

class GCPIoTEsp : public PollingComponent {

    public:

        /**
         * Create a new GCPIoT Esphome object
         *
         * Args:
         *   hw_serial: pointer to an Arduino HardwareSerial instance
         *   poll_interval: polling interval in milliseconds
         */
        MitsubishiHeatPump(
            HardwareSerial* hw_serial,
            uint32_t poll_interval=ESPMHP_POLL_INTERVAL_DEFAULT
        );

        // Print a banner with library information.
        void banner() {
            ESP_LOGI(TAG, "ESPHome MitsubishiHeatPump version %s",
                    ESPMHP_VERSION);
        }

        // Set the baud rate. Must be called before setup() to have any effect.
        void set_baud_rate(int);

        // print the current configuration
        void dump_config() override;

        // handle a change in settings as detected by the HeatPump library.
        void hpSettingsChanged();

        // Handle a change in status as detected by the HeatPump library.
        void hpStatusChanged(heatpumpStatus currentStatus);

        // Set up the component, initializing the HeatPump object.
        void setup() override;

        // This is called every poll_interval.
        void update() override;

        // Configure the climate object with traits that we support.
        climate::ClimateTraits traits() override;

        // Get a mutable reference to the traits that we support.
        climate::ClimateTraits& config_traits();

        // Debugging function to print the object's state.
        void dump_state();

        // Handle a request from the user to change settings.
        void control(const climate::ClimateCall &call) override;

        // Use the temperature from an external sensor. Use
        // set_remote_temp(0) to switch back to the internal sensor.
        void set_remote_temperature(float);

    protected:
        // HeatPump object using the underlying Arduino library.
        HeatPump* hp;

        // The ClimateTraits supported by this HeatPump.
        climate::ClimateTraits traits_;

        // Allow the HeatPump class to use get_hw_serial_
        friend class HeatPump;

        //Accessor method for the HardwareSerial pointer
        HardwareSerial* get_hw_serial_() {
            return this->hw_serial_;
        }

        //Print a warning message if we're using the sole hardware UART on an
        //ESP8266 or UART0 on ESP32
        void check_logger_conflict_();

        // various prefs to save mode-specific temperatures, akin to how the IR
        // remote works.
        ESPPreferenceObject cool_storage;
        ESPPreferenceObject heat_storage;
        ESPPreferenceObject auto_storage;

        optional<float> cool_setpoint;
        optional<float> heat_setpoint;
        optional<float> auto_setpoint;

        static void save(float value, ESPPreferenceObject& storage);
        static optional<float> load(ESPPreferenceObject& storage);

};

