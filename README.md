# ESPHome ZMPT101B Sensor

## Introduction

This is an ESPHome external component for measuring AC RMS voltage using the ZMPT101B voltage sensor module.

This component reads the analog output of the ZMPT101B module through a defined ADC sensor and computes the true RMS voltage over one or more AC cycles.
It’s ideal for real-time monitoring of your mains power with ESP32 or ESP8266-based smart devices.

Initial source from [Abdurraziq/ZMPT101B-arduino](https://github.com/Abdurraziq/ZMPT101B-arduino), inspired by [rafal83/zmpt101b](https://github.com/rafal83/zmpt101b)

Note: It works perfectly with both the Arduino and ESP-IDF frameworks.

![AC measurement](capture.png)

## Usage

```
external_components:
  - source: github://vgudzelyak/esphome-zmpt101b@main
    components: [zmpt101b]

sensor:
  # First, you need to declare an adc platform
  - platform: adc
    pin: GPIO36
    id: zmpt_adc
    attenuation: 12db
    update_interval: never    # Prevent default polling, let zmpt101b handle it
    internal: true            # Hide raw ADC reading from Home Assistant

  # And then reference the previously declared adc platform in adc_id
  - platform: zmpt101b
    name: "${friendly_name} AC Voltage RMS"
    adc_id: zmpt_adc
    frequency: 50
    sensitivity: 8.36
```

### Complete examples

Ready-to-adapt configurations for an ESP32-C3 board using the ESP-IDF framework:

| Example | Description | ADC pins |
| --- | --- | --- |
| [1_phase_example.yaml](1_phase_example.yaml) | Single-phase voltage monitor (one ZMPT101B module) | GPIO1 |
| [3_phase_example.yaml](3_phase_example.yaml) | 3-phase voltage monitor (three ZMPT101B modules, one per phase) | GPIO1, GPIO3, GPIO4 |

Before flashing, adjust the board and pins to your hardware and define these entries in your `secrets.yaml`:
`voltage_monitor_api_key`, `wifi_ssid`, `wifi_password`, `web_server_username`, `web_server_password`.

After flashing, calibrate `sensitivity` for each module (see [How to calibrate sensitivity](#how-to-calibrate-sensitivity)).

### Why declare the ADC sensor separately?

The zmpt101b sensor does not configure the analog pin itself.
Instead, it uses an existing adc sensor declared in the YAML via adc_id:.
This approach ensures maximum compatibility and avoids conflicts by ensures only one
component manages the ADC hardware (a limitation on ESP32/ESP8266).

### Reducing CPU load

One reading keeps the CPU busy for two AC periods (~40 ms at 50 Hz), and by default readings run back to back.
With several sensors or a single-core board (e.g. ESP32-C3), slow them down with `read_interval`:

```
  - platform: zmpt101b
    # ...
    read_interval: 6s   # Time between the starts of two readings (default: 0ms, continuous)
    loop_count: 10      # Readings averaged per published value (default: 50)
```

* CPU share ≈ number of sensors × 40 ms / `read_interval` (3 sensors at 1s: ~12%)
* A value is published every `read_interval` × `loop_count` (here: 60s)

The pause is non-blocking, so Wi-Fi, the API and the web server keep running during it.

### How to calibrate sensitivity

First, your sensor need to be calibrated according to [Abdurraziq/ZMPT101B-arduino#steep](https://github.com/Abdurraziq/ZMPT101B-arduino#steep)

The sensitivity value determines how the analog signal is converted into the real AC voltage.
It depends on your specific ZMPT101B module, hardware, and ADC settings (e.g., attenuation, voltage reference).

To calibrate it:

1. Start with a value around 8.0 to 10.0.
2. Compare the reported voltage in Home Assistant with a trusted multimeter measurement.
3. Adjust the sensitivity using this formula:

sensitivitycorrected = sensitivitycurrent × (Vreported / Vactual)

#### Example:

* You set sensitivity: 8.0
* ESPHome shows 270 V
* Your multimeter reads 230 V

Then:

sensitivitycorrected = 8.0 × (270 / 230) = 9.39

Update the YAML with the new value and repeat until the readings are accurate (±1V is acceptable).