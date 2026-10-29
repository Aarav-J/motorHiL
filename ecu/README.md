# HiL Testing system for a real-time motor control ECU

- Project will be a baremetal ECU running on a esp32
  - The motor reads an encoder, calculates the error between an actual speed and target speed(PID controller), and then adjusts the PWM output
- We will have an esp32-s3 system running zephyr that will calculate the PWM captured, as well as the motor physics model(to fake data), encoder emulation(faking the communication protocols and process), and fault injection(seeing how the ECU handles errors)
- There will then be a python script handling the testing

- [ ] Want to get the baremetal version of this working
  - [ ] Linker scripts etc
  - [ ] PWM and encoder mode
  - [ ] Closed loop ECU control
- [ ] Then the zephyr part of it
  - TBD
