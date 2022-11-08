#!/bin/sh


git diff master | grep '\+target_stm32' |
sed 's@.*(\(.*\)).*@+ \1@' | sort -u





