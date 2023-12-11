#!/usr/bin/python3
import wiringpi as GPIO

from evdev import InputDevice, categorize, ecodes
import time

gamepad = InputDevice('/dev/input/event8') # make sure this is the correct input device

aBtn = 304
bBtn = 305
xBtn = 307
yBtn = 308
up = 115
down = 114
left = 165
right = 163
playpause = 164
ls_x_last = 0
ls_y_last = 0
rs_x_last = 0
rs_y_last = 0
lt_last = 0
rt_last = 0

print(gamepad)

CENTER_TOLERANCE = 1000
STICK_MAX = 65536
TRIGGER_MAX = 1024

axis = {
	ecodes.ABS_X: 'ls_x',
	ecodes.ABS_Y: 'ls_y',
	ecodes.ABS_Z: 'rs_x',
	ecodes.ABS_RZ: 'rs_y',
	ecodes.ABS_BRAKE: 'lt',
	ecodes.ABS_GAS: 'rt',
	ecodes.ABS_HAT0X: 'dpad_x',
	ecodes.ABS_HAT0Y: 'dpad_y'
}

center = {
	'ls_x': STICK_MAX/2,
	'ls_y': STICK_MAX/2,
	'rs_x': STICK_MAX/2,
	'rs_y': STICK_MAX/2,
	'lt': 0,
	'rt': 0,
	'dpad_x': 0,
	'dpad_y': 0
}

last = {
	'ls_x': STICK_MAX/2,
	'ls_y': STICK_MAX/2,
	'rs_x': STICK_MAX/2,
	'rs_y': STICK_MAX/2
}


DEVICE_ID = 0x08

fd = GPIO.wiringPiI2CSetup(DEVICE_ID)
if fd == -1:
	print("Failed to initialize I2C communication.")
	exit()

print("I2C communication successfully set up.")

data_to_send = 17
GPIO.wiringPiI2CWrite(fd, data_to_send)
print("Sent data:", data_to_send)

received_data = GPIO.wiringPiI2CRead(fd)
print("Data received:", received_data)

if received_data == data_to_send:
	print("Success!")
	

#input("press enter to start")
count = 0

for event in gamepad.read_loop():
#	print(categorize(event))
#	input("waiting for keypress")
#	if event.type == ecodes.EV_KEY:
#		if event.value == 1:
#			if event.code == aBtn:
#				print("A")
#			elif event.code == bBtn:
#				print("B")
#			elif event.code == playpause:
#				print("Play/Pause")
#			elif event.code == up:
#				print("up")
#			elif event.code == down:
#				print("down")
#			elif event.code == left:
#				print("left")
#			elif event.code == right:
#				print("right")
	if event.type == ecodes.EV_ABS:
		if axis[event.code] in ['ls_x','ls_y','rs_x','rs_y','lt','rt','dpad_x','dpad_y']:
			last[axis[event.code]] = event.value
			value = event.value - center[axis[event.code]]
			if axis[event.code] == 'ls_x':
				ls_x_last = int((value+center['ls_x'])/512)
				#print(ls_x_last)
			if axis[event.code] == 'ls_y':
				ls_y_last = int((value+center['ls_y'])/512)
			if axis[event.code] == 'rs_x':
				rs_x_last = int((value+center['rs_x'])/512)
			if axis[event.code] == 'rs_y':
				rs_y_last = int((value+center['rs_y'])/512)
			if axis[event.code] == 'lt':
				lt_last = int(value / 8)
			if axis[event.code] == 'rt':
				rt_last = int(value / 8)
			if axis[event.code] == 'dpad_x':
				dpad_x_last = value + 1
			if axis[event.code] == 'dpad_y':
				dpad_y_last = value + 1
				
				
			if count % 16 == 0:
				GPIO.wiringPiI2CWrite(fd, 255)
				send_bytes = bytes([ls_x_last, ls_y_last, rs_x_last, rs_y_last, lt_last, rt_last])
				GPIO.wiringPiI2CWrite(fd, send_bytes[0])
				GPIO.wiringPiI2CWrite(fd, send_bytes[1])
				GPIO.wiringPiI2CWrite(fd, send_bytes[2])
				GPIO.wiringPiI2CWrite(fd, send_bytes[3])
				#GPIO.wiringPiI2CWrite(fd, send_bytes[4])
				GPIO.wiringPiI2CWrite(fd, send_bytes[5])
				count = 0
				
				
			count += 1
	
