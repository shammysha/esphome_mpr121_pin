#include "switch.h"
#include "../mpr121.h"

namespace esphome {
namespace mpr121 {

void MPR121Switch::set_output(uint8_t channel, bool high_side, bool low_side) {
	output_ = channel;
	high_side_ = high_side;
	low_side_ = low_side;
}
void MPR121Switch::write_state(bool state) {
	if (state) {
		MPR121Component::set_output(this->output_));
	} else {
		MPR121Component::clear_output(this->output_));
	}

	publish_state(this->output_, state);
}
void MPR121Switch::process(uint8_t data) { this->publish_state(static_cast<bool>((data>>(this->input_-4))&1)); }


}	// namespace mpr121
}	// namespace esphome
