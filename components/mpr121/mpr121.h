#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"

#include <vector>

namespace esphome {
namespace mpr121 {

enum {
	MPR121_TOUCHSTATUS_L = 0x00,
	MPR121_TOUCHSTATUS_H = 0x01,
	MPR121_FILTDATA_0L = 0x04,
	MPR121_FILTDATA_0H = 0x05,
	MPR121_BASELINE_0 = 0x1E,
	MPR121_MHDR = 0x2B,
	MPR121_NHDR = 0x2C,
	MPR121_NCLR = 0x2D,
	MPR121_FDLR = 0x2E,
	MPR121_MHDF = 0x2F,
	MPR121_NHDF = 0x30,
	MPR121_NCLF = 0x31,
	MPR121_FDLF = 0x32,
	MPR121_NHDT = 0x33,
	MPR121_NCLT = 0x34,
	MPR121_FDLT = 0x35,
	MPR121_TOUCHTH_0 = 0x41,
	MPR121_RELEASETH_0 = 0x42,
	MPR121_DEBOUNCE = 0x5B,
	MPR121_CONFIG1 = 0x5C,
	MPR121_CONFIG2 = 0x5D,
	MPR121_CHARGECURR_0 = 0x5F,
	MPR121_CHARGETIME_1 = 0x6C,
	MPR121_ECR = 0x5E,
	MPR121_AUTOCONFIG0 = 0x7B,
	MPR121_AUTOCONFIG1 = 0x7C,
	MPR121_UPLIMIT = 0x7D,
	MPR121_LOWLIMIT = 0x7E,
	MPR121_TARGETLIMIT = 0x7F,
	MPR121_GPIOCTL0 = 0x73,
	MPR121_GPIOCTL1 = 0x74,
	MPR121_GPIODATA = 0x75,
	MPR121_GPIODIR = 0x76,
	MPR121_GPIOEN = 0x77,
	MPR121_GPIOSET = 0x78,
	MPR121_GPIOCLR = 0x79,
	MPR121_GPIOTOGGLE = 0x7A,
	MPR121_SOFTRESET = 0x80,
};

class MPR121Channel : public binary_sensor::BinarySensor {
	friend class MPR121Component;

	public:
		void set_channel(uint8_t channel) { channel_ = channel; }
		void set_input(uint8_t channel, bool pull_up = false, bool pull_down = false) {
			input_ = channel;
			pull_up_ = pull_up;
			pull_down_ = pull_down;
		}
		void process_channel(uint16_t data) { this->publish_state(static_cast<bool>(data & (1 << this->channel_))); }
		void process_input(uint8_t data) { this->publish_state(static_cast<bool>((data>>(this->input_-4))&1)); }

		void set_touch_threshold(uint8_t touch_threshold) { this->touch_threshold_ = touch_threshold; };
		void set_release_threshold(uint8_t release_threshold) { this->release_threshold_ = release_threshold; };

	protected:
		uint8_t channel_{0};
		optional<uint8_t> touch_threshold_{};
		optional<uint8_t> release_threshold_{};

		uint8_t input_{0};
		bool pull_up_{};
		bool pull_down_{};
};

class MPR121Switch : public switch_::Switch {
	friend class MPR121Component;

	public:
		void set_output(uint8_t channel, bool high_side = false, bool low_side = false) {
			output_ = channel;
			high_side_ = high_side;
			low_side_ = low_side;
		}
		void write_state(bool state) override {
			if (state) {
				MPR121Component::set_output(this->output_));
			} else {
				MPR121Component::clear_output(this->output_));
			}

			publish_state(this->output_, state);
		}
		void process(uint8_t data) { this->publish_state(static_cast<bool>((data>>(this->input_-4))&1)); }

	protected:
		uint8_t output_{0};
		bool high_side_{};
		bool low_side_{};
};

class MPR121Component : public Component, public i2c::I2CDevice {
	public:
		void register_channel(MPR121Channel *channel) { this->channels_.push_back(channel); }
		void register_input(MPR121Channel *input) { this->inputs_.push_back(input); }
		void register_output(MPR121Switch *output) { this->outputs_.push_back(output); }

		void write_state(uint8_t channel, ) {
			this->wite
		}

		void set_touch_debounce(uint8_t debounce);
		void set_release_debounce(uint8_t debounce);
		void set_touch_threshold(uint8_t touch_threshold) { this->touch_threshold_ = touch_threshold; };
		void set_release_threshold(uint8_t release_threshold) { this->release_threshold_ = release_threshold; };

		void set_output(uint8_t channel) { this->write(MPR121_GPIOSET, 1<<(channel-4); }
		void clear_output(uint8_t channel) { this->write(MPR121_GPIOCLR, 1<<(channel-4); }

		uint8_t get_touch_threshold() { return this->touch_threshold_; };
		uint8_t get_release_threshold() { return this->release_threshold_; };

		void setup() override;
		void dump_config() override;
		float get_setup_priority() const override { return setup_priority::DATA; }
		void loop() override;

	protected:
		std::vector<MPR121Channel *> channels_{};
		std::vector<MPR121Channel *> inputs_{};
		std::vector<MPR121Switch *> outputs_{};

		uint8_t debounce_{0};
		uint8_t touch_threshold_{};
		uint8_t release_threshold_{};

		enum ErrorCode {
			NONE = 0,
			COMMUNICATION_FAILED,
			WRONG_CHIP_STATE,
		} error_code_{NONE};
};

}	// namespace mpr121
}	// namespace esphome
