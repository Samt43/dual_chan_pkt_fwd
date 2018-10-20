//
// Copyright (c) 2016 Maarten Westenberg version for ESP8266
// Verison 3.2.0
// Date: 2016-10-08
//
// 	based on work done by Thomas Telkamp for Raspberry PI 1ch gateway
//
//
//	and many others.
// Adapted for raspberry pi with Uputronics Raspberry Pi+ LoRa(TM) Expansion Board
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// Author: Maarten Westenberg
//
// This file contains the LoRa modem specific code enabling to receive
// and transmit packages/messages.
//

#include "LoraModem.h"

uint16_t bw = 125;
int debug = 1;
std::vector<Server_t> servers;
struct sockaddr_in si_other;
SpreadingFactor_t sf = SF7;
int s;
int slen = sizeof(si_other);
struct ifreq ifr;
uint32_t cp_nb_rx_rcv;
uint32_t cp_nb_rx_ok;
uint32_t cp_nb_rx_ok_tot;
uint32_t cp_nb_rx_bad;
uint32_t cp_nb_rx_nocrc;
uint32_t cp_up_pkt_fwd;

long txDelay= 0000; 

// Frequencies
// Set center frequency. If in doubt, choose the first one, comment all others
// Each "real" gateway should support the first 3 frequencies according to LoRa spec.
uint32_t  freq = 868100000;                                     // Channel 0, 868.1 MHz
uint32_t  freq_2 = 868300000;                                   // Channel 1, 868.3 MHz
//uint32_t  freq = 868500000;                                   // Channel 1, 868.5 MHz
//uint32_t  freq = 867100000;                                   // Channel 2, 867.1 MHz
//uint32_t  freq = 867300000;                                   // in Mhz! (867.3)
//uint32_t  freq = 867500000;                                   // in Mhz! (867.5)
//uint32_t  freq = 867700000;                                   // in Mhz! (867.7)
//uint32_t  freq = 867900000;                                   // in Mhz! (867.9)
//uint32_t  freq = 868800000;                                   // in Mhz! (868.8)
//uint32_t  freq = 869525000;                                   // in Mhz! (869.525)
// TTN defines an additional channel at 869.525Mhz using SF9 for class B. Not used

int ssPin = 0xff;
int dio0  = 0xff;
int ssPin_2 = 0xff;
int dio0_2  = 0xff;
int RST   = 0xff;
int Led1  = 0xff;
int NetworkLED    = 22;
int InternetLED   = 23;
int ActivityLED_0 = 21;
int ActivityLED_1 = 29;

bool sx1272 = true;

char *PinName(int pin, char *buff) 
{
  strcpy(buff, "unused");
  if (pin != 0xff) {
    sprintf(buff, "%d", pin);
  }
  return buff;
}

// ============================================================================
// LORA GATEWAY/MODEM FUNCTIONS
//
// The LoRa supporting functions are in the section below

// ----------------------------------------------------------------------------
// The SS (Chip select) pin is used to make sure the RFM95 is selected
// ----------------------------------------------------------------------------
void SelectReceiver(byte CE)
{
   if (CE == 0)
   {
    digitalWrite(ssPin, LOW);
   } else {
    digitalWrite(ssPin_2, LOW);
   }
}

// ----------------------------------------------------------------------------
// ... or unselected
// ----------------------------------------------------------------------------
void UnselectReceiver(byte CE)
{
    if (CE == 0)
    {
      digitalWrite(ssPin, HIGH);
    } else {
      digitalWrite(ssPin_2, HIGH);
    }
}


// ----------------------------------------------------------------------------
// Read one byte value, par addr is address
// Returns the value of register(addr)
// ----------------------------------------------------------------------------
uint8_t ReadRegister(uint8_t addr, byte CE)
{
    uint8_t spibuf[2];

    SelectReceiver(CE);
    spibuf[0] = addr & 0x7F;
    spibuf[1] = 0x00;
    wiringPiSPIDataRW(CE, spibuf, 2);
    UnselectReceiver(CE);

    return spibuf[1];
}

// ----------------------------------------------------------------------------
// Write value to a register with address addr. 
// Function writes one byte at a time.
// ----------------------------------------------------------------------------
void WriteRegister(uint8_t addr, uint8_t value, byte CE)
{
    uint8_t spibuf[2];

    SelectReceiver(CE);
    spibuf[0] = addr | 0x80;
    spibuf[1] = value;
    wiringPiSPIDataRW(CE, spibuf, 2);

    UnselectReceiver(CE);
}


// ----------------------------------------------------------------------------
//  setRate is setting rate etc. for transmission
//		Modem Config 1 (MC1) ==
//		Modem Config 2 (MC2) == (CRC_ON) | (sf<<4)
//		Modem Config 3 (MC3) == 0x04 | (optional SF11/12 LOW DATA OPTIMIZE 0x08)
//		sf == SF7 default 0x07, (SF7<<4) == SX72_MC2_SF7
//		bw == 125 == 0x70
//		cr == CR4/5 == 0x02
//		CRC_ON == 0x04
// ----------------------------------------------------------------------------
void setRate(uint8_t sf, uint8_t crc, byte CE) {

	uint8_t mc1=0, mc2=0, mc3=0;
	// Set rate based on Spreading Factor etc
    if (sx1272) {
		mc1= 0x0A;				// SX1276_MC1_BW_250 0x80 | SX1276_MC1_CR_4_5 0x02
		mc2= (sf<<4) | crc;
		// SX1276_MC1_BW_250 0x80 | SX1276_MC1_CR_4_5 0x02 | SX1276_MC1_IMPLICIT_HEADER_MODE_ON 0x01
        if (sf == SF11 || sf == SF12) { mc1= 0x0B; }			        
    } 
	else {
	    mc1= 0x72;				// SX1276_MC1_BW_125==0x70 | SX1276_MC1_CR_4_5==0x02
		mc2= (sf<<4) | crc;		// crc is 0x00 or 0x04==SX1276_MC2_RX_PAYLOAD_CRCON
		mc3= 0x04;				// 0x04; SX1276_MC3_AGCAUTO
        if (sf == SF11 || sf == SF12) { mc3|= 0x08; }		// 0x08 | 0x04
    }
	
	// Implicit Header (IH), for class b beacons
	//if (getIh(LMIC.rps)) {
    //   mc1 |= SX1276_MC1_IMPLICIT_HEADER_MODE_ON;
    //    writeRegister(REG_PAYLOAD_LENGTH, getIh(LMIC.rps)); // required length
    //}
		
	WriteRegister(REG_MODEM_CONFIG1, mc1, CE);
	WriteRegister(REG_MODEM_CONFIG2, mc2, CE);
	WriteRegister(REG_MODEM_CONFIG3, mc3, CE);
	
	// Symbol timeout settings
    if (sf == SF10 || sf == SF11 || sf == SF12) {
        WriteRegister(REG_SYMB_TIMEOUT_LSB,0x05, CE);
    } else {
        WriteRegister(REG_SYMB_TIMEOUT_LSB,0x08, CE);
    }
	
	return;
}


// ----------------------------------------------------------------------------
// Set the frequency for our gateway
// The function has no parameter other than the freq setting used in init.
// Since we are usin a 1ch gateway this value is set fixed.
// ----------------------------------------------------------------------------
void setFreq(uint32_t freq, byte CE)
{
 uint64_t frf;
    // set frequency
   if (CE==0){
    frf = ((uint64_t)freq << 19) / 32000000;
	if (debug >= 2) {
		printf("setFreq CE0 using: %i\n", freq);
	}
   } else {
    frf = ((uint64_t)freq_2 << 19) / 32000000;
	if (debug >= 2) {
		printf("setFreq CE1 using: %i\n", freq_2);
	}
   }
    WriteRegister(REG_FRF_MSB, (uint8_t)(frf>>16), CE );
    WriteRegister(REG_FRF_MID, (uint8_t)(frf>> 8), CE );
    WriteRegister(REG_FRF_LSB, (uint8_t)(frf>> 0), CE );
	
	return;
}


// ----------------------------------------------------------------------------
//	Set Power for our gateway
// ----------------------------------------------------------------------------
void setPow(uint8_t powe, byte CE) {

	if (powe >= 16) powe = 15;
	//if (powe >= 15) powe = 14;
	else if (powe < 2) powe =2;
	
	uint8_t pac = 0x80 | (powe & 0xF);
	WriteRegister(REG_PAC,pac, CE);								// set 0x09 to pac
	
	if (debug >=2){printf("setPow: CE: %i, powe: %i\n" ,CE, powe);}
	
	// XXX Power settings for CFG_sx1272 are different
	
	return;
}


// ----------------------------------------------------------------------------
// Used to set the radio to LoRa mode (transmitter)
// ----------------------------------------------------------------------------
static void opmodeLora(byte CE) {
    uint8_t u = OPMODE_LORA;
#ifdef CFG_sx1276_radio
    u |= 0x8;   											// TBD: sx1276 high freq
#endif
    WriteRegister(REG_OPMODE, u, CE);
}


// ----------------------------------------------------------------------------
// Set the opmode to a value as defined on top
// Values are 0x00 to 0x07
// ----------------------------------------------------------------------------
static void opmode(uint8_t mode, byte CE) {
    WriteRegister(REG_OPMODE, (ReadRegister(REG_OPMODE, CE) & ~OPMODE_MASK) | mode, CE);
//	writeRegister(REG_OPMODE, (ReadRegister(REG_OPMODE) & ~OPMODE_MASK) | mode);
}


// ----------------------------------------------------------------------------
// This DOWN function sends a payload to the LoRa node over the air
// Radio must go back in standby mode as soon as the transmission is finished
// ----------------------------------------------------------------------------
bool sendPkt(uint8_t *payLoad, uint8_t payLength, uint32_t tmst, byte CE)
{
	WriteRegister(REG_FIFO_ADDR_PTR, ReadRegister(REG_FIFO_TX_BASE_AD, CE), CE);	// 0x0D, 0x0E
	WriteRegister(REG_PAYLOAD_LENGTH, payLength, CE);				// 0x22
	if (debug>=2) { printf("Sending to LoRa node: "); } 
	for(int i = 0; i < payLength; i++)
    	{
       	 	WriteRegister(REG_FIFO, payLoad[i], CE);					// 0x00
		if (debug>=2) { printf("%i:", payLoad[i]); }
    	}
        if (debug>=2) { printf("\n"); }
	return true;
}


// ----------------------------------------------------------------------------
// Setup the LoRa receiver on the connected transceiver.
// - Determine the correct transceiver type (sx1272/RFM92 or sx1276/RFM95)
// - Set the frequency to listen to (1-channel remember)
// - Set Spreading Factor (standard SF7)
// The reset RST pin might not be necessary for at least the RGM95 transceiver
//
// 1. Put the radio in LoRa mode
// 2. Put modem in sleep or in standby
// 3. Set Frequency
// ----------------------------------------------------------------------------
void rxLoraModem(byte CE)
{
	// 1. Put system in LoRa mode
	opmodeLora(CE);
	
	// Put the radio in sleep mode
    opmode(OPMODE_SLEEP, CE);										// set 0x01 to 0x00
	
	// 3. Set frequency based on value in freq
	setFreq(freq, CE);												// set to 868.1MHz or 868.3MHz

	// Set spreading Factor
    if (sx1272) {
        if (sf == SF11 || sf == SF12) {
            WriteRegister(REG_MODEM_CONFIG1,0x0B, CE);
        } else {
            WriteRegister(REG_MODEM_CONFIG1,0x0A, CE);
        }
        WriteRegister(REG_MODEM_CONFIG2,(sf<<4) | 0x04, CE);
    } else {
        if (sf == SF11 || sf == SF12) {
            WriteRegister(REG_MODEM_CONFIG3,0x0C, CE);				// 0x08 | 0x04
        } else {
            WriteRegister(REG_MODEM_CONFIG3,0x04, CE);				// 0x04; SX1276_MC3_LOW_DATA_RATE_OPTIMIZE
        }
        WriteRegister(REG_MODEM_CONFIG1,0x72, CE);
        WriteRegister(REG_MODEM_CONFIG2,(sf<<4) | 0x04, CE);		// Set mc2 to (SF<<4) | CRC==0x04
    }

    if (sf == SF10 || sf == SF11 || sf == SF12) {
        WriteRegister(REG_SYMB_TIMEOUT_LSB,0x05, CE);
    } else {
        WriteRegister(REG_SYMB_TIMEOUT_LSB,0x08, CE);
    }
	
	// prevent node to node communication
	WriteRegister(REG_INVERTIQ,0x27, CE);							// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256byte buffer. At startup TX starts at
	// 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
    	WriteRegister(REG_MAX_PAYLOAD_LENGTH,0x80, CE);					// set 0x23 to 0x80
    	WriteRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH, CE);			// 0x22, 0x40; Payload is 64Byte long
	
    	WriteRegister(REG_FIFO_ADDR_PTR, ReadRegister(REG_FIFO_RX_BASE_AD, CE), CE);	// set 0x0D to 0x0F

	WriteRegister(REG_HOP_PERIOD,0x00, CE);							// 0x24, 0x00 was 0xFF
	
    	// Low Noise Amplifier used in receiver
    	WriteRegister(REG_LNA, LNA_MAX_GAIN, CE);  						// 0x0C, 0x23
	
	WriteRegister(REG_IRQ_FLAGS_MASK, ~IRQ_LORA_RXDONE_MASK, CE);	// Accept no interrupts except RXDONE
	WriteRegister(REG_DIO_MAPPING_1, MAP_DIO0_LORA_RXDONE, CE);		// Set RXDONE interrupt to dio0
	
	// Set Continous Receive Mode
    	opmode(OPMODE_RX, CE);											// 0x80 | 0x05 (listen)
	
	return;
}

// ----------------------------------------------------------------------------
// loraWait()
// This function implements the wait protocol needed for downstream transmissions.
// Note: Timing of downstream and JoinAccept messages is VERY critical.
//
// As the ESP8266 watchdog will not like us to wait more than a few hundred
// milliseconds (or it will kick in) we have to implement a simple way to wait
// time in case we have to wait seconds before sending messages (e.g. for OTAA 5 or 6 seconds)
// Without it, the system is known to crash in half of the cases it has to wait for 
// JOIN-ACCEPT messages to send.
//
// This function uses a combination of delay() statements and delayMicroseconds().
// As we use delay() only when there is still enough time to wait and we use micros()
// to make sure that delay() did not take too much time this works.
// 
// Parameter: uint32-t tmst gives the micros() value when transmission should start.
// ----------------------------------------------------------------------------
void loraWait(uint32_t tmst) {
      struct timeval now;
      uint32_t curr_time;
      gettimeofday(&now, NULL);
      uint32_t startTime = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);
	//uint32_t startTime = time(NULL);						// Start of the loraWait function
	tmst += txDelay;
	uint32_t waitTime = tmst - startTime;
		
	while (waitTime > 16000) {
       //         if (debug >=1) { printf("tmst; %i, timestamp: %i\n", tmst, time(NULL)); }
		delay(15);										// ms delay() including yield, slightly shorter
	        gettimeofday(&now, NULL);
                curr_time = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);
		waitTime= tmst - curr_time;
	}
	if (waitTime>0) delayMicroseconds(waitTime);
	else if ((waitTime+20) < 0) printf("loraWait TOO LATE\n");
	
	//yield();
	if (debug >=2) { 
		printf("start: %i",startTime);
		printf(", end: %i", tmst);
		printf(", waited: %i", tmst - startTime);
		printf(", delay=%li\n", txDelay);
	}
}


// ----------------------------------------------------------------------------
// txLoraModem
// Init the transmitter and transmit the buffer
// After successful transmission (dio0==1) re-init the receiver
//
//	crc is set to 0x00 for TX
//	iiq is set to 0x27 (or 0x40 based on ipol value in txpkt)
//
//	1. opmodeLoRa
//	2. opmode StandBY
//	3. Configure Modem
//	4. Configure Channel
//	5. write PA Ramp
//	6. config Power
//	7. RegLoRaSyncWord LORA_MAC_PREAMBLE
//	8. write REG dio mapping (dio0)
//	9. write REG IRQ flags
// 10. write REG IRQ flags mask
// 11. write REG LoRa Fifo Base Address
// 12. write REG LoRa Fifo Addr Ptr
// 13. write REG LoRa Payload Length
// 14. Write buffer (byte by byte)
// 15. opmode TX
// ----------------------------------------------------------------------------

static void txLoraModem(uint8_t *payLoad, uint8_t payLength, uint32_t tmst, uint8_t sfTx,
						uint8_t powe, uint32_t freq, uint8_t crc, uint8_t iiq, byte CE)
{
	if (debug>=2) {
		// Make sure that all serial stuff is done before continuing
		printf("txLoraModem::");
		printf("  powe: %i", powe);
		printf(", freq: %i", freq);
		printf(", crc: %i", crc);
		printf(", iiq: 0X%hi", iiq);
	}
	
	// 1. Select LoRa modem from sleep mode
	opmodeLora(CE);												// set register 0x01 to 0x80
	
	// Assert the value of the current mode
	if (ReadRegister(REG_OPMODE, CE) && OPMODE_LORA != 0 && debug>=2) { printf("Not 0\n");}
	
	// 2. enter standby mode (required for FIFO loading))
	opmode(OPMODE_STANDBY, CE);										// set 0x01 to 0x01
	
	// 3. Init spreading factor and other Modem setting
	setRate(sfTx, crc, CE);
	
	//writeRegister(REG_HOP_PERIOD, 0x00);						// set 0x24 to 0x00 only for receivers
	
	// 4. Init Frequency, config channel
	setFreq(freq, CE);

	// 6. Set power level, REG_PAC
	setPow(powe, CE);
	
	// 7. prevent node to node communication
	WriteRegister(REG_INVERTIQ,iiq, CE);							// 0x33, (0x27 or 0x40)
	
	// 8. set the IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP (or lesss for 1ch gateway)
    	//writeRegister(REG_DIO_MAPPING_1, MAP_DIO0_LORA_TXDONE|MAP_DIO1_LORA_NOP|MAP_DIO2_LORA_NOP);
	WriteRegister(REG_DIO_MAPPING_1, MAP_DIO0_LORA_TXDONE, CE);		// set 0x40 to 0x40
	
	// 9. clear all radio IRQ flags
    	WriteRegister(REG_IRQ_FLAGS, 0xFF, CE);
	
	// 10. mask all IRQs but TxDone
    	WriteRegister(REG_IRQ_FLAGS_MASK, ~IRQ_LORA_TXDONE_MASK, CE);
	
	// txLora
	opmode(OPMODE_FSTX, CE);										// set 0x01 to 0x02 (actual value becomes 0x82)

	// 11, 12, 13, 14. write the buffer to the FiFo
	sendPkt(payLoad, payLength, tmst, CE);

	// wait extra delay out. The delayMicroseconds timer is accurate until 16383 uSec.
	loraWait(tmst);
	//delay(5000);
	
	// 15. Initiate actual transmission of FiFo
	opmode(OPMODE_TX, CE);											// set 0x01 to 0x03 (actual value becomes 0x83)
	
	// XXX Intead of handling the interrupt of dio0, we wait it out, Not using delay(1);
	// for trasmitter this should not be a problem.
        if ( CE == 0) {
	while(digitalRead(dio0) == 0) {  }							// XXX tx done? handle by interrupt
        } else {
	while(digitalRead(dio0_2) == 0) {  }							// XXX tx done? handle by interrupt
	}

	// ----- TX SUCCESS, SWITCH BACK TO RX CONTINUOUS --------
	// Successful TX cycle put's radio in standby mode.
        if (debug >=2) { printf("Send TX done.\n");}
	
	// Reset the IRQ register
	WriteRegister(REG_IRQ_FLAGS, 0xFF, CE);							// set 0x12 to 0xFF

	// Give control back to continuous receive setup
	// Put's the radio in sleep mode and then in stand-by
	rxLoraModem(CE);								
	//initLoraModem(CE);
}

// ----------------------------------------------------------------------------
// First time initialisation of the LoRa modem
// Subsequent changes to the modem state etc. done by txLoraModem or rxLoraModem
// After initialisation the modem is put in rxContinuous mode (listen)
// ----------------------------------------------------------------------------
void initLoraModem(byte CE)
{
  char buff[16];
	
    //digitalWrite(RST, HIGH);
    //delay(100);
    //digitalWrite(RST, LOW);
    //delay(100);
  if (CE == 0) {
    printf("Trying to detect module CE0 with ");
    printf("NSS=%s "  , PinName(ssPin, buff));
    printf("DIO0=%s " , PinName(dio0 , buff));
    printf("Reset=%s ", PinName(RST  , buff));
    printf("Led1=%s\n", PinName(Led1 , buff));
  } else {
    printf("Trying to detect module CE1 with ");
    printf("NSS=%s "  , PinName(ssPin_2, buff));
    printf("DIO0=%s " , PinName(dio0_2 , buff));
    printf("Reset=%s ", PinName(RST  , buff));
    printf("Led1=%s\n", PinName(Led1 , buff));
  }

  // check basic 
  if (ssPin == 0xff || dio0 == 0xff) {
    Die("Bad pin configuration ssPin and dio0 need at least to be defined");
  }
	
    uint8_t version = ReadRegister(REG_VERSION, CE);				// Read the LoRa chip version id
    if (version == 0x22) {
        // sx1272
        printf("WARNING:: SX1272 detected\n");
        sx1272 = true;
    } else {
        // sx1276?
        digitalWrite(RST, LOW);
        delay(100);
        digitalWrite(RST, HIGH);
        delay(100);
        version = ReadRegister(REG_VERSION, CE);
        if (version == 0x12) {
            // sx1276
            if (debug >=1) printf("CE%i: SX1276 detected, starting.\n", CE);
            sx1272 = false;
        } else {
           printf("Transceiver version 0x%02X\n", version);
           Die("Unrecognized transceiver");
        }
    }
        opmodeLora(CE);												// set register 0x01 to 0x80
        opmode(OPMODE_SLEEP, CE);										// set register 0x01 to 0x00

	// 7. set sync word
        WriteRegister(REG_SYNC_WORD, 0x34, CE);							// set 0x39 to 0x34 LORA_MAC_PREAMBLE
	
	// 5. Config PA Ramp up time								// set 0x0A t0 
	WriteRegister(REG_PARAMP, (ReadRegister(REG_PARAMP, CE) & 0xF0) | 0x08, CE); // set PA ramp-up time 50 uSec
	
	// Set 0x4D PADAC for SX1276 ; XXX register is 0x5a for sx1272
	WriteRegister(REG_PADAC_SX1276,  0x84, CE); 					// set 0x4D (PADAC) to 0x84
	//writeRegister(REG_PADAC, ReadRegister(REG_PADAC, CE)|0x4);
	
	// Set the radio in Continuous listen mode
	rxLoraModem(CE);
	if (debug >= 2) printf("initLoraModem CE%i done\n", CE);
}


// ----------------------------------------------------------------------------
// This LoRa function reads a message from the LoRa transceiver
// returns message length read when message correctly received or 
// it returns a negative value on error (CRC error for example).
// UP function
// ----------------------------------------------------------------------------
bool ReceivePkt(uint8_t *payload , uint8_t* p_length, byte CE)
{
    // clear rxDone
    WriteRegister(REG_IRQ_FLAGS, 0x40, CE);						// 0x12; Clear RxDone
	
    int irqflags = ReadRegister(REG_IRQ_FLAGS, CE);				// 0x12

    cp_nb_rx_rcv++;											// Receive statistics counter

  //  payload crc: 0x20
  if((irqflags & 0x20) == 0x20) {
    printf("CRC error\n");
    WriteRegister(REG_IRQ_FLAGS, 0x20, CE);
    return false;

    } else {
    cp_nb_rx_ok++;
    cp_nb_rx_ok_tot++;

    uint8_t currentAddr = ReadRegister(REG_FIFO_RX_CURRENT_ADDR, CE);
    uint8_t receivedCount = ReadRegister(REG_RX_NB_BYTES, CE);
    *p_length = receivedCount;

    WriteRegister(REG_FIFO_ADDR_PTR, currentAddr, CE); // 0x0D XXX??? This sets the FiFo higher!!!

    for(int i = 0; i < receivedCount; i++) {
      payload[i] = ReadRegister(REG_FIFO, CE);
    }
  }
  return true;

}

// ----------------------------------------------------------------------------
// Send DOWN a LoRa packet over the air to the node. This function does all the 
// decoding of the server message and prepares a Payload buffer.
// The payload is actually transmitted by the sendPkt() function.
// This function is used for regular downstream messages and for JOIN_ACCEPT
// messages.
// ----------------------------------------------------------------------------
int sendPacket(uint8_t *buff_down, uint8_t length, byte CE) {

	// Received package with Meta Data:
	// codr	: "4/5"
	// data	: "Kuc5CSwJ7/a5JgPHrP29X9K6kf/Vs5kU6g=="	// for example
	// freq	: 868.1 									// 868100000
	// ipol	: true/false
	// modu : "LORA"
	// powe	: 14										// Set by default
	// rfch : 0											// Set by default
	// size	: 21
	// tmst : 1800642 									// for example
	// datr	: "SF7BW125"
	
	// 12-byte header;
	//		HDR (1 byte)
	//		
	//
	// Data Reply for JOIN_ACCEPT as sent by server:
	//		AppNonce (3 byte)
	//		NetID (3 byte)
	//		DevAddr (4 byte) [ 31..25]:NwkID , [24..0]:NwkAddr
 	//		DLSettings (1 byte)
	//		RxDelay (1 byte)
	//		CFList (fill to 16 bytes)
	
	int i=0;
	//StatictJsonBuffer<256> jsonBuffer;

/*
        JSON_Value *root_val = NULL;
	JSON_Object *txpk_obj = NULL;
	//JSON_Value *val = NULL;

	char * bufPtr = (char *) (buff_down);
	//buff_down[length] = 0;
	
	if (debug >= 2) printf((char *)buff_down);
	
	// Use JSON to decode the string after the first 4 bytes.
	// The data for the node is in the "data" field. This function destroys original buffer
	//JsonObject& root = jsonBuffer.parseObject(bufPtr);
	root_val = json_parse_string_with_comments((const char *)(bufPtr));
		
	//if (!root.success()) {
	if (root_val == NULL) {
		printf("sendPacket:: ERROR Json Decode \n");
		if (debug>=2) {
			printf(": %s", bufPtr);
		}
		return(-1);
	}
	delay(1);
	// Meta Data sent by server (example)
	// {"txpk":{"codr":"4/5","data":"YCkEAgIABQABGmIwYX/kSn4Y","freq":868.1,"ipol":true,"modu":"LORA","powe":14,"rfch":0,"size":18,"tmst":1890991792,"datr":"SF7BW125"}}

	// Used in the protocol:
	//const char * data	= root["txpk"]["data"];
	//uint8_t psize		= root["txpk"]["size"];
	//bool ipol			= root["txpk"]["ipol"];
	//uint8_t powe		= root["txpk"]["powe"];
	//uint32_t tmst		= (uint32_t) root["txpk"]["tmst"].as<unsigned long>();

	txpk_obj = json_object_get_object(json_value_get_object(root_val), "txpk");
			if (txpk_obj == NULL) {
				printf("WARNING: [down] no \"txpk\" object in JSON, TX aborted\n");
				json_value_free(root_val);
			}
*/
	const char * data	= "TODO";// json_object_get_string(txpk_obj, "data");
	uint8_t psize		= 0;//(uint8_t) json_value_get_number(json_object_get_value(txpk_obj, "size"));
	bool ipol		= "false";//(bool) json_value_get_boolean(json_object_get_value(txpk_obj,"ipol"));
	uint8_t powe		= 0; // json_value_get_number(json_object_get_value(txpk_obj,"powe"));
	uint32_t tmst		= 0; //(uint32_t) json_value_get_number(json_object_get_value(txpk_obj,"tmst"));

	// Not used in the protocol:
	const char * datr	= "TODO"; //json_object_get_string(txpk_obj, "datr");			// eg "SF7BW125"
	const float ff		= 0 ;//json_value_get_number(json_object_get_value(txpk_obj,"freq"));			// eg 869.525
	if (debug>=2) { printf("Sending frequency from server package: %lf\n", ff); }
	const char * modu	= "";//json_object_get_string(txpk_obj, "modu");			// =="LORA"
	const char * codr	= "";//json_object_get_string(txpk_obj, "codr");
	//if (root["txpk"].containsKey("imme") ) {
	//	const bool imme = root["txpk"]["imme"];			// Immediate Transmit (tmst don't care)
	//}


	
	if (data != NULL) {
		if (debug>=2) { printf("data: %s", (char *) data); }
	}
	else {
		printf("sendPacket:: ERROR data is NULL\n");
		return(-1);
	}
	
	uint8_t iiq = (ipol? 0x40: 0x27);					// if ipol==true 0x40 else 0x27
	uint8_t crc = 0x00;									// switch CRC off for TX
	//uint8_t payLength = base64_dec_len((char *) data, strlen(data));
	uint8_t payLoad[1024];							// Declare buffer
	//base64_decode((char *) payLoad, (char *) data, strlen(data));
	int payLength = b64_to_bin(data, strlen(data), payLoad, 1024);

	// Compute wait time in microseconds
	uint32_t w = (uint32_t) (tmst - time(NULL));
	
#if _STRICT_1CH == 1
	// Use RX1 timeslot as this is our frequency.
	// Do not use RX2 or JOIN2 as they contain other frequencies
	if ((w>1000000) && (w<3000000)) { tmst-=1000000; }
	else if ((w>6000000) && (w<7000000)) { tmst-=1000000; }
	
	const uint8_t sfTx = sf;
	const uint32_t fff = freq;
	txLoraModem(payLoad, payLength, tmst, sfTx,  powe, fff, crc, iiq, CE);
#else
	const uint8_t sfTx = atoi(datr+2);					// Convert "SF9BW125" to 9
	// convert double frequency (MHz) into uint32_t frequency in Hz.
	const uint32_t fff = (uint32_t) ((uint32_t)((ff+0.000035)*1000)) * 1000;
	// Not we determine CE from the frequency
	if (ff>868.2) { CE = 1; }
	txLoraModem(payLoad, payLength, tmst, sfTx, powe, fff, crc, iiq, CE);
#endif

	if (debug>=1) {
		printf("Sending packet:: ");
		printf("tmst=%i, ",tmst); 
		printf("wait= %i, ", w);
		
		printf("strict=%i, ", _STRICT_1CH);
		printf("datr=%s, ", datr);
		printf("freq=%i->%i, ", freq,fff);
		printf("sf=%i->%i, ",sf, sfTx);
		
		printf("modu=%s, ", modu);
		printf("powe=%i, ", powe);
		printf("codr=%s, ", codr);

		printf("ipol=%i, through CE%i.\n",ipol, CE);
	}
	
	if (payLength != psize) {
		printf("sendPacket:: WARNING payLength: %i", payLength);
		printf(", psize=%i", psize);
	}
	else if (debug >= 2 ) {
		for (i=0; i<payLength; i++) {printf("%hi:", payLoad[i]); }
		printf("\n");
	}

	cp_up_pkt_fwd++;
	
	return 1;
}

// ----------------------------------------------------------------------------
// Convert a float to string for printing
// f is value to convert
// p is precision in decimal digits
// val is character array for results
// ----------------------------------------------------------------------------
void ftoa(float f, char *val, int p) {
	int j=1;
	int ival, fval;
	char b[6];
	
	for (int i=0; i< p; i++) { j= j*10; }

	ival = (int) f;								// Make integer part
	fval = (int) ((f- ival)*j);					// Make fraction. Has same sign as integer part
	if (fval<0) fval = -fval;					// So if it is negative make fraction positive again.
												// sprintf does NOT fit in memory
	//strcat(val,itoa(ival,b,10));
        snprintf(b, sizeof(b), "%d", ival);
	strcat(val,b);
	strcat(val,".");							// decimal point
	
	//itoa(fval,b,10);
        snprintf(b, sizeof(b), "%d", fval);
	strcat(val,b);
	for (int i=0; i<(int)(p-strlen(b)); i++) strcat(val,"0");
	// Fraction can be anything from 0 to 10^p , so can have less digits
	strcat(val,b);
}

// ----------------------------------------------------------------------------
// Based on the information read from the LoRa transceiver (or fake message)
// build a gateway message to send upstream.
//
// parameters:
// tmst: Timestamp to include in the upstream message
// buff_up: The buffer that is generated for upstream
// message: The payload message toincludein the the buff_up
// messageLength: The number of bytes received by the LoRa transceiver
//
// ----------------------------------------------------------------------------
int buildPacket(uint32_t tmst, uint8_t *buff_up, uint8_t *message, char messageLength, byte CE) {

	long SNR;
    int rssicorr;
	int prssi;											// packet rssi
	char cfreq[12] = {0};								// Character array to hold freq in MHz
	//int lastTmst = tmst;									// Following/according to spec
	int buff_index=0;
	
	uint8_t value = ReadRegister(REG_PKT_SNR_VALUE, CE);		// 0x19; 
    if( value & 0x80 ) {								// The SNR sign bit is 1
		// Invert and divide by 4
		value = ( ( ~value + 1 ) & 0xFF ) >> 2;
		SNR = -value;
    }
	else {
		// Divide by 4
		SNR = ( value & 0xFF ) >> 2;
	}
	
	prssi = ReadRegister(REG_PKT_RSSI, CE);				// read register 0x1A
    
	// Correction of RSSI value based on chip used.	
	if (sx1272) {
		rssicorr = 139;
	} else {											// Probably SX1276 or RFM95
		rssicorr = 157;
	}
			
	if (debug>=1) {
		printf("Packet RSSI: %i", prssi-rssicorr);
		printf(" RSSI: %i", ReadRegister(0x1B, CE)-rssicorr);
		printf(" SNR: %li", SNR);
		printf(" Length: %i", (int)messageLength);
		printf(" -> ");
		int i;
		for (i=0; i< messageLength; i++) {
					printf("%hi ", message[i]);
		}
		printf("\n");
	}
			
	int j;
	// XXX Base64 library is nopad. So we may have to add padding characters until
	// 	length is multiple of 4!
	//int encodedLen = base64_enc_len(messageLength);		// max 341
	//base64_encode(b64, (char *) message, messageLength);// max 341

	// pre-fill the data buffer with fixed fields
	buff_up[0] = PROTOCOL_VERSION;						// 0x01 still
	buff_up[3] = PKT_PUSH_DATA;							// 0x00

	// READ MAC ADDRESS OF ESP8266, and insert 0xFF 0xFF in the middle
	buff_up[4]  = ifr.ifr_hwaddr.sa_data[0];
	buff_up[5]  = ifr.ifr_hwaddr.sa_data[1];
	buff_up[6]  = ifr.ifr_hwaddr.sa_data[2];
	buff_up[7]  = 0xFF;
	buff_up[8]  = 0xFF;
	buff_up[9]  = ifr.ifr_hwaddr.sa_data[3];
	buff_up[10] = ifr.ifr_hwaddr.sa_data[4];
	buff_up[11] = ifr.ifr_hwaddr.sa_data[5];

	// start composing datagram with the header 
	uint8_t token_h = (uint8_t)rand(); 					// random token
	uint8_t token_l = (uint8_t)rand(); 					// random token
	buff_up[1] = token_h;
	buff_up[2] = token_l;
	buff_index = 12; 									// 12-byte binary (!) header

	// start of JSON structure that will make payload
	memcpy((void *)(buff_up + buff_index), (void *)"{\"rxpk\":[", 9);
	buff_index += 9;
	buff_up[buff_index] = '{';
	++buff_index;
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, "\"tmst\":%u", tmst);
	buff_index += j;
	ftoa((double)freq/1000000,cfreq,6);					// XXX This can be done better
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%s", 0, 0, cfreq);
	buff_index += j;
	memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":1", 9);
	buff_index += 9;
	memcpy((void *)(buff_up + buff_index), (void *)",\"modu\":\"LORA\"", 14);
	buff_index += 14;
	/* Lora datarate & bandwidth, 16-19 useful chars */
	switch (sf) {
		case SF7:
			memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF7", 12);
			buff_index += 12;
			break;
		case SF8:
                memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF8", 12);
                buff_index += 12;
                break;
		case SF9:
                memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF9", 12);
                buff_index += 12;
                break;
		case SF10:
                memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF10", 13);
                buff_index += 13;
                break;
		case SF11:
                memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF11", 13);
                buff_index += 13;
                break;
		case SF12:
                memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF12", 13);
                buff_index += 13;
                break;
		default:
                memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF?", 12);
                buff_index += 12;
	}
	memcpy((void *)(buff_up + buff_index), (void *)"BW125\"", 6);
	buff_index += 6;
	memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/5\"", 13);
	buff_index += 13;
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"lsnr\":%li", SNR);
	buff_index += j;
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"rssi\":%d,\"size\":%u", prssi-rssicorr, messageLength);
	buff_index += j;
	memcpy((void *)(buff_up + buff_index), (void *)",\"data\":\"", 9);
	buff_index += 9;

	// Use gBase64 library to fill in the data string
	//encodedLen = base64_enc_len(messageLength);			// max 341
	//j = base64_encode((char *)(buff_up + buff_index), (char *) message, messageLength);
	j = bin_to_b64((buff_up + buff_index), messageLength, (char *)message, TX_BUFF_SIZE);

	buff_index += j;
	buff_up[buff_index] = '"';
	++buff_index;

	// End of packet serialization
	buff_up[buff_index] = '}';
	++buff_index;
	buff_up[buff_index] = ']';
	++buff_index;
	
	// end of JSON datagram payload */
	buff_up[buff_index] = '}';
	++buff_index;
	buff_up[buff_index] = 0; 							// add string terminator, for safety

	if (debug>=1) {
		printf("RXPK:: %s\n", (char *)(buff_up + 12));			// DEBUG: display JSON payload
	}
            
	return(buff_index);
}

void SolveHostname(const char* p_hostname, uint16_t port, struct sockaddr_in* p_sin)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  char service[6] = { '\0' };
  snprintf(service, 6, "%hu", port);

  struct addrinfo* p_result = NULL;

  // Resolve the domain name into a list of addresses
  int error = getaddrinfo(p_hostname, service, &hints, &p_result);
  if (error != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
      exit(EXIT_FAILURE);
  }

  // Loop over all returned results
  for (struct addrinfo* p_rp = p_result; p_rp != NULL; p_rp = p_rp->ai_next) {
    struct sockaddr_in* p_saddr = (struct sockaddr_in*)p_rp->ai_addr;
    //printf("%s solved to %s\n", p_hostname, inet_ntoa(p_saddr->sin_addr));
    p_sin->sin_addr = p_saddr->sin_addr;
  }

  freeaddrinfo(p_result);
}


void SendUdp(char *msg, int length)
{
  for (std::vector<Server_t>::iterator it = servers.begin(); it != servers.end(); ++it) {
    if (it->enabled) {
      si_other.sin_port = htons(it->port);

if (debug>1) {printf ("Sending message to server: %s\n", it->address.c_str());
             // for (int i=0;i<length;i++) {
	     //   printf("%i:", msg[i]);
             // }
//		printf("\n");
}
      SolveHostname(it->address.c_str(), it->port, &si_other);
      if (sendto(s, (char *)msg, length, 0 , (struct sockaddr *) &si_other, slen)==-1) {
        Die("sendto()");
      }
    }
  }
}

// ----------------------------------------------------------------------------
// Receive a LoRa package over the air
//
// Receive a LoRa message and fill the buff_up char buffer.
// returns values: (none for now maybe later port to Maartens buildpacket
// //- returns the length of string returned in buff_up
// //- returns -1 when no message arrived.
// ----------------------------------------------------------------------------
bool ReceivePacket(byte CE) {

  long int SNR;
  int rssicorr, dio_port;
  bool ret = false;

  if (CE == 0)
    {
        dio_port = dio0;
    } else {
        dio_port = dio0_2;
    }

  if (digitalRead(dio_port) == 1) {
    uint8_t message[256];
    uint8_t length = 0;
    if (ReceivePkt(message, &length, CE)) {
      // OK got one
      ret = true;

      uint8_t value = ReadRegister(REG_PKT_SNR_VALUE, CE);

      if (CE == 0)
            {
              digitalWrite(ActivityLED_0, HIGH);
            } else {
              digitalWrite(ActivityLED_1, HIGH);
           }

      if (value & 0x80) { // The SNR sign bit is 1
        // Invert and divide by 4
        value = ((~value + 1) & 0xFF) >> 2;
        SNR = -value;
      } else {
        // Divide by 4
        SNR = ( value & 0xFF ) >> 2;
      }

      rssicorr = sx1272 ? 139 : 157;

      printf("CE%i Packet RSSI: %d, ", CE, ReadRegister(0x1A, CE) - rssicorr);
      printf("RSSI: %d, ", ReadRegister(0x1B,CE) - rssicorr);
      printf("SNR: %li, ", SNR);
      printf("Length: %hhu Message:", length);
      //for (int i=0; i<length; i++) {
       // char c = (char) message[i];
        //printf("%i.",c);
      //}
      //printf("\n");

      char buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet */
      int buff_index = 0;

      /* gateway <-> MAC protocol variables */
      //static uint32_t net_mac_h; /* Most Significant Nibble, network order */
      //static uint32_t net_mac_l; /* Least Significant Nibble, network order */

      /* pre-fill the data buffer with fixed fields */
      buff_up[0] = PROTOCOL_VERSION;
      buff_up[3] = PKT_PUSH_DATA;

      /* process some of the configuration variables */
      //net_mac_h = htonl((uint32_t)(0xFFFFFFFF & (lgwm>>32)));
      //net_mac_l = htonl((uint32_t)(0xFFFFFFFF &  lgwm  ));
      //*(uint32_t *)(buff_up + 4) = net_mac_h;
      //*(uint32_t *)(buff_up + 8) = net_mac_l;

      buff_up[4] = (uint8_t)ifr.ifr_hwaddr.sa_data[0];
      buff_up[5] = (uint8_t)ifr.ifr_hwaddr.sa_data[1];
      buff_up[6] = (uint8_t)ifr.ifr_hwaddr.sa_data[2];
      buff_up[7] = 0xFF;
      buff_up[8] = 0xFF;
      buff_up[9] = (uint8_t)ifr.ifr_hwaddr.sa_data[3];
      buff_up[10] = (uint8_t)ifr.ifr_hwaddr.sa_data[4];
      buff_up[11] = (uint8_t)ifr.ifr_hwaddr.sa_data[5];

      /* start composing datagram with the header */
      uint8_t token_h = (uint8_t)rand(); /* random token */
      uint8_t token_l = (uint8_t)rand(); /* random token */
      buff_up[1] = token_h;
      buff_up[2] = token_l;
      buff_index = 12; /* 12-byte header */

      // TODO: tmst can jump is time is (re)set, not good.
      struct timeval now;
      gettimeofday(&now, NULL);
      uint32_t tmst = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);

      // Encode payload.
      char b64[BASE64_MAX_LENGTH];
      bin_to_b64((uint8_t*)message, length, b64, BASE64_MAX_LENGTH);

      // Build JSON object.
      /*rapidjson::StringBuffer sb;
      rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
      writer.StartObject();
      writer.String("rxpk");
      writer.StartArray();
      writer.StartObject();
      writer.String("tmst");
      writer.Uint(tmst);
      writer.String("freq");
      if (CE == 0) {
        writer.Double((double)freq / 1000000);
        writer.String("chan");
        writer.Uint(0);
      } else {
        writer.Double((double)freq_2 / 1000000);
        writer.String("chan");
        writer.Uint(1);
      }
      writer.String("rfch");
      writer.Uint(0);
      writer.String("stat");
      writer.Uint(1);
      writer.String("modu");
      writer.String("LORA");
      writer.String("datr");
      char datr[] = "SFxxBWxxx";
      snprintf(datr, strlen(datr) + 1, "SF%hhuBW%hu", sf, bw);
      writer.String(datr);
      writer.String("codr");
      writer.String("4/5");
      writer.String("rssi");
      writer.Int(ReadRegister(0x1A, CE) - rssicorr);
      writer.String("lsnr");
      writer.Double(SNR); // %li.
      writer.String("size");
      writer.Uint(length);
      writer.String("data");
      writer.String(b64);
      writer.EndObject();
      writer.EndArray();
      writer.EndObject();
*/
      std::string json = "TODO";
      printf("rxpk update: %s\n", json.c_str());

      // Build and send message.
      memcpy(buff_up + 12, json.c_str(), json.size());
      SendUdp(buff_up, buff_index + json.size());

      fflush(stdout);
    }
  }
  return ret;
}

double difftimespec(struct timespec end, struct timespec beginning) {
        double x;

        x = 1E-9 * (double)(end.tv_nsec - beginning.tv_nsec);
        x += (double)(end.tv_sec - beginning.tv_sec);

        return x;
}

