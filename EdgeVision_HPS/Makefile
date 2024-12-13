TARGET = SOBEL_HPS

ALT_DEVICE_FAMILY ?= soc_cv_av
SOCEDS_ROOT ?= $(SOCEDS_DEST_ROOT)
HWLIBS_ROOT = C:/intelFPGA/20.1/embedded/ip/altera/hps/altera_hps/hwlib
CROSS_COMPILE = C:/intelFPGA/20.1/embedded/host_tools/linaro/gcc/gcc-linaro-7.5.0-2019.12-i686-mingw32_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CFLAGS = -g -Wall -D$(ALT_DEVICE_FAMILY) -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY) -I$(HWLIBS_ROOT)/include/
LDFLAGS = -g -Wall 
CC = $(CROSS_COMPILE)gcc
ARCH = arm

# List both source files
SRCS = main.c EdgeVision.c
# Generate object file names from source files
OBJS = $(SRCS:.c=.o)

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~ *.txt output/*