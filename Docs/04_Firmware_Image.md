# Firmware Image

## 1. Overview

The Bootloader needs a well-defined firmware image format so that it can identify, validate, and authenticate the Application before execution.

The firmware image consists of the Application binary together with metadata required for verification.

## 2. Firmware Image Structure

The initial conceptual structure is:

```text
+-----------------------------+
|       Firmware Header       |
+-----------------------------+
|                             |
|      Application Image      |
|                             |
+-----------------------------+
|       Hash / Digest         |
+-----------------------------+
|       Digital Signature     |
+-----------------------------+
```

The exact format will be finalized during implementation.

## 3. Firmware Header

The header may contain information such as:

- Magic number
- Firmware version
- Firmware size
- Target/device identification
- Image type
- Header version
- Additional metadata required for validation

The magic number can be used to determine whether a valid firmware image is present.

## 4. Application Image

The Application Image contains the actual executable firmware.

The Bootloader must know:

- Application start address
- Application size
- Valid memory range

The Application must remain within its allocated Flash region.

## 5. Firmware Integrity

A cryptographic hash will be calculated over the defined firmware data.

Conceptually:
```text
Application Image
       |
       v
 Hash Function
       |
       v
Calculated Hash
       |
       v
Compare with expected value
```
A mismatch indicates that the firmware image has been modified or corrupted.

## 6. Firmware Authenticity

Integrity alone does not prove that the firmware came from a trusted source.

Digital signature verification will be used to establish firmware authenticity.

Conceptually:
```text
Firmware Image
      |
      v
Calculate Hash
      |
      v
Verify Digital Signature
      |
      v
Trusted Firmware?
   /          \
 YES           NO
  |             |
  v             v
Execute       Reject
```
## 7. Firmware Version

The firmware version will be included in the image metadata.

Version information can later be used to implement anti-rollback protection and prevent installation of unauthorized older firmware.

## 8. Image Validation

Before jumping to the Application, the Bootloader will validate:

- Image presence
- Header/magic number
- Firmware size
- Memory boundaries
- Firmware integrity
- Firmware authenticity
- Firmware version, when applicable

Only a successfully validated image will be allowed to execute.

## 9. Design Considerations

The following items will be finalized during implementation:

Exact header structure
- Hash algorithm
- Signature algorithm
- Signature size
- Key storage
- Metadata location
- Image generation process
- Image verification process
## 10. Implementation Status

Status: Design Phase

The firmware image format will be finalized before implementing the complete cryptographic verification mechanism.
