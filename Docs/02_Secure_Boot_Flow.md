# Secure Boot Flow

## 1. Overview

The Secure Boot process ensures that the application firmware is verified before it is allowed to execute.

The Bootloader is the first software component executed after MCU reset.

## 2. Boot Flow

```text
                 MCU RESET
                     |
                     v
              +--------------+
              |  Bootloader  |
              +--------------+
                     |
                     v
          Is Application Present?
                /          \
              NO            YES
              |              |
              v              v
       Recovery/Error    Read Firmware
                           Image
                             |
                             v
                    Calculate/Verify
                    Firmware Integrity
                             |
                             v
                    Verify Authenticity
                             |
                       +-----+-----+
                       |           |
                    SUCCESS      FAILURE
                       |           |
                       v           v
                Validate Image   Recovery /
                       |          Error Handling
                       v
                Set Application
                Vector Table
                       |
                       v
                Jump to Application
                       |
                       v
                 APPLICATION
```
## 3. Detailed Steps
### Step 1 – MCU Reset

After reset, the MCU starts execution from the configured reset address.

The Bootloader is located at the beginning of the boot memory region and executes first.

### Step 2 – Bootloader Initialization

The Bootloader performs the minimum required initialization to determine whether a valid application is available.

### Step 3 – Application Detection

The Bootloader checks whether an application firmware image is present at the expected application address.

### Step 4 – Firmware Verification

The Bootloader validates the firmware image.

The verification process will include:

Firmware integrity verification
Firmware authenticity verification
Image metadata validation
Application memory validation
### Step 5 – Verification Decision

If verification succeeds, the Bootloader continues with application execution.

If verification fails, the Bootloader must not execute the application and enters the defined failure/recovery path.

### Step 6 – Application Preparation

Before transferring control, the Bootloader prepares the MCU for application execution.

This includes configuring the application vector table and required processor state.

### Step 7 – Application Handover

The Bootloader loads the application's initial stack pointer and reset handler address and transfers execution to the application.

### Step 8 – Application Execution

The trusted application firmware starts execution.

## 4. Failure Handling

If the firmware verification fails:
```text
Firmware Verification
        |
        v
     FAILED
        |
        v
Do NOT execute application
        |
        v
Recovery / Error Handling
```
The exact recovery mechanism will be defined and implemented as the project progresses.

## 5. Security Principle

The fundamental security rule of this project is:

Unverified firmware must never be executed.

The Bootloader acts as the initial trust boundary between the MCU reset process and application firmware.

## 6. Future Extensions

The boot flow will be extended with:

- Cryptographic hash calculation
- Digital signature verification
- Firmware version checking
- Anti-rollback protection
- Secure firmware update
- Key management considerations
- Security-related testing
- Safety requirements and verification activities
