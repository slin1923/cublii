# Overview

Cubli is a controls project trying to balance a cube on its corner via 3 reaction wheels.  This project was inspired by [ETH Zurich](https://idsc.ethz.ch/research-dandrea/research-projects/archive/cubli.html) and [remRC](https://github.com/remrc/Self-Balancing-Cube).  This project is a WIP and is a personal project for practicing dynamic modeling, controls, and state estimation. There is a LOT of under-the-hood theory that goes unappreciated only looking at this repository.  For documentation on theory and hardware, please visit my project post on my portfolio:

https://slin1923-portfolio.github.io/projects/cubli.html

All code in this repository is flashed OTA to the ESP32 WROOM 32 onboard Cublii.  **This repository is useless without having built your own Cublii hardware**.  

<!-- <figure align="center">
  <img src="/images/assembled_cubli_controller_side.jpg" width="600">
  <figcaption>Cublii in the flesh.  Go build it it's completely open source!</figcaption>
</figure> -->

<div align="center">
  <img src="/images/assembled_cubli_controller_side.jpg" width="600">
  <p><em>Cublii in the flesh.  Go build it it's completely open source!</em></p>
</div>

# Quick Facts

- Cublii is connected to wifi
- I flash new firmware to Cublii cable-free using Platform.io OTA protocol
- A huge range of telemetry is transmitted from Cublii back to my laptop via MQTT protocol at 50 Hz. The most important telemetry fields include
  - ```[ax, ay, az, gx, gy, gz]```: Raw MPU6050 IMU data
  - ```[rpm1, rpm2, rpm3]```: NIDEC 24H BLDC motor RPMs as read off by the 2-Channel 100 PPR built-in encoders
  - ```t```: ESP32 timestamp per data point (importantly distinct from laptop receive time which is heavily influenced by network latency)
  - ```batt_mv```: Battery level sensor, scaled down by roughly 4.4 from true battery voltage. 
- I read live telemetry out using [PlotJuggler4](https://plotjuggler.io/) - a fast powerful live data plotter. 
- Network maintenance and telemetry run on a different core than the control loop
- The central control loop is modular and runs at 100 Hz.  I plan to develop as many controller/observer combos as I know, and they will be easily interchangeable.
- In addition to the MQTT node onboard publishing telemetry topics, it is able to subscribe to topics defined by user commands.
  - ```en```: sending 0 disables all motors, sending 1 enables all motors - effectively a killswitch topic
  - ```yaw_cmd```: the user sends a particular yaw angle that cublii should track. 
- Cublii communicates informative status messages via MQTT and essential messages via an onboard beeper.

# Usage

During Run
- Upon start, 1 Beep indicates that **Calibration routine will start in 5 seconds**
- You have 5 seconds to hold the cubli approximately near its unstable equilibrium point and keep it there
- 2 Beeps indicate calibration has started **HOLD CUBLII AS STILL AS POSSIBLE DURING THESE 5 SECONDS**
- 5 seconds later, another 2 Beeps indicate calibration has ended
- Enable the motors
- Watch Cublii balance and read live telemetry
- Optional: send yaw commands and see cublii follow them. 

Telemetry
- After cubli is running its control loop, you can 

Status messages
- When beeper begins to loop in a 3-beep cadence - battery is low, change it

### Update 10/7/2026

- Architecture complete
- All hardware implemented