#include "WiringPiSpi.h"
#include <cstdio>
 
 using namespace std;
 uint8_t     WiringPiSpiHelper::spiBPW = 8;
 uint16_t    WiringPiSpiHelper::spiDelay = 0;
 uint32_t WiringPiSpiHelper::spiSpeeds[];
 int WiringPiSpiHelper::spiFds[];
 
 
 

	int WiringPiSpiHelper::Failure (int fatal, const char *message, ...)
	{
	  va_list argp ;
	  char buffer [1024] ;

	  if (!fatal)
	    return -1 ;

	  va_start (argp, message) ;
	  vsnprintf (buffer, 1023, message, argp) ;
	  va_end (argp) ;

	  fprintf (stderr, "%s", buffer) ;

	  return -1 ;
	}

	/*
	 * wiringPiSPIGetFd:
	 *	Return the file-descriptor for the given channel
	 *********************************************************************************
	 */

	int WiringPiSpiHelper::wiringPiSPI1GetFd (int channel)
	{
	  return spiFds [channel & 1] ;
	}


	/*
	 * wiringPiSPIDataRW:
	 *	Write and Read a block of data over the SPI bus.
	 *	Note the data ia being read into the transmit buffer, so will
	 *	overwrite it!
	 *	This is also a full-duplex operation.
	 *********************************************************************************
	 */

	int WiringPiSpiHelper::wiringPiSPI1DataRW (int channel, unsigned char *data, int len)
	{
	  struct spi_ioc_transfer spi ;

	  channel &= 1 ;

	// Mentioned in spidev.h but not used in the original kernel documentation
	//	test program )-:

	  memset (&spi, 0, sizeof (spi)) ;

	  spi.tx_buf        = (unsigned long)data ;
	  spi.rx_buf        = (unsigned long)data ;
	  spi.len           = len ;
	  spi.delay_usecs   = spiDelay ;
	  spi.speed_hz      = spiSpeeds [channel] ;
	  spi.bits_per_word = spiBPW ;

	  return ioctl (spiFds [channel], SPI_IOC_MESSAGE(1), &spi) ;
	}


	/*
	 * wiringPiSPISetupMode:
	 *	Open the SPI device, and set it up, with the mode, etc.
	 *********************************************************************************
	 */

	int WiringPiSpiHelper::wiringPiSPI1SetupMode (int channel, int speed, int mode)
	{
	  int fd ;

	  mode    &= 3 ;	// Mode is 0, 1, 2 or 3
	  channel &= 1 ;	// Channel is 0 or 1

	  if ((fd = open (channel == 0 ? spiDev0 : spiDev1, O_RDWR)) < 0)
	    return Failure (-1, "Unable to open SPI device: %s\n", strerror (errno)) ;

	  spiSpeeds [channel] = speed ;
	  spiFds    [channel] = fd ;

	// Set SPI parameters.

	  if (ioctl (fd, SPI_IOC_WR_MODE, &mode)            < 0)
	    return Failure (-1, "SPI Mode Change failure: %s\n", strerror (errno)) ;
	  
	  if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
	    return Failure (-1, "SPI BPW Change failure: %s\n", strerror (errno)) ;

	  if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)   < 0)
	    return Failure (-1, "SPI Speed Change failure: %s\n", strerror (errno)) ;

	  return fd ;
	}


	/*
	 * wiringPiSPISetup:
	 *	Open the SPI device, and set it up, etc. in the default MODE 0
	 *********************************************************************************
	 */

	int WiringPiSpiHelper::wiringPiSPI1Setup (int channel, int speed)
	{
	  return wiringPiSPI1SetupMode (channel, speed, 0) ;
	}
