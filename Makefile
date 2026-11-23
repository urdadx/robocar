SKETCH := firmware/robocar
FQBN   := esp32:esp32:esp32
PORT   ?= $(firstword $(wildcard /dev/ttyUSB* /dev/ttyACM*))
CLI    ?= arduino-cli

.PHONY: build flash monitor secrets

build:
	$(CLI) compile --fqbn $(FQBN) --build-path build $(SKETCH)

flash: build
	@test -n "$(PORT)" || (echo "No ESP32 found. Plug it in over USB, or pass PORT=/dev/ttyUSB0" && exit 1)
	$(CLI) upload --fqbn $(FQBN) --input-dir build -p $(PORT) $(SKETCH)

monitor:
	@test -n "$(PORT)" || (echo "No ESP32 found. Plug it in over USB, or pass PORT=/dev/ttyUSB0" && exit 1)
	$(CLI) monitor -p $(PORT) -c baudrate=115200

secrets: $(SKETCH)/secrets.h

$(SKETCH)/secrets.h:
	cp $(SKETCH)/secrets.example.h $@
	@echo "Created $@. Put your Wi-Fi name and password in it."
