EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:misc
LIBS:sop8breakout-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCP6002 IC1
U 1 1 55721724
P 5800 3700
F 0 "IC1" H 6100 4000 60  0000 C CNN
F 1 "MCP6002" V 5800 3700 60  0000 C CNN
F 2 "sop8:SOP-8" H 5800 3700 60  0001 C CNN
F 3 "" H 5800 3700 60  0000 C CNN
	1    5800 3700
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X04 P1
U 1 1 55721A0D
P 4800 3700
F 0 "P1" H 4800 3950 50  0000 C CNN
F 1 "CONN_01X04" V 4900 3700 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04" H 4800 3700 60  0001 C CNN
F 3 "" H 4800 3700 60  0000 C CNN
	1    4800 3700
	-1   0    0    -1  
$EndComp
$Comp
L CONN_01X04 P2
U 1 1 55721A4D
P 6800 3700
F 0 "P2" H 6800 3950 50  0000 C CNN
F 1 "CONN_01X04" V 6900 3700 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04" H 6800 3700 60  0001 C CNN
F 3 "" H 6800 3700 60  0000 C CNN
	1    6800 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3550 5150 3550
Wire Wire Line
	5150 3650 5000 3650
Wire Wire Line
	5000 3750 5150 3750
Wire Wire Line
	5150 3850 5000 3850
Wire Wire Line
	6450 3550 6600 3550
Wire Wire Line
	6600 3650 6450 3650
Wire Wire Line
	6450 3750 6600 3750
Wire Wire Line
	6600 3850 6450 3850
$EndSCHEMATC
