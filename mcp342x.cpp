/* Copyright (C) 2015-2020 Enrico Rossi

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
 * \file
 *
 * TBD
 */

#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include "mcp342x.h"

/*! Static class initializations
 */
uint8_t MCP342x::requester { 0 };
uint8_t MCP342x::enabler { 0 };
uint8_t MCP342x::sreg { 0 };
uint8_t MCP342x::err { 0 };

/*! Create the device.
 *
 * MCP342x do not need to be suspended, but if
 * I2C must be suspended, then this should be as well.
 *
 * \param chan the channel to use.
 * \param i initialize the device.
 *
 * \warning On some installations using the I2C bus before some
 * setups may crash the system. Here this device can be
 * created without access the I2C bus.
 *
 * \bug check the status register to check the device.
 * \bug on the datasheet a general reset and latch should be
 * performed during the initialization.
 */
MCP342x::MCP342x(uint8_t chan, bool i) : channel{ chan }
{
	if (i)
		resume();

	requester++;
}

/*! Deregister the class
 *
 * If this device has already initialized, then remove it also
 * from the initializers.
 */
MCP342x::~MCP342x()
{
	requester--;

	if (status)
		suspend();
}

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
void MCP342x::resume()
{
	uint8_t buffer[4];

	if (!enabler) {
		// Read 4 byte from the device to acquire
		// the status.
		i2c.mrm(4, buffer);

		if (i2c.error()) {
			err |= (1 << MCP342X_EI2C);
		} else {
			sreg = buffer[2]; // extract the status register

			// Initialize the device by set
			// the 1st byte of the buffer.
			buffer[0] = MCP342X_REG_INIT;
			i2c.mtm(1, buffer); // send it

			if (i2c.error())
				err |= (1 << MCP342X_EI2C);
		}

		if (err)
			err |= (1 << MCP342X_ENOINIT);
	}

	enabler++; // register this initializers
	status = true; // record this init.
}

/*! De-register the device.
 *
 * Not much to do to de-register the device, just deregister
 * from the devices.
 */
void MCP342x::suspend()
{
	if (status)
		enabler--;
}

/*! Read the channel value.
 *
 * Loop 500ms in 5 read for the ADC conversion.
 *
 * \return the value read
 * \bug Missing error handling, value always returned!
 */
uint16_t MCP342x::read()
{
	uint8_t buffer[4];
	uint16_t value { 0 };

	// set the channel to read.
	switch(channel) {
		case 2:
			buffer[0] = MCP342X_REG_START_CH2;
			break;
		default:
			buffer[0] = MCP342X_REG_START_CH1;
	}

	i2c.mtm(1, buffer); // start conversion.

	if (i2c.error())
		err |= (1 << MCP342X_EI2C);
	else
		// loop an arbitray number of retry
		for (uint8_t i=0; i<5; i++) {
			i2c.mrm(4, buffer); // read 4 byte
			sreg = buffer[2]; // extract the status register

			if (i2c.error()) {
				err |= (1 << MCP342X_EI2C);
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

/*! Get the error
*/
uint8_t MCP342x::error()
{
	return(err);
}
