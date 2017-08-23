#! /usr/bin/env python

###############################################################################
# BBB_BasicMotorControl.py
#
# Basic test of motor control using the SparkFun TB6612FNG breakout board
#  http://sfe.io/p9457
# 
# Requires - Adafruit BeagleBone IO Python library
#
# NOTE: Any plotting is set up for output, not viewing on screen.
#       So, it will likely be ugly on screen. The saved PDFs should look
#       better.
#
# Created: 01/09/15
#   - Joshua Vaughan
#   - joshua.vaughan@louisiana.edu
#   - http://www.ucs.louisiana.edu/~jev9637
#
# Modified:
#   *
#
###############################################################################

import Adafruit_BBIO.GPIO as GPIO
import Adafruit_BBIO.PWM as PWM
import time
STBY = 'P8_9'       # STBY pin on the breakout, must go low to enable motion
A01 = 'P8_7'        # A01 pin on board, controls direction along with A02
A02 = 'P8_8'        # A02 pin on board, controls direction along with A01
# PWMA = 'P8_13'      # PWMA pin on board, controls the speed of Motor A

# # Set up the pins - These are mutable, but *don't* change them
# STBY = 'P8_45'       # STBY pin on the breakout, must go low to enable motion
# A01 = 'P8_46'        # A01 pin on board, controls direction along with A02
# A02 = 'P8_43'        # A02 pin on board, controls direction along with A01
PWMA = 'P9_14'      # PWMA pin on board, controls the speed of Motor A

# Set up the GPIO pins as output
GPIO.setup(STBY, GPIO.OUT)
GPIO.setup(A01, GPIO.OUT)
GPIO.setup(A02, GPIO.OUT)
# GPIO.setup('P9_22', GPIO.OUT)

# while True:
# 	GPIO.output('P9_22', GPIO.LOW)
# 	print'low'
# 	time.sleep(3)
# 	GPIO.output('P9_22', GPIO.HIGH)
# 	print 'high'
# 	time.sleep(3)

# Standby pin should go high to enable motion
GPIO.output(STBY, GPIO.HIGH)

# A01 and A02 have to be opposite to move, toggle to change direction
GPIO.output(A01, GPIO.HIGH)
GPIO.output(A02, GPIO.LOW)

# Start the motor
# PWM.start(channel, duty, freq=2000, polarity=0)
PWM.start(PWMA, 50)

# optionally, change the PWM frequency and polarity from their defaults
# PWM.start("P9_14", 50, 1000, 1)

# Run the motor for 10s
time.sleep(5)

# Stop the motor and cleanup the PWM
PWM.stop(PWMA)
PWM.cleanup()

# Make standby pin low to go back into standby mode
GPIO.output(STBY, GPIO.LOW)