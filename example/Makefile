
ASSEMBLE_SRC_ROOT := .
TB_ROOT := .

IFLAG    +=  -g -DHLS_SIM
CC        = verilator
MODULE    = scoreboard
OUT_DIR   = obj_dir
GTK       = gtkwave
WAVE_FILE = waveform.vcd

all: compile build run 

compile:
	$(CC) --cc $(MODULE).v --top-module $(MODULE)

build:
	$(CC) -Wall --trace --exe --build -cc $(MODULE).cpp $(MODULE).v

run:
	./$(OUT_DIR)/V$(MODULE)

clean:
	rm -rf $(OUT_DIR)

wave:
	$(GTK) $(WAVE_FILE)