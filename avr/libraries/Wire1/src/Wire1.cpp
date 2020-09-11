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

