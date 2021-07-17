//HardwareSerial.cpp - A3ides/STM32
#include <Arduino.h>
#include "buffered_serial.hpp"
#include "cmsis_os.h"
#include "bsod.h"
#include "HardwareSerial.h"
#include "main.h"

using namespace buddy::hw;

HardwareSerial::HardwareSerial(void *peripheral) {
}

void HardwareSerial::begin(unsigned long baud) {
    BufferedSerial::uart2.Open();
}

void HardwareSerial::begin(unsigned long baud, byte config) {
}

int HardwareSerial::available(void) {
    bsod("HardwareSerial::available() not implemented");
    return 0;
}

int HardwareSerial::peek(void) {
    return -1;
}

int HardwareSerial::read(void) {
    char ch;
    int read = BufferedSerial::uart2.Read(&ch, 1);
    return read ? ch : -1;
}

void HardwareSerial::flush() {
    BufferedSerial::uart2.Flush();
}

size_t HardwareSerial::write(const uint8_t c) {
    return BufferedSerial::uart2.Write((const char *)&c, 1);
}

size_t HardwareSerial::write(const uint8_t *buffer, size_t size) {
    return BufferedSerial::uart2.Write((const char *)buffer, size);
}

HardwareSerial::operator bool() {
    return true;
}


#ifdef USE_UART6_SERIAL

extern "C" {
extern UART_HandleTypeDef huart6;
}

USART_RECEIVETYPE usartType;

void HardwareSerial6::begin(unsigned long baud) {
    // Do nothing, the MX_UART_Init() function takes care of this.
}

int HardwareSerial6::read(void) {
	 unsigned char chr;
	 if ((usartType.RX_Index >= RX_LEN) || (usartType.RX_Index >= usartType.RX_Size)){
		 //something wrong, skip command and reset buffer
		 usartType.RX_Index=0;
		 usartType.RX_flag=0;
		 usartType.RX_Size=0;
		 return -1;
	 }
 if ((usartType.RX_Index < usartType.RX_Size)  && (usartType.RX_flag > 0))   	// Sign of received data
		    {
		         //usartType.RX_flag=0;	//Clear the received flag
		         chr=usartType.RX_pData[usartType.RX_Index];

		         usartType.RX_Index++;

		         return chr;
		    }
	    else

	    {
	    	usartType.RX_flag=0;	//Clear the received flag
	    	return -1;
	    }

}

int HardwareSerial6::available() {
	//return (huart6.Instance->SR & UART_FLAG_RXNE) > 0;
	//USART_RECEIVETYPE *usartType){
	return usartType.RX_flag;
}

void HardwareSerial6::flush() {
    // Nothing to do here, the TX is blocking.
}

size_t HardwareSerial6::write(uint8_t c) {
    if (HAL_UART_Transmit(&huart6, &c, 1, HAL_MAX_DELAY) == HAL_OK) {
        return 1;
    } else {
        return 0;
    }
}
size_t HardwareSerial6::write(uint8_t *buffer, size_t size) {
    if (HAL_UART_Transmit(&huart6, buffer, size, HAL_MAX_DELAY) == HAL_OK) {
        return size;
    } else {
        return 0;
    }
}

HardwareSerial6 SerialUART6(USART6);

#endif // SERIAL_PORT == 6

HardwareSerial Serial3(USART3);
