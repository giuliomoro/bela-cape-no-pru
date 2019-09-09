#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "SPI.h"
#include "GPIOcontrol.h"
#include <fcntl.h>

const int busSpeedHz = 12000000;
const int wordLength = 32;

int cs_pin = 6*32 + 17; // P9.17a on BB AI
//int cs_pin = 0 * 32 + 5; // P9.17 on BBB
int cs_fd;
int transfer_txrx(uint32_t oword, uint32_t* iword) {
	int transmissionLength = 4;
	gpio_write(cs_fd, 0);
	usleep(30);
	int ret = SPIDEV_transfer((unsigned char*)&oword, (unsigned char*)iword, transmissionLength);
	usleep(30);
	gpio_write(cs_fd, 1);
	return ret;
}

int transfer_tx(uint32_t oword) {
	uint32_t iword;
	return transfer_txrx(oword, &iword);
}

#define AD5668_COMMAND_OFFSET 24
#define AD5668_ADDRESS_OFFSET 20
#define AD5668_DATA_OFFSET    4
#define AD5668_REF_OFFSET     0

int main()
{ 
	if(gpio_export(cs_pin))
		return -1;
	cs_fd = gpio_fd_open(cs_pin, O_WRONLY);
	if(cs_fd < 0)
		return -1;
	if(gpio_set_dir(cs_pin, OUTPUT_PIN))
		return -1;
	if(gpio_write(cs_fd, 1)) // de-assert
		return -1;

	if (SPIDEV_init("/dev/spidev1.0", 0, busSpeedHz, SPI_SS_LOW, 0, wordLength, SPI_MODE3) == -1)
	{
		printf("initialization failed\r\n");
		return -1;
	}
	else
		printf("initialized - READY\r\n");

	uint32_t oword;
	// DAC power-on reset sequence
	// MOV r2, (0x07 << AD5668_COMMAND_OFFSET)
	oword = 0x07 << AD5668_COMMAND_OFFSET;
	transfer_tx(oword);
	// Enable DAC internal reference
	// MOV r2, (0x08 << AD5668_COMMAND_OFFSET) | (0x01 << AD5668_REF_OFFSET)
	oword = (0x08 << AD5668_COMMAND_OFFSET) | (0x01 << AD5668_REF_OFFSET);
	transfer_tx(oword);

	int count = 0;
	while (1)
	{
		unsigned char channel = 0xf; // 0xf for all channels
		uint16_t reg_dac_data = 1000 * count;
		oword = ((0x3 << AD5668_COMMAND_OFFSET) | (channel << AD5668_ADDRESS_OFFSET) | (1000*count << AD5668_DATA_OFFSET));
		if (transfer_tx(oword) == 0)
			;
		else
			printf("(Main)spidev1.0: Transaction Failed\r\n");
		count++;
		if(count > 64)
			count = 0;
		usleep(10000);
	}
	gpio_dismiss(cs_fd, cs_pin);
	return 0;
}

