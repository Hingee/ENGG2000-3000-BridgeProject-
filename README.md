# Automated Scale Model Bridge Control System

## Project Overview
This repository contains the design, firmware, and systems engineering documentation for an automated scale model bridge. This system coordinates road, pedestrian, and marine traffic through a synchronized network of sensors and actuators controlled by an ESP32.

The project emphasizes a **Systems Engineering** approach to ensure safety, reliability, and real-time responsiveness in an infrastructure model.

## Key Features
* **ESP32 Control Logic:** Centralized microcontroller management for all peripheral components.
* **Automated Traffic Sequencing:** Coordination of LED traffic signals, audio warnings (buzzers), and physical barriers (servos) based on real-time detection.
* **Dynamic Sensing:**
    * **Boat Detection:** Ultrasonic sensors (HC-SR04) to trigger bridge opening sequences.
    * **Pedestrian Safety:** PIR motion sensors to detect activity on the span.
* **Remote User Interface (RUI):** A Wi-Fi-enabled web dashboard for live monitoring and manual override capabilities.
* **Efficiency:** Designed for a bridge deployment time (opening/closing) of ≤ 10 seconds.

## Technical Specifications
* **Microcontroller:** ESP32 (utilizing Wi-Fi for RUI communication).
* **Actuators:** 12V 251RPM DC Geared Motors, Continuous Rotation Servos.
* **Power Management:** 12V 5A battery source with step-up/down converters for 5V/3.3V logic.
* **Safety Systems:** Fail-safe default states and synchronized audio-visual warnings.
