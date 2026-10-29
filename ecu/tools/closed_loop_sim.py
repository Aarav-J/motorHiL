#!/usr/bin/env python3
"""
Software-in-the-loop motor simulation for the STM32F446RE ECU project.

Reads the STM32's UART telemetry line (needs a "duty=<percent>" field --
see the ENCODER_PROJECT notes), runs a simple first-order-lag motor model
driven by that commanded duty cycle, and reprograms the AD2's Pattern
Generator in real time so the simulated speed actually responds to the
STM32's PWM output -- closing the loop the AD2 can't close on its own.

Wiring (unchanged from the open-loop encoder test):
    AD2 DIO0 -> Nucleo PA6  (TIM3_CH1, "D12")
    AD2 DIO1 -> Nucleo PA7  (TIM3_CH2, "D11")
    AD2 GND  -> Nucleo GND

Requires: pyserial (already installed), WaveForms app installed (for the
dwf shared library + dwfconstants.py it ships with).
"""

import ctypes
import os
import re
import sys
import time
from os import sep
from sys import path, platform

import serial

# ----------------------------------------------------------------------
# Config -- adjust these for your setup
# ----------------------------------------------------------------------

SERIAL_PORT = "/dev/tty.usbmodem1403"  # check `ls /dev/tty.usbmodem*`
SERIAL_BAUD = 115200

DIO_A = 0  # -> PA6 / TIM3_CH1
DIO_B = 1  # -> PA7 / TIM3_CH2

ENCODER_CPR = 4.0  # must match ENCODER_CPR in main.c -- 1 pattern cycle = 1 "revolution"

# Simple first-order-lag motor model: speed chases a duty-proportional
# target with time constant TAU_S. Not a real motor's dynamics -- a
# stand-in good enough to get genuine step-response-shaped behavior
# instead of the AD2's previous "speed changes only when you touch
# WaveForms" open-loop behavior.
MAX_RPM = 3000.0   # simulated speed at 100% duty
TAU_S = 0.5        # seconds -- how snappy the simulated motor is; raise
                    # for a sluggish motor+load, lower for a twitchy one

MIN_FREQ_HZ = 0.1   # floor to avoid a divide-by-zero in the AD2 divider math

DUTY_RE = re.compile(r"duty=(-?\d+\.?\d*)")


# ----------------------------------------------------------------------
# AD2 / dwf setup -- library-loading pattern matches a script confirmed
# working for this AD2 on this machine (os.path.exists() check before
# LoadLibrary, app-bundled path tried first). API calls below (DividerSet/
# CounterSet/etc.) verified separately against Digilent's own WF_SDK
# source (WF_SDK/pattern.py, WF_SDK/device.py).
# ----------------------------------------------------------------------

if platform.startswith("darwin"):
    _candidates = [
        sep + "Applications" + sep + "WaveForms.app" + sep + "Contents" + sep + "Frameworks" + sep + "dwf.framework" + sep + "dwf",
        sep + "Library" + sep + "Frameworks" + sep + "dwf.framework" + sep + "dwf",
    ]
    dwf = None
    for _candidate in _candidates:
        if os.path.exists(_candidate):
            dwf = ctypes.cdll.LoadLibrary(_candidate)
            break
    if dwf is None:
        print("Could not find dwf.framework. Checked:", _candidates)
        sys.exit(1)
    constants_path = sep + "Applications" + sep + "WaveForms.app" + sep + "Contents" + sep + "Resources" + sep + "SDK" + sep + "samples" + sep + "py"
elif platform.startswith("win"):
    dwf = ctypes.cdll.dwf
    constants_path = "C:" + sep + "Program Files (x86)" + sep + "Digilent" + sep + "WaveFormsSDK" + sep + "samples" + sep + "py"
else:
    dwf = ctypes.cdll.LoadLibrary("libdwf.so")
    constants_path = sep + "usr" + sep + "share" + sep + "digilent" + sep + "waveforms" + sep + "samples" + sep + "py"

path.append(constants_path)
import dwfconstants as const  # noqa: E402 -- must come after sys.path append


def check_error():
    msg = ctypes.create_string_buffer(512)
    dwf.FDwfGetLastErrorMsg(msg)
    msg = msg.value.decode("ascii")
    if msg:
        raise RuntimeError("dwf error: " + msg)


def ad2_open():
    hdwf = ctypes.c_int()
    dwf.FDwfDeviceOpen(ctypes.c_int(-1), ctypes.byref(hdwf))
    if hdwf.value == 0:
        check_error()
        raise RuntimeError("no AD2 found")
    return hdwf


def ad2_set_channel_freq(hdwf, channel, frequency_hz, wait_seconds):
    """Configure one DIO channel as a 50% duty square wave at the given
    frequency, starting after `wait_seconds` -- that wait is how the two
    channels get their relative phase offset, since this API has no
    direct phase-set call at the DigitalOut level."""
    internal_freq = ctypes.c_double()
    if dwf.FDwfDigitalOutInternalClockInfo(hdwf, ctypes.byref(internal_freq)) == 0:
        check_error()

    counter_limit = ctypes.c_uint()
    if dwf.FDwfDigitalOutCounterInfo(hdwf, ctypes.c_int(channel), ctypes.c_int(0), ctypes.byref(counter_limit)) == 0:
        check_error()

    # ceiling division -- same formula Digilent's own wrapper uses for pulse type
    divider = int(-(-(internal_freq.value / frequency_hz) // counter_limit.value))

    if dwf.FDwfDigitalOutEnableSet(hdwf, ctypes.c_int(channel), ctypes.c_int(1)) == 0:
        check_error()
    if dwf.FDwfDigitalOutTypeSet(hdwf, ctypes.c_int(channel), const.DwfDigitalOutTypePulse) == 0:
        check_error()
    if dwf.FDwfDigitalOutDividerSet(hdwf, ctypes.c_int(channel), ctypes.c_int(divider)) == 0:
        check_error()

    steps = int(round(internal_freq.value / frequency_hz / divider))
    high_steps = int(steps * 50 / 100)  # 50% duty square wave for the quadrature channel itself
    low_steps = int(steps - high_steps)
    if dwf.FDwfDigitalOutCounterSet(hdwf, ctypes.c_int(channel), ctypes.c_int(low_steps), ctypes.c_int(high_steps)) == 0:
        check_error()

    if dwf.FDwfDigitalOutWaitSet(hdwf, ctypes.c_double(wait_seconds)) == 0:
        check_error()
    if dwf.FDwfDigitalOutRepeatSet(hdwf, ctypes.c_int(0)) == 0:  # 0 = infinite repeat
        check_error()
    if dwf.FDwfDigitalOutRunSet(hdwf, ctypes.c_double(0)) == 0:  # 0 = infinite run
        check_error()


def ad2_apply(hdwf):
    """Push whatever's been Set above out to the device. Calling this
    again on an already-running output is how you update live -- this is
    what actually changes the simulated speed in real time."""
    if dwf.FDwfDigitalOutConfigure(hdwf, ctypes.c_int(True)) == 0:
        check_error()


def update_ad2_quadrature(hdwf, freq_hz):
    freq_hz = max(freq_hz, MIN_FREQ_HZ)
    quarter_period = (1.0 / freq_hz) / 4.0  # 90 degrees of phase, as a time delay
    ad2_set_channel_freq(hdwf, DIO_A, freq_hz, wait_seconds=0.0)
    ad2_set_channel_freq(hdwf, DIO_B, freq_hz, wait_seconds=quarter_period)
    ad2_apply(hdwf)


# ----------------------------------------------------------------------
# Main loop
# ----------------------------------------------------------------------


def main():
    print(f"Opening serial port {SERIAL_PORT} @ {SERIAL_BAUD}...")
    ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1.0)

    print("Opening AD2...")
    hdwf = ad2_open()

    sim_speed_rpm = 0.0
    last_time = time.monotonic()

    print("Running. Ctrl-C to stop.\n")
    try:
        while True:
            line = ser.readline().decode(errors="replace").strip()
            if not line:
                continue

            match = DUTY_RE.search(line)
            if match is None:
                # not every line necessarily has duty= yet (e.g. boot
                # message) -- skip rather than crash
                continue

            duty_percent = float(match.group(1))

            now = time.monotonic()
            dt = now - last_time
            last_time = now

            target_rpm = (duty_percent / 100.0) * MAX_RPM
            sim_speed_rpm += (target_rpm - sim_speed_rpm) * (dt / TAU_S)
            sim_speed_rpm = max(sim_speed_rpm, 0.0)  # unidirectional drive, no negative speed

            cycles_per_sec = sim_speed_rpm / 60.0  # CPR=4 convention: 1 cycle = 1 "revolution"
            freq_hz = cycles_per_sec

            update_ad2_quadrature(hdwf, freq_hz)

            print(
                f"duty={duty_percent:6.1f}%  target={target_rpm:7.1f} rpm  "
                f"sim_speed={sim_speed_rpm:7.1f} rpm  ad2_freq={freq_hz:6.2f} Hz"
            )

    except KeyboardInterrupt:
        print("\nStopping.")
    finally:
        dwf.FDwfDigitalOutReset(hdwf)
        dwf.FDwfDeviceClose(hdwf)
        ser.close()


if __name__ == "__main__":
    main()
