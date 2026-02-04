# STM32 Gesture Authentication System

An embedded authentication system built on an STM32 microcontroller that unlocks access using user-defined motion gestures captured from a gyroscope. The system performs real-time sensor data acquisition, feature extraction, and template matching to authenticate users based on their unique gesture patterns.

This project focuses on embedded systems design, signal processing, and real-time control on resource-constrained hardware.

---

## Overview

The system allows a user to enroll a custom gesture by performing it multiple times. During authentication, the user repeats the gesture, and the system compares the input against stored templates to determine whether access should be granted.

Gesture data is collected from an onboard gyroscope and processed directly on the microcontroller. Feedback and system status are displayed in real time using an LCD interface.

---

## Features

- Gesture-based authentication using motion input
- Real-time gyroscope data capture (gx, gy, gz)
- User enrollment with multiple gesture samples
- Correlation-based feature extraction and normalization
- Threshold-based template matching for authentication
- Finite-state control flow for system logic
- LCD display for live feedback and status messages
- Fully embedded implementation on STM32 hardware

---

## Tech Stack

- STM32 microcontroller
- C / C++
- Embedded Linux / bare-metal firmware
- Gyroscope (IMU sensor)
- LCD display module

---

## System Architecture

- **Sensor Layer**
  - Collects real-time gyroscope data during gesture input
- **Processing Layer**
  - Extracts normalized correlation-based features from time-series data
- **Authentication Layer**
  - Compares incoming gestures against stored templates using threshold matching
- **Control Layer**
  - Finite-state machine managing enrollment, authentication, success, and failure states
- **UI Layer**
  - LCD display providing system feedback and authentication results

---

## How It Works

1. **Enrollment**
   - User performs a gesture multiple times
   - System records gyroscope time-series data
   - Features are extracted and stored as gesture templates

2. **Authentication**
   - User repeats the gesture
   - Incoming data is processed and compared against stored templates
   - Access is granted or denied based on similarity threshold

3. **Feedback**
   - System state and authentication result are displayed on the LCD in real time

---

## Project Structure

