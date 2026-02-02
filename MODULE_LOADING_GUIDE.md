# Module Loading Guide

## Status

Both kernel modules have been **successfully compiled and signed**:

- ✅ **pcie_test_driver.ko** (378KB) - PCIe device test framework
- ✅ **btintel_test_generic_driver.ko** (359KB) - Intel Bluetooth test driver

Both modules are signed with a self-generated MOK (Machine Owner Key) certificate to satisfy kernel Secure Boot requirements.

## Current System Status

This system has **Secure Boot enabled** with module signature verification enforced:

```
CONFIG_MODULE_SIG=y
CONFIG_MODULE_SIG_ALL=y
CONFIG_MODULE_SIG_SHA512=y
```

Kernel prevents loading of unsigned or improperly-signed modules:
```
[273761.869745] Loading of module with unavailable key is rejected
```

## Next Steps to Load Modules

### Option 1: Reboot to Enroll MOK (Recommended)

1. **Reboot the system:**
   ```bash
   sudo reboot
   ```

2. **During boot, you will see the MOK Manager (UEFI interface):**
   - Select "Enroll MOK"
   - Confirm the unit-test-signing certificate
   - Enter the MOK password (if prompted during import, use empty password)
   - Reboot again

3. **After reboot, load the modules:**
   ```bash
   cd /home/kirank1/code/git/linux_iosf_test/unit-test
   sudo insmod pcie_test_driver.ko
   sudo insmod btintel_test_generic_driver.ko
   ```

4. **Verify modules are loaded:**
   ```bash
   lsmod | grep -E "pcie_test|btintel_test"
   dmesg | tail -20
   ```

### Option 2: Disable Secure Boot (If Available)

If you have access to system BIOS/UEFI settings:

1. Reboot into BIOS/UEFI settings
2. Disable "Secure Boot"
3. Save and reboot
4. Load modules normally:
   ```bash
   cd /home/kirank1/code/git/linux_iosf_test/unit-test
   sudo insmod pcie_test_driver.ko
   sudo insmod btintel_test_generic_driver.ko
   ```

## File Locations

- **Modules:** `/home/kirank1/code/git/linux_iosf_test/unit-test/*.ko`
- **MOK Certificate (public):** `mok.der`
- **MOK Key (private):** `mok.key` (keep secure)

## Module Information

### pcie_test_driver.ko

**Purpose:** Generic PCIe device test framework

**Features:**
- PCI device enumeration and BAR memory access
- Interrupt handling (MSI-X ready)
- Device statistics (interrupt count, error count, operation count)
- Sysfs attributes for device information
- 14 IOCTL commands for BAR read/write operations

**Usage:**
```bash
sudo insmod pcie_test_driver.ko
ls /dev/pcie_test_driver*
cat /sys/module/pcie_test_driver/parameters/*
```

### btintel_test_generic_driver.ko

**Purpose:** Intel Bluetooth-specific test driver

**Features:**
- Character device with internal buffer management
- 8 IOCTL commands (GET_INFO, GET_STATS, RESET_STATS, CLEAR_BUFFER, SET_BUFFER_SIZE, GET_STATUS, ENABLE, DISABLE)
- Configurable buffer size (default 4096 bytes)
- Statistics tracking (read/write/ioctl counts, error count)
- Proper mutex locking and device lifecycle management

**Usage:**
```bash
sudo insmod btintel_test_generic_driver.ko
ls /dev/btintel_test_generic_driver*
cat /dev/btintel_test_generic_driver0  # Read from device
```

## Troubleshooting

### Error: "Loading of unsigned module is rejected"

This means:
1. Secure Boot is enabled
2. The module is not signed with an enrolled key
3. **Solution:** Complete Option 1 (MOK enrollment and reboot)

### Error: "key was rejected by service"

This means:
1. The MOK certificate has been imported but not yet enrolled in UEFI
2. **Solution:** Reboot and enroll the MOK when prompted at boot

### Error: "No such file or directory"

This means:
1. The module file is not in the current directory
2. **Solution:** Navigate to `/home/kirank1/code/git/linux_iosf_test/unit-test/` first

## Unloading Modules

After successful loading:

```bash
sudo rmmod pcie_test_driver
sudo rmmod btintel_test_generic_driver
```

Verify unload:
```bash
lsmod | grep -E "pcie_test|btintel_test"
```

## Verification

Once modules are loaded, verify with:

```bash
# Check if modules are loaded
lsmod | grep -E "pcie_test|btintel_test"

# Check module information
modinfo pcie_test_driver.ko
modinfo btintel_test_generic_driver.ko

# Check kernel messages
dmesg | tail -20

# Check device files
ls -la /dev/pcie_test_driver*
ls -la /dev/btintel_test_generic_driver*
```

## Additional Resources

- See `BLUETOOTH_TEST_STRATEGY.md` for Intel Bluetooth driver architecture
- See `CUSTOMIZATION.md` for device configuration options
- See `README.md` for general framework information
