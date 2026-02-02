# Intel Bluetooth PCIe Test Strategy

## Analysis of btintel_pcie Code

The existing driver (`drivers/bluetooth/btintel_pcie.c`, 3396 lines) has comprehensive functionality:

**Key Device Operations:**
- Device enumeration & PCI configuration (8 device IDs: A876, E476, E376, D346, 6E74, 4D76, 2732, 2731)
- BAR memory mapping & register access
- MSI-X interrupt handling (8+ handler types: GP0/GP1, TX, RX, FW trigger, HW exception)
- DMA queue management (TX/RX queues with 128-byte alignment)
- Firmware loading & boot stages (ROM → IML → OPFW → D0/D3)
- HCI packet processing (CMD/ACL/SCO/EVT/ISO with 4-byte Intel header)
- Device memory access (DRAM buffers, SMEM, hardware exception dumps)
- Power management (suspend/resume/hibernate)
- Error recovery & device reset

---

## Proposed Test Architecture Options

### **Option A: Extend pcie_test_driver.c with Bluetooth-Specific Tests**
**Approach:** Add Bluetooth device detection & test commands to existing generic driver

**Pros:**
- Single loadable module for all device testing
- Leverages existing BAR/config access infrastructure
- Reuses checkpoint tracking & stats framework
- Simpler build (no duplicate code)

**Cons:**
- Driver becomes bloated with device-specific logic
- Hard to disable Bluetooth tests separately
- Mixed HCI and generic register testing

**Implementation:**
```c
// In pcie_test_driver.c, add:
#ifdef CONFIG_BTINTEL_PCIE_TESTS
  btintel_device_detected();  // Register vendor-specific handlers
  btintel_test_fw_boot();     // Test firmware boot sequence
  btintel_test_dma_queues();  // Test TX/RX DMA operations
#endif
```

**Effort:** Medium (few hundred lines)

---

### **Option B: Create Separate btintel_pcie_test.c Module (RECOMMENDED)**
**Approach:** Standalone test module that reuses btintel_pcie.h and key register definitions

**Pros:**
- Clean separation of concerns
- Independent enable/disable in Kconfig
- Can focus deeply on Bluetooth-specific test scenarios
- Easier to maintain alongside official driver
- Reuses register definitions but not code dependencies
- Full compatibility with pcie_test_driver framework

**Cons:**
- Duplicate some register access code
- Two modules to load/manage

**Implementation Structure:**
```
btintel_pcie_test.c (1500+ lines)
├── Device Detection (reuse PCI IDs from btintel_pcie.h)
├── Boot Sequence Tests
│   ├── ROM Stage
│   ├── IML (Intermediate Loader)
│   └── OPFW (Operational Firmware)
├── Register Tests
│   ├── CSR Read/Write
│   ├── Boot Control Register
│   └── Function Control Register
├── Interrupt Tests
│   ├── MSI-X Configuration
│   ├── Handler Invocation
│   └── Interrupt Statistics
├── DMA Tests
│   ├── TX Queue Setup
│   ├── RX Queue Setup
│   └── Buffer Transfer Verification
├── Memory Access Tests
│   ├── DRAM Buffer Read
│   ├── SMEM Read (max 256KB)
│   └── Hardware Exception Dump
└── Integration Tests
    ├── Firmware Download Simulation
    ├── Device Reset
    └── Power State Transitions

btintel_pcie_test.h
├── Test Data Structures
├── IOCTL Command Definitions
└── Register Bit Field Helpers

btintel_pcie_test_util.c
├── Device Enumeration
├── Boot Stage Polling
├── Register Read/Write CLI
└── Test Report Generation

unit-test/Kconfig (additions)
├── CONFIG_BTINTEL_PCIE_TEST (bool, depends on BTINTEL_PCIE)
├── CONFIG_BTINTEL_PCIE_TEST_DEBUG (bool)
└── CONFIG_BUILD_BTINTEL_PCIE_TEST_UTIL (bool)
```

**Reused Code from btintel_pcie:**
- Register address macros (#define BTINTEL_PCIE_CSR_BASE, etc.)
- Boot stage enums (BTINTEL_PCIE_ROM, FW_DL, HCI_RESET, D0, D3)
- Device ID table (8 Intel Bluetooth PCIe devices)
- MSI-X register mappings & bit definitions
- DMA buffer management constants
- HCI packet type definitions

**Effort:** High (1500+ lines, comprehensive testing)

---

### **Option C: Generic Device-Specific Test Template**
**Approach:** Create reusable pattern for device-specific tests in framework

**Pros:**
- Establishes pattern for other Intel PCIe devices
- Modular, extensible architecture
- Can test multiple device types with single framework
- Clean plugin system

**Cons:**
- Most abstract approach (highest learning curve)
- Requires framework redesign

**Structure:**
```c
// device_test_plugin.h
struct device_test_ops {
  int (*detect)(struct pci_dev *pdev);
  int (*test_boot)(struct pci_dev *pdev);
  int (*test_registers)(struct pci_dev *pdev);
  int (*test_dma)(struct pci_dev *pdev);
  struct test_stats (*report)(void);
};

// For Bluetooth: btintel_pcie_ops
struct device_test_ops btintel_pcie_ops = {
  .detect = btintel_detect,
  .test_boot = btintel_test_boot,
  ...
};
```

**Effort:** Very High (framework redesign + plugin + tests)

---

### **Option D: User-Space Test Suite (Lowest Kernel Impact)**
**Approach:** Pure user-space tests using `/sys` and `/proc` interfaces

**Pros:**
- No kernel module needed
- Easy to debug & iterate
- Can use standard testing frameworks (gtest, pytest)
- Non-intrusive to kernel

**Cons:**
- Limited access to kernel internals
- Requires kernel driver to expose test interfaces
- Can't directly test interrupt handlers

**Implementation:**
```bash
# btintel_pcie_test_suite.sh / Python test framework
├── Read /sys/kernel/debug/btintel_pcie/* (if exposed)
├── Query device registers via debugfs
├── Trigger firmware boot via standard ioctl
├── Monitor sysfs for boot stage transitions
└── Validate state via HCI commands
```

**Effort:** Medium (300-500 lines of bash/Python)

---

## Recommendation: **Option B (Separate btintel_pcie_test.c)**

### Why:
1. **Highest Test Coverage** - Can comprehensively test Intel Bluetooth specific scenarios
2. **Clean Architecture** - No bloat to generic PCIe driver, proper separation
3. **Independent Control** - Enable/disable Bluetooth tests via Kconfig
4. **Code Reuse** - Leverages register definitions from btintel_pcie.h
5. **Maintenance** - Evolves independently with official driver changes
6. **Framework Integration** - Fits naturally with existing pcie_test_driver infrastructure
7. **Extensibility** - Pattern can be reused for other Intel devices (WiFi, etc.)

### Key Test Scenarios to Cover:
```
Priority 1 (Core):
├── Device Detection (PCI ID matching)
├── BAR Mapping & CSR Access
├── Boot Stage Sequence (ROM → IML → OPFW)
└── Basic Register Read/Write

Priority 2 (Functional):
├── MSI-X Interrupt Configuration
├── TX/RX DMA Queue Setup
├── Firmware Image Loading (mock)
└── Device Reset Sequence

Priority 3 (Advanced):
├── Memory Access (DRAM/SMEM)
├── Hardware Exception Capture
├── Power State Transitions
└── Error Recovery
```

### Integration with Existing Framework:
```
Shared:
├── pcie_test_driver.c - Generic BAR/config access
├── pcie_test_utility.c - Base device enumeration
└── test_driver.sh - Checkpoint tracking

Bluetooth-Specific:
├── btintel_pcie_test.c - Intel Bluetooth device tests
├── btintel_pcie_test.h - Bluetooth IOCTL definitions
├── btintel_pcie_test_util.c - Bluetooth CLI wrapper
└── btintel_pcie_tests.sh - Bluetooth test automation
```

---

## Next Steps (If Approved):

1. **Create btintel_pcie_test.h**
   - IOCTL definitions for Bluetooth test operations
   - Boot stage test commands
   - DMA queue test structures

2. **Create btintel_pcie_test.c**
   - Device detection & initialization
   - Boot sequence polling logic
   - Register validation tests
   - Interrupt handler test stubs

3. **Create btintel_pcie_test_util.c**
   - CLI wrapper for Bluetooth tests
   - Boot stage status queries
   - Register dump utilities

4. **Update Kconfig**
   - CONFIG_BTINTEL_PCIE_TEST (depends on CONFIG_PCIE_TEST_DRIVER)
   - CONFIG_BTINTEL_PCIE_TEST_DEBUG

5. **Create Test Scripts**
   - btintel_pcie_tests.sh with checkpoint integration
   - Auto-detection and validation

---

## Code Reuse Matrix

| Code Element | Option A | Option B | Option C | Option D |
|---|---|---|---|---|
| btintel_pcie.h (defines) | ✅ Direct | ✅ Direct | ✅ Direct | ✗ No |
| BAR/Config access | ✅ Reuse | ✅ Separate | ✅ Plugin | ✅ Via sysfs |
| Interrupt handling | ✅ Reuse | ✅ Separate | ✅ Plugin | ✗ No |
| DMA infrastructure | ✅ Reuse | ✅ Separate | ✅ Plugin | ⚠️ Limited |
| Device IDs | ✅ Reuse | ✅ Reuse | ✅ Plugin | ✅ Reuse |
| Kernel Module | Single | Dual | Plugin | None |
| Kconfig Options | Shared | Separate | Unified | None |
| Effort | Medium | High | VHigh | Medium |
| Maintainability | Medium | High | High | Low |
| Test Coverage | Good | Excellent | Excellent | Limited |

