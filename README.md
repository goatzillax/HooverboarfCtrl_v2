# HooverboarfCtrl_v2

This time it's not Hooverboarf V1!

Rewriting to target a Lolin ESP32-C3, kick out bluetooth and use better libraries overall.

Also seems like I broke the crossfire mode of the old code anyways.

## Hardware

### Returning IO

* High voltage pulldowns (FET)
  * Mower Bale
  * Mower Power
  * GND
* Hooverboarf UART
  * TX
  * RX
  * GND
* Fan control (regulator enable; active high?)
* I2C (OLED)
* ELRS UART
  * TX
  * RX
  * GND
* Hooverboard Main Power Pulldown

And before I forget, my power distribution PCB is (Fan) Enable-12V-GND

### ESP32-C3 Mini IO map

* GPIO7 is already connected to RGB LED
* I2C (Lolin Shield Pins)
  * GPIO10 (SCL)
  * GPIO8 (SDA)
* UART? (Crossfire cluster)
  * GPIO21 (TX)
  * GPIO20 (RX)
* UART? (Moved from previous pins)
  * GPIO2 (TX)
  * GPIO1 (RX)
* GPIO3 - Output - Hoverboard Power (Hoverboard wire cluster)
* GPIO0 - Output - Mower Bale (Accessory PCB Mandatory)
* GPIO4 - Output - Mower Power (Accessory PCB Mandatory)
* GPIO6 - Ootput - Fan Control (Regulator wire cluster)

And that's it folks.  Only IO remaining is GPIO5.

Bonus:  GPIO9 is on a momentary switch.

TODO:  Check bootstrap requirements
  
## Software

### Crossfire

https://github.com/ZZ-Cat/CRSFforArduino

### Circular buffer

https://github.com/rlogiacco/CircularBuffer

### Hooverboarf control

copy from previous code

### OLED

### Webserver

### Expo?

Set on the TX so this can wait.
