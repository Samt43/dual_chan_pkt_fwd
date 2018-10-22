#include "ILoraModem.h"
#include "LoraModem.h"

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define SPI_CHANNEL 0


std::unique_ptr<LoraModem> LoraModemBuilder::CreateModem()
{
    
    return std::make_unique<LoraModem>();
}

LoraModem::LoraModem()
{
    
}

LoraModem::~LoraModem()
{
    
}

bool LoraModem::Start(const Configuration& configuration, LoraModemObserver& observer)
{
  // Init WiringPI
  wiringPiSetup() ;
  pinMode(pins.ss, OUTPUT);
  pinMode(pins.dio0, INPUT);
  pinMode(pins.dio1, INPUT);
  pinMode(pins.rst, OUTPUT);

  // Init SPI
  wiringPiSPISetup(SPI_CHANNEL, 500000);

  // Setup LORA
  digitalWrite(pins.rst, HIGH);
  delay(100);
  digitalWrite(pins.rst, LOW);
  delay(1000);   
    
    

	_state = S_INIT;
	initLoraModem();

    _state = S_RX;
	rxLoraModem();
	
    
	// init interrupt handlers, which are shared for GPIO15 / D8, 
	// we switch on HIGH interrupts
	if (pins.dio0 == pins.dio1) {
        wiringPiISR (pins.dio0, INT_EDGE_RISING, Interrupt);
	}
	// Or in the traditional Comresult case
	else {
        wiringPiISR (pins.dio0, INT_EDGE_RISING, Interrupt_0);
        wiringPiISR (pins.dio1, INT_EDGE_RISING, Interrupt_1);	
	}
    
	if (_cad) {
		_state = S_SCAN;
		cadScanner();										// Always start at SF7
	}
	
	while(1)
        {
	    if (_state == S_RXDONE) {
		eventHandler();							// Is S_RXDONE read a message

	    }   
	}	

}

bool LoraModem::SendPacket(const nlohmann::json& json)
{
    
}
