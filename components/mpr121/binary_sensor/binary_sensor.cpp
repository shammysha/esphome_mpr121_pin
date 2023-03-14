#include "binary_sensor.h"

namespace esphome {
namespace mpr121 {

void MPR121Channel::set_channel(uint8_t channel) {
	channel_ = channel;
}

void MPR121Channel::set_input(uint8_t channel, bool pull_up = false, bool pull_down = false) {
	input_ = channel;
	pull_up_ = pull_up;
	pull_down_ = pull_down;
}

void MPR121Channel::process_channel(uint16_t data) {
	this->publish_state(static_cast<bool>(data & (1 << this->channel_)));
}

void MPR121Channel::process_input(uint8_t data) {
	this->publish_state(static_cast<bool>((data>>(this->input_-4))&1));
}

void MPR121Channel::set_touch_threshold(uint8_t touch_threshold) {
	this->touch_threshold_ = touch_threshold;
}

void MPR121Channel::set_release_threshold(uint8_t release_threshold) {
	this->release_threshold_ = release_threshold;
}

}
}
