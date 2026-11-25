#pragma once

#include "esphome/core/component.h"
#include "esphome/components/adc/adc_sensor.h"

namespace esphome {
  namespace zmpt101b {

    #define ADC_SCALE 4095
    #define VREF 3.3

    class ZMPT101BSensor : public sensor::Sensor, public Component {
    public:
      void set_adc_sensor(adc::ADCSensor *sensor) { adc_sensor_ = sensor; }
      void set_frequency(uint16_t freq) { period_ = 1000000UL / freq; }
      void set_sensitivity(float sens) { sensitivity_ = sens; }
      void set_loop_count(float sens) { loop_count_ = sens; }
      void set_read_interval(uint32_t ms) { read_interval_ = ms; }

      void setup() override {}
      void loop() override;

    protected:
      adc::ADCSensor *adc_sensor_;
      uint32_t period_;
      float sensitivity_;
      int loop_count_;
      uint32_t read_interval_{0};

      uint32_t last_read_{0};
      double voltage_sum_{0.0};
      int readings_count_{0};

      int getZeroPoint_();
    };
  }
}