# Candidate Update State Machine

```mermaid
stateDiagram-v2

    [*] --> EMPTY

    EMPTY --> PENDING_VALIDATION : New candidate installed

    PENDING_VALIDATION --> VALIDATED : SHA-256 validation successful

    VALIDATED --> BOOT_PENDING : Update state
    BOOT_PENDING --> CANDIDATE_IMAGE : Execute candidate image

    BOOT_PENDING --> ROLLBACK : Candidate not confirmed after reset
    ROLLBACK --> ACTIVE_IMAGE : Roll back to active image

    CONFIRMED --> CANDIDATE_IMAGE : Execute confirmed candidate

    INVALID --> ACTIVE_IMAGE : Invalid / unknown state

```


| State                | Meaning                                                 |
| -------------------- | ------------------------------------------------------- |
| `EMPTY`              | No candidate update is pending                          |
| `PENDING_VALIDATION` | Candidate image is installed and waiting for validation |
| `VALIDATED`          | Candidate image passed integrity validation             |
| `BOOT_PENDING`       | Candidate is approved for boot testing                  |
| `CONFIRMED`          | Candidate successfully confirmed by the application     |
| `ROLLBACK`           | Candidate boot was unsuccessful or not confirmed        |
| `INVALID`            | Invalid or unknown update state                         |
| `CANDIDATE_IMAGE`    | Candidate firmware is executed                          |
| `ACTIVE_IMAGE`       | Current known-good firmware is executed                 |


## Main Flow
```text
EMPTY
  ↓
PENDING_VALIDATION
  ↓
VALIDATED
  ↓
BOOT_PENDING
  ↓
CANDIDATE_IMAGE
```
## Candidate Boot Failure
```text
BOOT_PENDING
  ↓
ROLLBACK
  ↓
ACTIVE_IMAGE
```
## Candidate Confirmation
```text
CONFIRMED
  ↓
CANDIDATE_IMAGE

Invalid State
INVALID
  ↓
ACTIVE_IMAGE
```
