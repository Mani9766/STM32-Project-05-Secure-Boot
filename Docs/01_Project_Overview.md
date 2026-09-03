# Secure Boot Project – Project Overview



## 1. Introduction



This project focuses on designing and implementing a Secure Boot mechanism for an STM32-based embedded system.



The primary objective is to ensure that the microcontroller executes only a valid and trusted application firmware.



## 2. Problem Statement



In a conventional boot flow, the microcontroller may directly execute application firmware after reset.



If the firmware is corrupted, modified, or replaced with an unauthorized image, the system may execute unintended code.



Secure Boot introduces a verification step before application execution.



## 3. Project Objective



The Secure Bootloader will:



- Start execution after MCU reset.

- Identify the installed application firmware.

- Validate the application firmware.

- Verify firmware integrity.

- Verify firmware authenticity.

- Execute the application only when verification succeeds.

- Handle invalid or corrupted firmware safely.



## 4. High-Level Architecture



The system consists of two major software components:



### Bootloader



The Bootloader executes first after MCU reset.



Responsibilities include:



- Boot decision

- Application image validation

- Firmware verification

- Failure handling

- Application handover



### Application



The Application contains the actual product functionality.



The Application is executed only after the Bootloader successfully validates the firmware.



## 5. Development Approach



The project will be developed incrementally:



1\. Basic Bootloader

2\. Application memory separation

3\. Bootloader-to-Application jump

4\. Firmware image identification

5\. Firmware integrity verification

6\. Firmware authenticity verification

7\. Invalid image handling

8\. Security hardening

9\. Testing and verification

10\. Extension toward automotive software safety practices



## 6. Target Platform



- MCU: STM32

- CPU: ARM Cortex-M

- Language: Embedded C

- Debug Interface: SWD

- Development Environment: STM32CubeIDE



## 7. Project Status



\*\*Status: Work in Progress\*\*



Features will be documented as they are implemented and verified.

