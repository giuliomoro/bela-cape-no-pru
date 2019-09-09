#!/bin/bash

arecord -r 48000 -f S24_LE -c 2 --period-size=64 --buffer-size 64 | aplay -r 48000 -f S24_LE -c 2 --period-size=64 --buffer-size 64 -
