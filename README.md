# Bela no PRU

Some example code to use the Bela ADC/DAC and audio codec without Bela's PRU code.
This could come in handy if you are using the Bela cape with the BB-AUDI-02 overlay on the BeagleBone or PocketBeagle, or the Bela dtb on the Beaglebone AI.

Use `./build_all.sh` to build the adc/dac examples. Then run the examples with `sudo` (as it's required to export the chip select GPIOs). This requires having the Bela code on the board at the usual location `~/Bela`.

To change the soundcard's settings (e.g.: gain, output level) when running with ALSA, use `alsamixer`.

