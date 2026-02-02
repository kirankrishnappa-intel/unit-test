# PCIe Test Driver Framework - Kconfig Integration

**Version:** 1.0  
**Date:** February 2, 2026

## Overview

The PCIe Test Driver Framework is now fully integrated into the Linux backport project's menuconfig system. You can enable or disable the framework build through the kernel configuration menu.

## Kconfig Structure

### Location
- **Main Menu:** Linux backports Kconfig menu
- **Submenu:** "Unit Test Framework"
- **Configuration File:** `unit-test/Kconfig`

### Configuration Options

#### 1. **PCIE_TEST_DRIVER** (Main Option)
```
├─ PCIe Test Driver Framework [y/n/m]
   │
   ├─ Default: n (disabled)
   │
   └─ Description:
      PCIe Test Driver Framework is a comprehensive Linux kernel
      module and user-space utilities for testing PCIe devices.
```

**Effects:**
- When `y` or `m`: Kernel module will be built
- When `n`: Kernel module will NOT be built

#### 2. **PCIE_TEST_DRIVER_DEBUG** (Debug Option)
```
├─ Enable debug output in PCIe Test Driver [y/n]
   │
   ├─ Depends on: CONFIG_PCIE_TEST_DRIVER
   ├─ Default: n (disabled)
   │
   └─ Description:
      Enable verbose debug logging in the PCIe test driver module.
      This will output detailed information to kernel log (dmesg).
```

**Effects:**
- When `y`: Compiles with `-DDEBUG -g` flags
- When `n`: Standard optimization flags

#### 3. **BUILD_PCIE_TEST_UTILITY** (User-Space Tool)
```
├─ Build PCIe Test Utility [y/n]
   │
   ├─ Depends on: CONFIG_PCIE_TEST_DRIVER
   ├─ Default: y (enabled)
   │
   └─ Description:
      Build the user-space command-line utility for testing
      PCIe devices.
```

#### 4. **BUILD_PCIE_TEST_SCRIPTS** (Test Scripts)
```
├─ Build PCIe Test Scripts [y/n]
   │
   ├─ Depends on: CONFIG_PCIE_TEST_DRIVER
   ├─ Default: y (enabled)
   │
   └─ Description:
      Build automated test scripts including module loading,
      testing, and checkpoint tracking.
```

## Using Menuconfig

### Accessing the Configuration Menu

```bash
# In the backport project directory
make menuconfig

# Or use one of the alternatives
make nconfig    # Text interface
make xconfig    # X11 interface
make gconfig    # GTK interface
```

### Navigation Steps

1. **Start menuconfig**
   ```bash
   cd /home/kirank1/code/git/linux_iosf_test
   make menuconfig
   ```

2. **Navigate to Unit Test Framework**
   - Use arrow keys to navigate
   - Look for "Unit Test Framework" menu

3. **Enable/Disable Options**
   ```
   [ ] PCIe Test Driver Framework
   [ ] Enable debug output in PCIe Test Driver
   [ ] Build PCIe Test Utility
   [ ] Build PCIe Test Scripts
   ```

4. **Toggle Selections**
   - Press `Space` to enable/disable (checkbox `[ ]` → `[*]`)
   - Press `M` for module build (optional for kernel modules)

5. **Save Configuration**
   - Select "Save" or press `S`
   - Confirm file name (usually `.config`)

### Example Configuration Scenarios

#### Scenario 1: Enable Everything (Development)
```
[*] PCIe Test Driver Framework
[*] Enable debug output in PCIe Test Driver
[*] Build PCIe Test Utility
[*] Build PCIe Test Scripts
```

#### Scenario 2: Kernel Module Only (Production)
```
[*] PCIe Test Driver Framework
[ ] Enable debug output in PCIe Test Driver
[ ] Build PCIe Test Utility
[ ] Build PCIe Test Scripts
```

#### Scenario 3: Completely Disabled
```
[ ] PCIe Test Driver Framework
[ ] Enable debug output in PCIe Test Driver (grayed out)
[ ] Build PCIe Test Utility (grayed out)
[ ] Build PCIe Test Scripts (grayed out)
```

#### Scenario 4: Module Build (Load Separately)
```
[m] PCIe Test Driver Framework    (press M for module)
[*] Enable debug output in PCIe Test Driver
[*] Build PCIe Test Utility
[*] Build PCIe Test Scripts
```

## Building with Kconfig

### After Configuring

The configuration is saved in `.config` file:

```bash
# View generated configuration
cat .config | grep PCIE_TEST_DRIVER
```

Output example:
```
CONFIG_PCIE_TEST_DRIVER=y
CONFIG_PCIE_TEST_DRIVER_DEBUG=n
CONFIG_BUILD_PCIE_TEST_UTILITY=y
CONFIG_BUILD_PCIE_TEST_SCRIPTS=y
```

### Building the Framework

```bash
# Build based on Kconfig settings
cd unit-test
make modules

# Or from project root
make
```

The Makefile automatically:
- Reads CONFIG_PCIE_TEST_DRIVER settings
- Only builds if enabled
- Applies debug flags if CONFIG_PCIE_TEST_DRIVER_DEBUG=y
- Includes proper compiler flags

### Check Build Configuration

```bash
cd unit-test
make info
```

Output shows:
```
Module: pcie_test_driver
Kernel: 6.8.0-53-generic
Build dir: /lib/modules/6.8.0-53-generic/build
Module path: /lib/modules/6.8.0-53-generic/kernel/drivers/misc/

Kconfig Settings:
CONFIG_PCIE_TEST_DRIVER=y
CONFIG_PCIE_TEST_DRIVER_DEBUG=n
CONFIG_BUILD_PCIE_TEST_UTILITY=y
CONFIG_BUILD_PCIE_TEST_SCRIPTS=y
```

## Kconfig Dependencies

### Dependency Tree

```
CONFIG_PCIE_TEST_DRIVER (main option)
├── CONFIG_PCIE_TEST_DRIVER_DEBUG (depends on main)
├── CONFIG_BUILD_PCIE_TEST_UTILITY (depends on main)
└── CONFIG_BUILD_PCIE_TEST_SCRIPTS (depends on main)
```

### Automatic Disabling

When you disable `CONFIG_PCIE_TEST_DRIVER`:
- All dependent options become unavailable (grayed out)
- They are automatically disabled
- No manual action needed

## Kernel Configuration File Integration

### .config File Format

```
# PCIe Test Driver Framework options
CONFIG_PCIE_TEST_DRIVER=y
CONFIG_PCIE_TEST_DRIVER_DEBUG=n
CONFIG_BUILD_PCIE_TEST_UTILITY=y
CONFIG_BUILD_PCIE_TEST_SCRIPTS=y
```

### Editing Directly

You can also edit `.config` directly:

```bash
# Enable framework
sed -i 's/CONFIG_PCIE_TEST_DRIVER=n/CONFIG_PCIE_TEST_DRIVER=y/' .config

# Enable debug
sed -i 's/CONFIG_PCIE_TEST_DRIVER_DEBUG=n/CONFIG_PCIE_TEST_DRIVER_DEBUG=y/' .config

# Disable utility build
sed -i 's/CONFIG_BUILD_PCIE_TEST_UTILITY=y/CONFIG_BUILD_PCIE_TEST_UTILITY=n/' .config
```

## Makefile Integration

### How It Works

The unit-test/Makefile respects Kconfig settings:

```makefile
# Only build module if enabled
ifeq ($(CONFIG_PCIE_TEST_DRIVER),y)
obj-m := pcie_test_driver.o
else ifeq ($(CONFIG_PCIE_TEST_DRIVER),m)
obj-m := pcie_test_driver.o
else
obj-m :=
endif

# Apply debug flags if enabled
ifeq ($(CONFIG_PCIE_TEST_DRIVER_DEBUG),y)
ccflags-y += -DDEBUG -g
endif
```

### Conditional Building

```bash
# If CONFIG_PCIE_TEST_DRIVER=y
make modules        # Builds pcie_test_driver.ko

# If CONFIG_PCIE_TEST_DRIVER=n
make modules        # Skips build, obj-m is empty
```

## Build Scenarios

### Scenario A: Full Debug Build

```bash
# 1. Configure
make menuconfig
# Select: PCIE_TEST_DRIVER=y, DEBUG=y

# 2. Build
make
cd unit-test
make modules

# 3. Load with debug
sudo insmod pcie_test_driver.ko debug_level=2
```

### Scenario B: Production Build

```bash
# 1. Configure
make menuconfig
# Select: PCIE_TEST_DRIVER=y, DEBUG=n

# 2. Build
make
cd unit-test
make modules

# 3. Load without debug
sudo insmod pcie_test_driver.ko
```

### Scenario C: Module Build

```bash
# 1. Configure
make menuconfig
# Select: PCIE_TEST_DRIVER=m (press M)

# 2. Build
make
cd unit-test
make modules

# 3. Load as module
sudo insmod pcie_test_driver.ko
```

### Scenario D: Disabled

```bash
# 1. Configure
make menuconfig
# Select: PCIE_TEST_DRIVER=n

# 2. Build
make
cd unit-test
make modules
# Nothing happens - module is not built
```

## Integration with Backport Project

### File Structure

```
linux_iosf_test/
├── Kconfig                    (main config - updated)
├── Kconfig.sources
├── Kconfig.local
├── Kconfig.kernel
├── unit-test/
│   ├── Kconfig               (new - defines test options)
│   ├── Makefile              (updated - respects Kconfig)
│   ├── pcie_test_driver.c
│   ├── pcie_test_utility.c
│   ├── test_driver.sh
│   └── ... (other files)
└── ... (other dirs)
```

### Integration Points

1. **Main Kconfig** sources unit-test/Kconfig
2. **Unit-test Makefile** checks CONFIG_PCIE_TEST_DRIVER
3. **Build System** respects all Kconfig settings

## Testing Configuration

### Verify Configuration

```bash
# Check if enabled
grep CONFIG_PCIE_TEST_DRIVER .config

# Output if enabled:
CONFIG_PCIE_TEST_DRIVER=y

# Output if disabled:
CONFIG_PCIE_TEST_DRIVER=n
```

### Build Test

```bash
# With framework enabled
cd unit-test
make clean
make modules
ls -lh *.ko  # Should show pcie_test_driver.ko

# With framework disabled
# (Change config to n, then:)
cd unit-test
make clean
make modules
ls -lh *.ko  # Should be empty
```

## Advanced Configuration

### Custom Kernel Configuration

```bash
# Load existing config
cp /boot/config-$(uname -r) .config

# Update with new options
make oldconfig

# Or merge
make olddefconfig
```

### Out-of-Tree Building

```bash
# Build against different kernel
KDIR=/usr/src/linux-headers-X.Y.Z make modules

# With Kconfig enabled
KDIR=/usr/src/linux-headers-X.Y.Z make modules
```

## Troubleshooting

### Option Appears Grayed Out

**Cause:** Parent option is disabled

**Solution:** Enable `CONFIG_PCIE_TEST_DRIVER` first

### Changes Don't Take Effect

**Cause:** Configuration not saved properly

**Solution:** 
```bash
make menuconfig
# Make sure to select "Save"
# Then rebuild: make clean && make
```

### Module Not Building

**Cause:** CONFIG_PCIE_TEST_DRIVER might be disabled

**Solution:**
```bash
grep CONFIG_PCIE_TEST_DRIVER .config
# If =n, enable it in menuconfig
```

## Best Practices

1. **Always use menuconfig for configuration**
   ```bash
   make menuconfig
   ```

2. **Save configuration after changes**
   - Select "Save"
   - Confirm filename

3. **Verify configuration before building**
   ```bash
   grep CONFIG_PCIE_TEST_DRIVER .config
   ```

4. **Use `make info` to check Kconfig status**
   ```bash
   cd unit-test
   make info
   ```

5. **Keep configuration consistent**
   - Don't manually edit .config without rebuilding
   - Use menuconfig for changes

## Summary

The PCIe Test Driver Framework now supports:

✓ Kconfig integration for easy enable/disable  
✓ Debug flag configuration  
✓ Conditional building based on settings  
✓ Dependency management  
✓ Standard Linux kernel configuration workflow  

To use:

1. Run `make menuconfig`
2. Navigate to "Unit Test Framework"
3. Enable desired options
4. Save configuration
5. Build normally with `make`

---

**Last Updated:** February 2, 2026  
**Status:** ✅ Kconfig Integration Complete
