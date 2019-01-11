/* Copyright (C) 2015-2019 Enrico Rossi

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
 * MCP342x do not need to be suspended, but if
 * I2C must be suspended, then this should be as well.
 *
 * \bug check the status register to check the device.
 */
MCP342x::MCP342x(uint8_t addr) : address{addr}
{
	uint8_t buffer[4];

	// Read 4 byte from the device to acquire
	// the status.
	i2c.mrm(4, (uint8_t *) &buffer);

	if (i2c.error()) {
		error_ |= (1 << MCP342X_ERR_I2C);
	} else {
		sreg = buffer[2]; // extract the status register

		// Initialize the device by set
		// the 1st byte of the buffer.
		buffer[0] = MCP342X_REG_INIT;
		i2c.mtm(1, (uint8_t *) &buffer); // send it

		if (i2c.error())
			error_ |= (1 << MCP342X_ERR_I2C);
	}

	if (error_)
		error_ |= (1 << MCP342X_ERR_INI);
}

/*! Read the channel value.
 *
 * Loop 500ms in 5 read for the ADC conversion.
 *
 * \param channel channel to read (1, 2).
 * \return the value read
 * \bug Missing error handling, value always returned!
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

	i2c.mtm(1, (uint8_t *) &buffer); // start conversion.

	if (i2c.error())
		error_ |= (1 << MCP342X_ERR_I2C);
	else
		// loop an arbitray number of retry
		for (uint8_t i=0; i<5; i++) {
			i2c.mrm(4, (uint8_t *) &buffer); // read 4 byte
			sreg = buffer[2]; // extract the status register

			if (i2c.error()) {
				error_ |= (1 << MCP342X_ERR_I2C);
				i = 5; // terminate
			} else {
				// data ready? (bit 7 of sreg)
				if (sreg & _BV(7)) {
					_delay_ms(100); // NOT ready, wait.
				} else {
					value = buffer[0] << 8 | buffer[1]; // swap MSB/LSB
					i = 5; // exit
				}
			}
		}

	return(value);
}
