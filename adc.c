#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "GPIOcontrol.h"
#include "SPI.h"
#include <fcntl.h>

const int busSpeedHz = 12000000;
const int csDelay = 0; // how long to delay after the last bit transfer before deselecting the device 
const int wordLength = 16;

int cs_pin = 2 * 32 + 12; // P9.15a on BB AI
//int cs_pin = 1 * 32 + 16; // P9.15 on BBB
//int cs_pin = 0 * 32 + 5; // P1.6 on PB
int cs_fd;
int transfer16_txrx(uint16_t oword, uint16_t* iword) {
	int transmissionLength = 2;
	gpio_write(cs_fd, 0);
	usleep(30);
	int ret = SPIDEV_transfer((unsigned char*)&oword, (unsigned char*)iword, transmissionLength);
	usleep(30);
	gpio_write(cs_fd, 1);
	return ret;
}
int transfer16_tx(uint16_t oword) {
	uint16_t iword;
	return transfer16_txrx(oword, &iword);
}
#define AD7699_CFG_MASK       0xF120 // Mask for config update, unipolar, full BW
#define AD7699_CHANNEL_OFFSET 9      // 7 bits offset of a 14-bit left-justified word
#define AD7699_SEQ_OFFSET     3      // sequencer (0 = disable, 3 = scan all)

int main()
{ 
	gpio_export(cs_pin);
	cs_fd = gpio_fd_open(cs_pin, O_WRONLY);
	gpio_set_dir(cs_pin, OUTPUT_PIN);
	gpio_write(cs_fd, 1); // de-assert

	if (SPIDEV_init("/dev/spidev1.0", 0, busSpeedHz, SPI_SS_LOW, csDelay, wordLength, SPI_MODE3) == -1)
	{
		printf("initialization failed\r\n");
		return -1;
	}
	else
		printf("initialized - READY\r\n");

	uint16_t oword;
	uint16_t iword;
	// MOV r2, AD7699_CFG_MASK | (0 << AD7699_CHANNEL_OFFSET) | (0 << AD7699_SEQ_OFFSET)
	oword = AD7699_CFG_MASK | (0 << AD7699_CHANNEL_OFFSET) | (0 << AD7699_SEQ_OFFSET);
	//SPIDEV1_transfer((unsigned char*)&oword, (unsigned char*)&iword, transmissionLength);
	transfer16_tx(oword);
	printf("AD7699_config: %#8x\n", iword);
	int numChannels = 8;
	int adcOffsetChannels = 2;
	// Read ADC ch0 and ch1: result is always adcOffset samples behind so
	// start here and discard results
	for(int ch = 0; ch < adcOffsetChannels; ++ch) {
		oword = AD7699_CFG_MASK | (ch << AD7699_CHANNEL_OFFSET);
		transfer16_txrx(oword, &iword);
	}
	while (1)
	{
		uint16_t reads[numChannels];
		for(int ch = 0; ch < numChannels; ++ch) {
			//MOV r2, AD7699_CFG_MASK | (0x00 << AD7699_CHANNEL_OFFSET)
			oword = AD7699_CFG_MASK | (((ch + adcOffsetChannels) % numChannels) << AD7699_CHANNEL_OFFSET);

			if(!transfer16_txrx(oword, &iword) == 0)
			{
				printf("(Main)spidev1.0: Transaction Failed\r\n");
				return -1;
			}
			//printf("AD7699_read: %#8x\n", iword);
			reads[ch] = iword;
		}
		for(int ch = 0; ch < numChannels; ++ch)
			printf("[ch %d]: %5u, ", ch, reads[ch]);
		printf("\n");
		usleep(100000);
	}
	gpio_dismiss(cs_fd, cs_pin);
	return 0;
}

