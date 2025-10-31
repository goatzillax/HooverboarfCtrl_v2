
#define USE_OLED
#define USE_CRSF
#define USE_FAN

//  mlemetry would be spiffy neato but i r srsly cornstrained on pins
//  ESP32-C3 pins btw
//  crossfire pins
#define SERIAL1_RX 0
#define SERIAL1_TX -1

//  hooverboarf pins
#define SERIAL2_RX 20
#define SERIAL2_TX 21

//  without getting into linker scripts this seems like the easiest way.
//  unfortunately the indices are worthless if the declaration doesn't follow exactly, and perhaps not even that.
//  probably should just make a rule to never use an index in a specific manner

enum modules {
#ifdef USE_CRSF
	MODULE_CRSF,
#endif
#ifdef USE_OLED
	MODULE_OLED,
#endif
	MODULE_CNT
};

unsigned long print_last = 0;

////////////////////////////////////////////////////////////////////////////////
//  mower stuff

enum mower_states {
	MOWER_DISARMED,
	MOWER_ARMED
};

#define POWER_PULSE_LEN 100


////////////////////////////////////////////////////////////////////////////////
//  hooverboarf stuff

#include <CircularBuffer.hpp>

//  hooverboarf serial stats
unsigned long hover_csum_fails=0;
unsigned long hover_ser_rcvd=0;
unsigned long hover_ser_frame_rcvd=0;
unsigned long hover_ser_frame_last=0;

struct serial_cmd_s {
  uint16_t start = 0xabcd;
  int16_t  steer = 0;   // -1000 to 1000
  int16_t  speed = 0;   // -1000 to 1000
  uint16_t csum;
} hover_cmd;

//  Honestly if we manage this correctly we only need one frame.
CircularBuffer<byte,9*2> feedback_circbuf;

enum feedback_states_e {
	FEEDBACK_SEARCH,
	FEEDBACK_FOUND
} feedback_state = FEEDBACK_SEARCH;




////////////////////////////////////////////////////////////////////////////////
//  I guess fan is a module
#ifdef USE_FAN

//  need GPIO
#define GPIO_FAN -1

void setup_fan() {
}

void loop_fan() {
}



#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef USE_OLED
//  default pins appear correct for shield
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
//  nice butt wemos no longer makes these n the knockoffs don't even have buttons
//#include <LOLIN_I2C_BUTTON.h>
//I2C_BUTTON oled_button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C Address 0x31
//unsigned long last_button=0;

unsigned long display_last = 0;

void dump_oled() {
	display.clearDisplay();
	display.setCursor(0,0);
	//  probably do a thing like raw input first line
	//  hover cmd second line
	
}

void setup_oled() {
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.clearDisplay();
	display.setTextSize(1);  //  im old and don't see to good anymore
	display.setTextColor(WHITE);
	display.display();  //  hurr durrrrr
}

void loop_oled() {
	//"%s%04d,%04d%s\n", "C", CH0, CH1, deadman
	//"H%04d,%04d%s\n", steer, velocity, armed
	//temperature vbat
	//IP
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef USE_CRSF
#include "CRSFforArduino.hpp"

CRSFforArduino *crsf = nullptr;

int rcChannelCount = crsfProtocol::RC_CHANNEL_COUNT;

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData);

const char *rcChannelNames[] = {
        "T",
        "A",
        "E",
        "R",
        "AUX1",
        "AUX2",
        "AUX3",
        "AUX4",
        "AUX5",
        "AUX6",
        "AUX7",
        "AUX8",
        "AUX9",
        "AUX10",
        "AUX11",
        "AUX12"
};

// protocal value is 11bits so likely 0-2047, with 1023 being middle.  will czech.
// the lib can turn this into ms but the full 11bit seems better.  funny bit of history, the
// OG rc proportional PWM protocol was actually digital.  fight me, bro.

//  prlly stuff this in the header
uint16_t crsf_values_raw[crsfProtocol::RC_CHANNEL_COUNT];  //  not totally raw but mostly raw.  kind of like a ceviche.
uint16_t crsf_fs_values[crsfProtocol::RC_CHANNEL_COUNT];   //  AUX1 should be LOW during FS.

#define CRSF_MIN	0
#define CRSF_MAX	2047
#define CRSF_CENTER	1023

struct crsf_channel_cfg_s {
	uint16_t min;
	uint16_t max;
	uint16_t center;
	uint16_t deadband;  //  symmetric around center
};

crsf_channel_cfg_s crsf_channel_cfg[crsfProtocol::RC_CHANNEL_COUNT];  //  read from config

enum crsf_state_e {
	CRSF_ACTIVE,
	CRSF_FAILSAFE,
};

crsf_state_e crsf_state = CRSF_ACTIVE;

uint16_t adjust_value(uint16_t value, struct crsf_channel_cfg_s cfg) {
	//  probably should have a switch to enable totaly raw mode for debug
#if 0
	//  deadband optimally is applied before any expo.  i.e. raw value of 6 with a deadband should turn into a raw value of 1.
	if ( (cfg.center-deadband) < value < (cfg.center+cfg.deadband) ) {
		return(CRSF_CENTER);
	}
	//  deadband values have been dealt with, so do other ranges
	//  FIXME
	if (value > cfg.center) {
		value = value - cfg.deadband;
		//  need to map value
	} else {
		value = value + cfg.deadband;
		//  need to map value
	}
#endif
	return(value);
}

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData) {
        unsigned long now = millis();

        if (now - print_last > 1000) {
                print_last = now;
                //  print stuff
        }

	//  not sure what exactly is atomic in this arch.

	if (rcData->failsafe) {
		if (crsf_state != CRSF_FAILSAFE) {
			crsf_state = CRSF_FAILSAFE;

			for (int i=1; i<crsfProtocol::RC_CHANNEL_COUNT; i++) {
				rcData->value[i] = crsf_values_raw[i] = crsf_fs_values[i];
			}
		}  //  transition to failsafe
	}
	else {
		if (crsf_state == CRSF_FAILSAFE) {
			crsf_state = CRSF_ACTIVE;
		}  //  transition to active

		for (int i=0; i<crsfProtocol::RC_CHANNEL_COUNT; i++) {
			crsf_values_raw[i] = adjust_value(rcData->value[i], crsf_channel_cfg[i]);

		}

	}


}

void setup_crsf() {
        crsf = new CRSFforArduino(&Serial1, SERIAL1_RX, SERIAL1_TX);
        if (!crsf->begin()) {
                crsf->end();
                crsf = nullptr;
                Serial.println("CRSF for Arduino initialization failed");
                while (1) {
                        delay(100);
                }
        }

        rcChannelCount = rcChannelCount > crsfProtocol::RC_CHANNEL_COUNT ? crsfProtocol::RC_CHANNEL_COUNT : rcChannelCount;

        crsf->setRcChannelsCallback(onReceiveRcChannels);
}

void loop_crsf() {
        crsf->update();
}
#endif

////////////////////////////////////////////////////////////////////////////////


void (*setup_array[MODULE_CNT])(void) = {
#ifdef USE_CRSF
	setup_crsf,
#endif
#ifdef USE_OLED
	setup_oled,
#endif
};

void (*loop_array[MODULE_CNT])(void) {
#ifdef USE_CRSF
	loop_crsf,
#endif
#ifdef USE_OLED
	loop_oled,
#endif
};


void setup() {
        Serial.begin(115200);
        while (!Serial) {
		delay(1000);
	}

	//  walk through setup array
	for (int i=0; i<MODULE_CNT; i++) {
		setup_array[i]();   //  can't count on order
	}
}

void loop() {
	//  walk through loop array
	for (int i=0; i<MODULE_CNT; i++) {
		loop_array[i]();   //  can't count on order
	}

}



