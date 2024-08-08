# Axivion Tool Qualification Kit

This directory and its subdirectories contain the Axivion Tool Qualification
Kit (TQK).

> **NOTE:** `perform_tests.exe` must be on PATH in the correct version.

- This test can be run using the test script:
  `<repository-root>/tests/axivion/axivion_self_tests.py`
- The test files of the TQK are in the directory
  `<repository-root>/tests/axivion/qualification-test/qualification-kit/`.
  **These test files are not part of the public foxBMS 2 release.**
- To ensure that the test suite is not changed inadvertently, the SHA-256 of
  the directory is checked in CI:

  ```pwsh
  PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 run-script tools\utils\verify_checksums.py tests\axivion\qualification-test\qualification-kit <known-hash> -vv
  ```

- If the TQK is changed/updated the following needs to be done:
  - The changes need to be applied in
    `<repository-root>/tests/axivion/qualification-test/qualification-kit/`.
  - The known SHA-256 of the test files needs to be updated in the CI step.
  - The commit message **MUST** contain ``Update Axivion``.
    This ensures that the TQK tests are run.
- When the Axivion Bauhaus Suite is updated, the TQK also needs to be updated.
  The pinned Bauhaus version is found in
  ``<repository-root>/conf/env/paths_win32.txt``.
