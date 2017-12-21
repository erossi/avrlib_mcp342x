/* Copyright (C) 2015-2017 Enrico Rossi

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

/*!
 * \file mcp342x.cpp
 *
 */

#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include "mcp342x.h"

/*! Initialize the device.
 *
 * Single shot mode, stop bit.
 * Wait at least 300us before start.
 *
 * \ingroup sleep_group
 * MCP342x do not need to be suspended.
 *
 * \bug initialization errors missing.
 */
MCP342x::MCP342x(uint8_t addr = MCP342X_ADDR) : address{addr}
{
	uint8_t buffer[4];

	// Read 4 byte from the device to acquire
	// the status.
	i2c.rx(4, &buffer);

	// get the status register
	sreg = buffer[2];

	// set the 1st byte of the buffer
	buffer[0] = MCP342X_REG_INIT;

	// send it to the device
	i2c.tx(1, &buffer);
}

//! Suspend
void MCP342x::suspend(void)
{
	i2c.suspend();
}

//! Resume
void MCP342x::resume(void)
{
	i2c.resume();
}

/*! Read the channel value.
 *
 * Loop 500ms in 5 read for the ADC conversion.
 *
 * \param channel channel to read (1, 2).
 * \return the value read
 * \bug Missing error handling
 */
uint16_t MCP342x::read(const uint8_t channel)
{
	uint8_t buffer[4];

	// set the channel to read.
	switch(channel) {
		case 2:
			buffer[0] = MCP342X_REG_START_CH2;
			break;
		default:
			buffer[0] = MCP342X_REG_START_CH1;
	}

	// start conversion.
	i2c.tx(1, &buffer);

	if (!i2c.BusError())
		/* loop an arbitray number of retry */
		for (uint8_t i=0; i<5; i++) {
			i2c.rx(4, &buffer); // read 4 byte
			sreg = buffer[2]; // extract the status register

			if (i2c.BusError())
				i = 5; // terminate
			else
				// data ready?
				if (sreg & _BV(7)) {
					_delay_ms(100); // NOT ready, wait.
				} else {
					// swap the MSB/LSB
					value = buffer[0] << 8 | buffer[1];
					i = 5; // exit
				}
		}

	return(value);
}
