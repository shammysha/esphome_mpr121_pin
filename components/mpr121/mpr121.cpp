#include "mpr121.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mpr121 {

static const char *const TAG = "mpr121";

void MPR121Channel::set_channel(uint8_t channel) {
	channel_ = channel;
}

void MPR121Channel::set_input(uint8_t channel, bool pull_up, bool pull_down) {
	input_ = channel;
	pull_up_ = pull_up;
	pull_down_ = pull_down;
}

void MPR121Channel::process_channel(uint16_t data) {
	this->publish_state(static_cast<bool>(data & (1 << this->channel_)));
}

void MPR121Channel::process_input(uint8_t data) {
	this->publish_state(static_cast<bool>((data>>(this->input_-4))&1) == 1));
}

void MPR121Channel::set_touch_threshold(uint8_t touch_threshold) {
	this->touch_threshold_ = touch_threshold;
}

void MPR121Channel::set_release_threshold(uint8_t release_threshold) {
	this->release_threshold_ = release_threshold;
}


void MPR121Switch::set_output(uint8_t channel, bool high_side, bool low_side) {
	output_ = channel;
	high_side_ = high_side;
	low_side_ = low_side;

	this->turn_off();
}

void MPR121Switch::write_state(bool state) {
	if (state) {
		this->parent_->set_output(this->output_);
	} else {
		this->parent_->clear_output(this->output_);
	}

	publish_state(state);
}

void MPR121Switch::set_parent(MPR121Component *parent) {
	this->parent_ = parent;
}


void MPR121Component::setup() {
	uint8_t data = 0;

	ESP_LOGCONFIG(TAG, "Setting up MPR121...");

	// soft reset device
	this->write_byte(MPR121_SOFTRESET, 0x63);
	delay(100);	// NOLINT
	if (!this->write_byte(MPR121_ECR, 0x0)) {
		this->error_code_ = COMMUNICATION_FAILED;
		this->mark_failed();
		return;
	}

	// set touch sensitivity for all channels
	for (auto *channel : this->channels_) {
		ESP_LOGI(TAG, "Channel %d'", channel->channel_);
		this->write_byte(
			MPR121_TOUCHTH_0 + 2 * channel->channel_,
			channel->touch_threshold_.value_or(this->touch_threshold_)
		);
		this->write_byte(
			MPR121_RELEASETH_0 + 2 * channel->channel_,
			channel->release_threshold_.value_or(this->release_threshold_)
		);
	}

	// set touch sensitivity for all channels
	for (auto *input : this->inputs_) {
		ESP_LOGI(TAG, "Channel %d'", input->input_);
		uint8_t bitmask = 1<<(input->input_-4);

		this->read_byte(MPR121_GPIOEN, &data);
		this->write_byte(MPR121_GPIOEN, data | bitmask);
		this->read_byte(MPR121_GPIODIR, &data);
		this->write_byte(MPR121_GPIODIR, data &~ bitmask);

		if (input->pull_up_) {
			this->read_byte(MPR121_GPIOCTL0, &data);
			this->write_byte(MPR121_GPIOCTL0, data | bitmask);
			this->read_byte(MPR121_GPIOCTL1, &data);
			this->write_byte(MPR121_GPIOCTL1, data | bitmask);
		} else if (input->pull_down_) {
			this->read_byte(MPR121_GPIOCTL0, &data);
			this->write_byte(MPR121_GPIOCTL0, data | bitmask);
			this->read_byte(MPR121_GPIOCTL1, &data);
			this->write_byte(MPR121_GPIOCTL1, data & ~bitmask);
		} else {
			this->read_byte(MPR121_GPIOCTL0, &data);
			this->write_byte(MPR121_GPIOCTL0, data & ~bitmask);
			this->read_byte(MPR121_GPIOCTL1, &data);
			this->write_byte(MPR121_GPIOCTL1, data & ~bitmask);
		}
	}

	this->write_byte(MPR121_MHDR, 0x01);
	this->write_byte(MPR121_NHDR, 0x01);
	this->write_byte(MPR121_NCLR, 0x0E);
	this->write_byte(MPR121_FDLR, 0x00);

	this->write_byte(MPR121_MHDF, 0x01);
	this->write_byte(MPR121_NHDF, 0x05);
	this->write_byte(MPR121_NCLF, 0x01);
	this->write_byte(MPR121_FDLF, 0x00);

	this->write_byte(MPR121_NHDT, 0x00);
	this->write_byte(MPR121_NCLT, 0x00);
	this->write_byte(MPR121_FDLT, 0x00);

	this->write_byte(MPR121_DEBOUNCE, 0);
	// default, 16uA charge current
	this->write_byte(MPR121_CONFIG1, 0x10);
	// 0.5uS encoding, 1ms period
	this->write_byte(MPR121_CONFIG2, 0x20);
	// start with first 5 bits of baseline tracking
	this->write_byte(MPR121_ECR, 0x8F);
}

void MPR121Component::set_touch_debounce(uint8_t debounce) {
	uint8_t mask = debounce << 4;
	this->debounce_ &= 0x0f;
	this->debounce_ |= mask;
}

void MPR121Component::set_release_debounce(uint8_t debounce) {
	uint8_t mask = debounce & 0x0f;
	this->debounce_ &= 0xf0;
	this->debounce_ |= mask;
};

void MPR121Component::dump_config() {
	ESP_LOGCONFIG(TAG, "MPR121:");
	LOG_I2C_DEVICE(this);
	switch (this->error_code_) {
		case COMMUNICATION_FAILED:
			ESP_LOGE(TAG, "Communication with MPR121 failed!");
			break;
		case WRONG_CHIP_STATE:
			ESP_LOGE(TAG, "MPR121 has wrong default value for CONFIG2?");
			break;
		case NONE:
		default:
			break;
	}
//	uint8_t data = 0;
//	this->read_byte(MPR121_GPIOEN, &data);
//	for (int i=0;i<12;i++) {
//		bool bit =  data & (1 << i);
//		if (bit) {
//			ESP_LOGCONFIG(TAG, "   PIN %d set as GPIO", i);
//		} else {
//			ESP_LOGCONFIG(TAG, "   PIN %d set as TOUCH", i);
//		}
//	}
}
void MPR121Component::loop() {
	uint16_t val = 0;
	this->read_byte_16(MPR121_TOUCHSTATUS_L, &val);

	// Flip order
	uint8_t lsb = val >> 8;
	uint8_t msb = val;
	val = (uint16_t(msb) << 8) | lsb;

	for (auto *channel : this->channels_) {
		channel->process_channel(val);
	}

	this->read_byte(MPR121_GPIODATA, &lsb);
	for (auto *input : this->inputs_) {
		input->process_input(lsb);
	}
}

void MPR121Component::register_channel(MPR121Channel *channel) {
	this->channels_.push_back(channel);
}

void MPR121Component::register_input(MPR121Channel *input) {
	this->inputs_.push_back(input);
}


void MPR121Component::set_touch_threshold(uint8_t touch_threshold) {
	this->touch_threshold_ = touch_threshold;
};

void MPR121Component::set_release_threshold(uint8_t release_threshold) {
	this->release_threshold_ = release_threshold;
};

void MPR121Component::set_output(uint8_t channel) {
	bool res = this->write_byte(MPR121_GPIOSET, 1<<(channel-4));
	ESP_LOGI(TAG, "Result of set: %s'", res);
}

void MPR121Component::clear_output(uint8_t channel) {
	bool res = this->write_byte(MPR121_GPIOCLR, 1<<(channel-4));
	ESP_LOGI(TAG, "Result of unset: %s'", res);
}

uint8_t MPR121Component::get_touch_threshold() {
	return this->touch_threshold_;
};

uint8_t MPR121Component::get_release_threshold() {
	return this->release_threshold_;
};

float MPR121Component::get_setup_priority() const {
	return setup_priority::DATA;
}


}	// namespace mpr121
}	// namespace esphome
