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
 * \page mcp342x ADC usage and diagnostic.
 * \author Enrico Rossi.
 * \copyright GNU Lesser General Public License.
 *
 * \section i2c Works with I2C bus.
 */

#ifndef _MCP342X_H_
#define _MCP342X_H_

#include "i2c.h"

//! default device address 0b1101000[0/1]
#define MCP342X_ADDR 0xd0

/*! Register setup:
 * One Shot
 * 16 bit resolution
 * 1x gain.
 */
#define MCP342X_REG_INIT 0x08

//! Start a sample on the channel 1
#define MCP342X_REG_START_CH1 (MCP342X_REG_INIT | (1 << 7))

//! Start a sample on the channel 2
#define MCP342X_REG_START_CH2 (MCP342X_REG_INIT | (1 << 7) | (1 << 5))

// Errors
//! I2C bus Error
#define MCP342X_EI2C 0
//! Initialization error
#define MCP342X_ENOINIT 1

class MCP342x {
	private:
		static uint8_t initializers; // istances
		static uint8_t sreg; //! status register
		static uint8_t err; //! device error.
		I2C i2c { MCP342X_ADDR }; //! i2c bus object
		uint8_t channel;
	public:
		MCP342x(uint8_t); // constructor
		~MCP342x();
		uint8_t error(); //! errors
		uint16_t read();
};

#endif
