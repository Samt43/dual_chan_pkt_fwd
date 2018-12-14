 #pragma once 
/*
 * wiringPiSPI.c:
 *	Simplified SPI access routines
 *	Copyright (c) 2012-2015 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringPi.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */


#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdarg.h>

/*
 * wiringPiFailure:
 *	Fail. Or not.
 *********************************************************************************
 */



class WiringPiSpiHelper
{
	// The SPI bus parameters
	//	Variables as they need to be passed as pointers later on

	static constexpr char       *spiDev0  = "/dev/spidev1.0" ;
	static constexpr char       *spiDev1  = "/dev/spidev1.1" ;
	static uint8_t     spiBPW;
	static uint16_t    spiDelay;

	static uint32_t    spiSpeeds [2] ;
	static int         spiFds [2] ;
	
	public:


	static int Failure (int fatal, const char *message, ...);

	/*
	 * wiringPiSPIGetFd:
	 *	Return the file-descriptor for the given channel
	 *********************************************************************************
	 */

	static int wiringPiSPI1GetFd (int channel);


	/*
	 * wiringPiSPIDataRW:
	 *	Write and Read a block of data over the SPI bus.
	 *	Note the data ia being read into the transmit buffer, so will
	 *	overwrite it!
	 *	This is also a full-duplex operation.
	 *********************************************************************************
	 */

	static int wiringPiSPI1DataRW (int channel, unsigned char *data, int len);


	/*
	 * wiringPiSPISetupMode:
	 *	Open the SPI device, and set it up, with the mode, etc.
	 *********************************************************************************
	 */

	static int wiringPiSPI1SetupMode (int channel, int speed, int mode);


	/*
	 * wiringPiSPISetup:
	 *	Open the SPI device, and set it up, etc. in the default MODE 0
	 *********************************************************************************
	 */

	static int wiringPiSPI1Setup (int channel, int speed);

};