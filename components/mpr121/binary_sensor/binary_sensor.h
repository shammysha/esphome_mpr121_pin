#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace mpr121 {

class MPR121Channel : public binary_sensor::BinarySensor {
	friend class MPR121Component;
	protected:
		uint8_t channel_{0};
		optional<uint8_t> touch_threshold_{};
		optional<uint8_t> release_threshold_{};

		uint8_t input_{0};
		bool pull_up_{};
		bool pull_down_{};

	public:
		void set_channel(uint8_t channel);
		void set_input(uint8_t channel, bool pull_up = false, bool pull_down = false);
		void process_channel(uint16_t data);
		void process_input(uint8_t data);

		void set_touch_threshold(uint8_t touch_threshold);
		void set_release_threshold(uint8_t release_threshold);
};

}
}

