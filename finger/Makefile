EXE_NAME = finger

SDK_INCLUDES = ../../../Include
OUT_DIR ?= .
LIB_OUT_DIR ?= /usr/lib

CCFLAGS = -g -Wall -I$(SDK_INCLUDES) $(CFLAGS)

# Función para obtener la ruta real del enlace
getlink = $(shell cygpath -wm `readlink -f $(1)`)

ifeq ($(findstring arm,$(CFLAGS))$(findstring CYGWIN,$(shell uname)),armCYGWIN)
	# La herramienta Code Sourcery bajo Cygwin no puede desreferenciar enlaces simbólicos, es necesario especificar la biblioteca real para la vinculación
	LDFLAGS = -lpthread -lm -lc $(CFLAGS) $(call getlink,$(LIB_OUT_DIR)/libdpfpdd.so) $(call getlink,$(LIB_OUT_DIR)/libdpfj.so)
else
	LDFLAGS = -lpthread -lm -lc -ldl $(CFLAGS) -L$(LIB_OUT_DIR) -ldpfpdd -ldpfj
endif

LDFLAGS += -lmicrohttpd  # Añade esta línea para enlazar con libmicrohttpd 

OBJS = fingerprint_server.o fingerprint_capture.o fingerprint_selection.o

all: $(OBJS)
	mkdir -p $(OUT_DIR)
	$(CC) $(OBJS) -Wl,--no-as-needed $(LDFLAGS) -o $(OUT_DIR)/$(EXE_NAME)

all_debug: all

clean:
	rm -f $(OUT_DIR)/$(EXE_NAME) *.o *~

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

.PHONY: install
