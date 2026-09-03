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
