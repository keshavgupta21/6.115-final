# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\info\Desktop\Spring 2019 Classes\6.115\6.115-final\PSoCCreator\Final Project\FinalEmulator.cydsn\FinalEmulator.cyprj
# Date: Thu, 09 May 2019 02:25:27 GMT
#set_units -time ns
create_clock -name {clk_timer(routed)} -period 166664.47368421051 -waveform {0 83332.2368421053} [list [get_pins {ClockBlock/dclk_2}]]
create_clock -name {CyILO} -period 10000 -waveform {0 5000} [list [get_pins {ClockBlock/ilo}] [get_pins {ClockBlock/clk_100k}] [get_pins {ClockBlock/clk_1k}] [get_pins {ClockBlock/clk_32k}]]
create_clock -name {CyIMO} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyPLL_OUT} -period 15.350877192982454 -waveform {0 7.67543859649123} [list [get_pins {ClockBlock/pllout}]]
create_clock -name {CyMASTER_CLK} -period 15.350877192982454 -waveform {0 7.67543859649123} [list [get_pins {ClockBlock/clk_sync}]]
create_generated_clock -name {CyBUS_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 2 3} [list [get_pins {ClockBlock/clk_bus_glb}]]
create_generated_clock -name {VGA_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 9 17} [list [get_pins {ClockBlock/dclk_glb_0}]]
create_generated_clock -name {spi_oled_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 3 5} [list [get_pins {ClockBlock/dclk_glb_1}]]
create_generated_clock -name {clk_timer} -source [get_pins {ClockBlock/clk_sync}] -edges {1 10857 21715} [list [get_pins {ClockBlock/dclk_glb_2}]]
create_generated_clock -name {clk_snd} -source [get_pins {ClockBlock/clk_sync}] -edges {1 65143 130287} -nominal_period 1000002.192982456 [list [get_pins {ClockBlock/dclk_glb_3}]]


# Component constraints for C:\Users\info\Desktop\Spring 2019 Classes\6.115\6.115-final\PSoCCreator\Final Project\FinalEmulator.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\info\Desktop\Spring 2019 Classes\6.115\6.115-final\PSoCCreator\Final Project\FinalEmulator.cydsn\FinalEmulator.cyprj
# Date: Thu, 09 May 2019 02:25:20 GMT
