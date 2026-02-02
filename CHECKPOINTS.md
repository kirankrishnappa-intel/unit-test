# PCIe Test Driver Framework - Checkpoints Guide

**Version:** 1.0  
**Date:** February 2, 2026

## Overview

The PCIe Test Driver Framework now includes comprehensive checkpoint tracking to monitor progress, validate operations, and provide detailed diagnostic information about each step of the testing process.

## What Are Checkpoints?

Checkpoints are progress markers that track the success or failure of specific operations within the framework:

- **Passed** (✓) - Operation completed successfully
- **Failed** (✗) - Operation encountered an error
- **Skipped** (⊘) - Operation was not applicable or already completed

## Checkpoint Coverage

The framework tracks checkpoints across all major operations:

### Build Checkpoints
```
✓ clean_build            - Clean previous build artifacts
✓ compile_module         - Compile kernel module
✓ compile_utility        - Compile user-space utility
✓ executable_permissions - Set executable permissions
```

### Load Module Checkpoints
```
✓ module_exists          - Kernel module file exists
✓ insmod_success         - Module loaded with insmod
✓ module_loaded          - Module confirmed in lsmod
```

### Unload Module Checkpoints
```
✓ rmmod_success          - Module removed with rmmod
✓ module_unloaded        - Module confirmed removed from lsmod
```

### Reload Module Checkpoints
```
Combines load and unload checkpoints
```

### Basic Test Checkpoints
```
✓ test_utility_exists    - Test utility executable found
✓ module_running         - Module is loaded and running
✓ test_list_devices      - Device enumeration works
✓ device_available       - PCIe test device detected
✓ test_device_info       - Device information retrieval works
✓ test_statistics        - Statistics collection works
```

### Status Check Checkpoints
```
✓ status_check_module    - Module loaded status verified
✓ status_check_devices   - Device availability verified
✓ status_check_logs      - Kernel logs present
```

### Cleanup Checkpoints
```
✓ cleanup_complete       - Build artifacts removed
```

## Using Checkpoints

### View Checkpoints During Operation

Checkpoints are displayed automatically during execution:

```bash
# Build with checkpoint tracking
./test_driver.sh build
# Output shows: ✓ CHECKPOINT [clean_build] = passed
#               ✓ CHECKPOINT [compile_module] = passed
#               etc.

# Load module with checkpoints
./test_driver.sh load
# Output shows: ✓ CHECKPOINT [module_exists] = passed
#               ✓ CHECKPOINT [insmod_success] = passed
#               ✓ CHECKPOINT [module_loaded] = passed

# Run tests with checkpoints
./test_driver.sh test-basic
# Output shows: ✓ CHECKPOINT [test_utility_exists] = passed
#               ✓ CHECKPOINT [module_running] = passed
#               etc.
```

### View Checkpoint Summary

After each action, a summary is displayed:

```
Checkpoint Summary:
====================
✓ clean_build
✓ compile_module
✓ compile_utility
✓ executable_permissions
```

### Generate Checkpoint Report

View all recorded checkpoints from test runs:

```bash
./test_driver.sh checkpoints
```

**Output example:**
```
========================================
   PCIe Test Driver - Checkpoint Report
========================================

Recent Checkpoints:
===================
[2026-02-02 12:49:15] [INFO] CHECKPOINT: clean_build = passed
[2026-02-02 12:49:16] [INFO] CHECKPOINT: compile_module = passed
[2026-02-02 12:49:18] [INFO] CHECKPOINT: compile_utility = passed
[2026-02-02 12:49:19] [INFO] CHECKPOINT: executable_permissions = passed
[2026-02-02 12:49:25] [INFO] CHECKPOINT: module_exists = passed
[2026-02-02 12:49:26] [INFO] CHECKPOINT: insmod_success = passed
[2026-02-02 12:49:27] [INFO] CHECKPOINT: module_loaded = passed
...

Checkpoint Statistics:
======================
Total:   35
Passed:  33
Failed:  0
Skipped: 2

Success Rate: 94% (33/35)

Log File:
/home/kirank1/code/git/linux_iosf_test/unit-test/test_results.log
```

## Checkpoint Log File

All checkpoints are logged to `test_results.log`:

```bash
# View checkpoint log
cat test_results.log

# Search for specific checkpoint
grep "CHECKPOINT:" test_results.log

# Count checkpoint types
grep "passed" test_results.log | wc -l
grep "failed" test_results.log | wc -l
grep "skipped" test_results.log | wc -l

# View checkpoints from last run
tail -50 test_results.log | grep "CHECKPOINT:"
```

## Checkpoint Examples

### Successful Build Sequence
```
[2026-02-02 12:49:15] [INFO] CHECKPOINT: clean_build = passed
[2026-02-02 12:49:16] [INFO] CHECKPOINT: compile_module = passed
[2026-02-02 12:49:18] [INFO] CHECKPOINT: compile_utility = passed
[2026-02-02 12:49:19] [INFO] CHECKPOINT: executable_permissions = passed

✓ Checkpoint Summary:
✓ clean_build
✓ compile_module
✓ compile_utility
✓ executable_permissions
```

### Successful Module Load
```
[2026-02-02 12:49:25] [INFO] CHECKPOINT: module_exists = passed
[2026-02-02 12:49:26] [INFO] CHECKPOINT: insmod_success = passed
[2026-02-02 12:49:27] [INFO] CHECKPOINT: module_loaded = passed

✓ Checkpoint Summary:
✓ module_exists
✓ insmod_success
✓ module_loaded
```

### Failed Build (Example)
```
[2026-02-02 12:49:15] [INFO] CHECKPOINT: clean_build = passed
[2026-02-02 12:49:16] [INFO] CHECKPOINT: compile_module = failed
[ERROR] Build failed

✓ Checkpoint Summary:
✓ clean_build
✗ compile_module
```

### Test With Device Unavailable
```
[2026-02-02 12:49:30] [INFO] CHECKPOINT: test_utility_exists = passed
[2026-02-02 12:49:31] [INFO] CHECKPOINT: module_running = passed
[2026-02-02 12:49:32] [INFO] CHECKPOINT: test_list_devices = passed
[2026-02-02 12:49:33] [INFO] CHECKPOINT: device_available = skipped

✓ Checkpoint Summary:
✓ test_utility_exists
✓ module_running
✓ test_list_devices
⊘ device_available  (no PCIe devices available)
```

## Checkpoint Workflow

### Complete Setup Workflow with Checkpoints
```bash
# 1. Build (4 checkpoints)
./test_driver.sh build

# 2. Load Module (3 checkpoints)
sudo ./test_driver.sh load

# 3. Check Status (3 checkpoints)
./test_driver.sh check

# 4. Run Tests (6 checkpoints)
./test_driver.sh test-basic

# 5. View All Checkpoints (summary)
./test_driver.sh checkpoints

# 6. Unload (2 checkpoints)
sudo ./test_driver.sh unload

# 7. Cleanup (1 checkpoint)
./test_driver.sh cleanup
```

**Total Checkpoints: ~22 across all operations**

## Troubleshooting With Checkpoints

### Issue: Module fails to load

Check checkpoints:
```bash
./test_driver.sh checkpoints | grep "module"
```

If `module_exists` failed:
```bash
./test_driver.sh build
```

If `insmod_success` failed:
```bash
# Try manual load
sudo insmod pcie_test_driver.ko debug_level=2
dmesg | tail -20
```

### Issue: Tests don't run

Check checkpoints:
```bash
./test_driver.sh checkpoints | grep "test"
```

If `module_running` failed:
```bash
sudo ./test_driver.sh load
```

If `device_available` skipped:
```bash
lspci  # Check for PCIe devices
```

### Issue: Build fails

Check checkpoints:
```bash
./test_driver.sh checkpoints | grep "compile"
```

View build logs:
```bash
./test_driver.sh build 2>&1 | head -50
```

Clean and rebuild:
```bash
./test_driver.sh cleanup
./test_driver.sh build
```

## Checkpoint Statistics

### After Successful Setup
- **Expected Passed:** ~15-20 (depends on available devices)
- **Expected Failed:** 0
- **Expected Skipped:** 2-5 (depending on hardware)
- **Success Rate:** >95%

### After Failed Checkpoint
- Immediate indication of what failed
- Suggested remediation in output
- Detailed logging for debugging

## Understanding Checkpoint Output

### Color Coding
```
✓ GREEN = Passed (success)
✗ RED   = Failed (error)
⊘ YELLOW = Skipped (not applicable)
```

### Log Format
```
[2026-02-02 12:49:15] [INFO] CHECKPOINT: operation_name = status
 └─ Timestamp         └─ Level  └─ Type      └─ Operation  └─ Status
```

### Summary Format
```
Checkpoint Summary:
====================
✓ checkpoint_name       (passed)
✗ checkpoint_name       (failed)
⊘ checkpoint_name       (skipped)
```

## Automated Checkpoint Tracking

The framework automatically tracks checkpoints for:

1. **Build Phase**
   - Cleaning previous artifacts
   - Compiling module
   - Compiling utility
   - Setting permissions

2. **Load Phase**
   - Module file existence
   - Module loading
   - Module verification

3. **Test Phase**
   - Utility availability
   - Module status
   - Device enumeration
   - Device availability
   - Device info retrieval
   - Statistics collection

4. **Status Phase**
   - Module status
   - Device status
   - Log file status

5. **Cleanup Phase**
   - Artifact removal

## Integration with CI/CD

Checkpoints are ideal for continuous integration:

```bash
#!/bin/bash
# ci-test.sh

./test_driver.sh build
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

sudo ./test_driver.sh load
if [ $? -ne 0 ]; then
    echo "Load failed"
    exit 1
fi

sudo ./test_driver.sh test-basic
if [ $? -ne 0 ]; then
    echo "Tests failed"
    exit 1
fi

# Generate report
./test_driver.sh checkpoints > ci-report.txt
```

## Performance Impact

- Checkpoint tracking adds minimal overhead
- Logging is asynchronous
- No performance impact on actual driver operation
- Log file grows ~50 bytes per checkpoint

## Log File Management

### View Log File
```bash
cat test_results.log
```

### Clear Old Logs
```bash
rm test_results.log
# New log starts on next run
```

### Archive Logs
```bash
cp test_results.log test_results_$(date +%Y%m%d_%H%M%S).log
```

### Analyze Logs
```bash
# Count total checkpoints
grep "CHECKPOINT:" test_results.log | wc -l

# Count by status
echo "Passed: $(grep 'CHECKPOINT:.*passed' test_results.log | wc -l)"
echo "Failed: $(grep 'CHECKPOINT:.*failed' test_results.log | wc -l)"
echo "Skipped: $(grep 'CHECKPOINT:.*skipped' test_results.log | wc -l)"
```

## Best Practices

1. **Always check checkpoints after operations**
   ```bash
   ./test_driver.sh checkpoints
   ```

2. **Save reports before critical changes**
   ```bash
   cp test_results.log test_results_backup.log
   ```

3. **Use checkpoints for troubleshooting**
   - Check which checkpoint failed
   - Refer to suggested remediation
   - Review kernel logs if needed

4. **Review after major changes**
   - Rebuild framework
   - Run full test suite
   - Generate checkpoint report
   - Verify all critical checkpoints

5. **Monitor for patterns**
   - Track repeated failures
   - Identify performance trends
   - Catch early issues

## Summary

The checkpoint system provides:

- ✓ Real-time operation status
- ✓ Detailed progress tracking
- ✓ Comprehensive logging
- ✓ Easy troubleshooting
- ✓ CI/CD integration support
- ✓ Performance analysis
- ✓ Audit trail

For more information, see:
- [README.md](README.md) - Complete reference
- [test_driver.sh](test_driver.sh) - Implementation details
- `test_results.log` - Your checkpoint history

---

**Last Updated:** February 2, 2026  
**Status:** ✅ Checkpoints Fully Implemented
