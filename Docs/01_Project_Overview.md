# Secure Boot Project – Project Overview

## 1. Introduction

This project focuses on designing and implementing a Secure Boot and dual-image firmware update architecture for an STM32F407-based embedded system.

The current implementation focuses on firmware integrity verification, application image validation, secure application handover, and candidate/active firmware management.

Digital-signature-based firmware authenticity verification is planned as a future security extension.

---

## 2. Problem Statement

In a conventional boot flow, the microcontroller may directly execute application firmware after reset.

If the firmware is corrupted or modified, the system may execute unintended code.

A Secure Bootloader introduces a verification step before application execution to ensure that the firmware image satisfies the defined validation and integrity requirements.

For firmware update scenarios, the system also needs to evaluate a new candidate image without unnecessarily overwriting the currently active firmware.

---

## 3. Project Objective

The project objectives are to:

* Start bootloader execution after MCU reset.
* Identify and validate the installed firmware image.
* Validate firmware metadata and application memory boundaries.
* Verify firmware integrity using SHA-256.
* Manage separate active and candidate firmware images.
* Independently verify candidate firmware before execution.
* Execute an application only after successful validation.
* Reject invalid or corrupted firmware safely.
* Provide an active-image fallback when candidate validation fails.
* Provide a foundation for future authenticated firmware updates.

---

## 4. High-Level Architecture

The system consists of three logical areas:

### Bootloader

The Bootloader executes first after MCU reset.

Responsibilities include:

* Boot decision
* Firmware metadata handling
* Application image boundary validation
* Firmware version validation
* SHA-256 integrity verification
* Candidate image validation
* Active-image fallback
* Failure handling
* Application handover

### Candidate Firmware

The Candidate Firmware represents a new firmware image being evaluated by the bootloader.

The candidate image is stored separately from the active firmware so that the existing active image remains available while the candidate is being validated.

The bootloader independently calculates the can
