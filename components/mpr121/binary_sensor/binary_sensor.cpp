#include "binary_sensor.h"

namespace esphome {
namespace mpr121 {

void set_channel(uint8_t channel) {
	channel_ = channel;
}

void set_input(uint8_t channel, bool pull_up = false, bool pull_down = false) {
	input_ = channel;
	pull_up_ = pull_up;
	pull_down_ = pull_down;
}

void process_channel(uint16_t data) {
	this->publish_state(static_cast<bool>(data & (1 << this->channel_)));
}

void process_input(uint8_t data) {
	this->publish_state(static_cast<bool>((data>>(this->input_-4))&1));
}

void set_touch_threshold(uint8_t touch_threshold) {
	this->touch_threshold_ = touch_threshold;
}

void set_release_threshold(uint8_t release_threshold) {
	this->release_threshold_ = release_threshold;
}

}
}
