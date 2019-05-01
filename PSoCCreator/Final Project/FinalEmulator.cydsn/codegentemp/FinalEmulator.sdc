# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\info\Desktop\Spring 2019 Classes\6.115\6.115-final\PSoCCreator\Final Project\FinalEmulator.cydsn\FinalEmulator.cyprj
# Date: Wed, 01 May 2019 04:41:39 GMT
#set_units -time ns
create_clock -name {clk_timer(routed)} -period 16662597.65625 -waveform {0 8331298.828125} [list [get_pins {ClockBlock/dclk_1}]]
create_clock -name {CyXTAL_32kHz} -period 30517.578125 -waveform {0 15258.7890625} [list [get_pins {ClockBlock/clk_32k_xtal}]]
create_clock -name {CyILO} -period 1000000 -waveform {0 500000} [list [get_pins {ClockBlock/ilo}] [get_pins {ClockBlock/clk_100k}] [get_pins {ClockBlock/clk_1k}] [get_pins {ClockBlock/clk_32k}]]
create_clock -name {CyIMO} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyPLL_OUT} -period 15.350877192982454 -waveform {0 7.67543859649123} [list [get_pins {ClockBlock/pllout}]]
create_clock -name {CyMASTER_CLK} -period 15.350877192982454 -waveform {0 7.67543859649123} [list [get_pins {ClockBlock/clk_sync}]]
create_generated_clock -name {CyBUS_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 2 3} [list [get_pins {ClockBlock/clk_bus_glb}]]
create_generated_clock -name {VGA_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 9 17} [list [get_pins {ClockBlock/dclk_glb_0}]]
create_generated_clock -name {clk_timer} -source [get_pins {ClockBlock/clk_sync}] -edges {1 1085451 2170899} -nominal_period 16662597.65625 [list [get_pins {ClockBlock/dclk_glb_1}]]


# Component constraints for C:\Users\info\Desktop\Spring 2019 Classes\6.115\6.115-final\PSoCCreator\Final Project\FinalEmulator.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\info\Desktop\Spring 2019 Classes\6.115\6.115-final\PSoCCreator\Final Project\FinalEmulator.cydsn\FinalEmulator.cyprj
# Date: Wed, 01 May 2019 04:41:36 GMT
