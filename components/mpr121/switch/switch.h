#pragma once

#include "esphome/components/switch/switch.h"

namespace esphome {
namespace mpr121 {

class MPR121Switch : public switch_::Switch {
	friend class MPR121Component;

	protected:
		uint8_t output_{0};
		bool high_side_{};
		bool low_side_{};

	public:
		void set_output(uint8_t channel, bool high_side = false, bool low_side = false);
		void write_state(bool state) override;
		void process(uint8_t data);


};

}
}

