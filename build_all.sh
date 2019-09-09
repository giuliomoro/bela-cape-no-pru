#!/bin/bash -x

BELA_FOLDER=~/Bela
COMMON_FILES_INCLUDES="BeagleBone-SPI-Library/SPI_Library/SPI.c -IBeagleBone-SPI-Library/SPI_Library/ $BELA_FOLDER/core/GPIOcontrol.cpp -I$BELA_FOLDER/include"

gcc $COMMON_FILES_INCLUDES adc.c -o adc
gcc $COMMON_FILES_INCLUDES dac.c -o dac

