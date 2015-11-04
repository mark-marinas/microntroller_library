# connect to the J-Link gdb server
target remote localhost:2331

# Set gdb server to little endian
monitor endian little

# Set JTAG speed to 1000 kHz
monitor speed 1000

# Reset the target
monitor reset
monitor sleep 100

# Set JTAG speed in khz
monitor speed auto

# Vector table placed in RAM
monitor writeu32 0xE000ED08 = 0x10000000
