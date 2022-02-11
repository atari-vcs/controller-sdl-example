#
# Copyright 2022 Collabora, Ltd.
#
# SPDX-License-Identifier: MIT
#

all: controller

controller: controller_example.o
	gcc -Wall -Wextra -pedantic -O3 -flto $< -o $@ -lSDL2

%.o: %.c
	gcc -c -Wall -Wextra -pedantic -O3 -flto $< 

clean:
	rm -f *.o controller

.PHONY: all clean
