/*
  TwoWire.cpp - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified 2017 by Chuck Todd (ctodd@cableone.net) to correct Unconfigured Slave Mode reboot
*/

extern "C" {
  #include "utility/twi1.h"
}

#include "Wire1.h"

TwoWire Wire1 = TwoWire(TWI1_BUFFER_SIZE,
                        twi_init1,
                        twi_disable1,
                        twi_setAddress1,
                        twi_setFrequency1,
                        twi_readFrom1,
                        twi_writeTo1,
                        twi_transmit1,
                        twi_reply1,
                        twi_stop1,
                        twi_releaseBus1,
                        twi_attachSlaveRxEvent1,
                        [](uint8_t* v, int len){ Wire1.onReceiveService(v, len); },
                        twi_attachSlaveTxEvent1,
                        [](){ Wire1.onRequestService(); });
