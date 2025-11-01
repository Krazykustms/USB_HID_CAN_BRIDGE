# Project Structure

## Directory Layout

```
epic_can_logger/
├── epic_can_logger.ino      # Main Arduino sketch
├── epic_variables.h          # EPIC ECU variable definitions
├── sd_logger.h               # SD card logger header
├── sd_logger.cpp             # SD card logger implementation
├── rusefi_dbc.h              # rusEFI DBC parser header
├── rusefi_dbc.cpp            # rusEFI DBC parser implementation
│
├── README.md                 # Main project documentation
├── QUICK_START.md            # 5-minute setup guide
├── DEPENDENCIES.md           # Library requirements
├── PROJECT_STRUCTURE.md      # This file
│
├── ASSEMBLY_BLUEPRINT.md     # Hardware assembly instructions
├── STEP_BY_STEP_ASSEMBLY.md # Detailed assembly guide
├── PIN_DIAGRAM.md            # GPIO pin assignments
├── PIN_ASSIGNMENT_AUDIT.md   # Pin selection rationale
├── PRE_ASSEMBLY_CHECKLIST.md # Safety checklist
│
├── EPIC_LOGGING_GUIDE.md     # How to use EPIC variable logging
├── DBC_INTEGRATION.md        # rusEFI DBC parsing documentation
├── WIFI_SETUP.md             # WiFi access point configuration
│
├── DEBUG_GUIDE.md            # Debug system usage
├── DEBUG_STATUS.md           # Current debug configuration
├── DEBUG_REMOVAL_BENEFITS.md # Performance impact analysis
│
├── PERFORMANCE_OPTIMIZATIONS.md # Speed optimizations applied
├── COMMUNICATION_FLOW_OPTIMIZATION.md # Non-blocking design
│
├── CODE_REVIEW.md            # Code analysis findings
├── ERRORS_FIXED.md            # Initial error fixes
├── LOGIC_ERRORS_FIXED.md     # Logic error corrections
├── FUNCTIONALITY_VERIFICATION.md # Feature verification
│
└── [Various feature docs]    # Additional documentation
```

---

## Core Source Files

### epic_can_logger.ino
**Purpose**: Main application entry point  
**Size**: ~1000+ lines  
**Key Functions**:
- `setup()`: Initialization (CAN, USB, WiFi, SD, buttons)
- `loop()`: Main task scheduler (priority-based)
- `handleCanRx()`: CAN message processing with time budgets
- `requestVar()`: EPIC variable request function
- `sendCMD()`: Button/keyboard CAN transmission

**Configuration Sections**:
- Debug flags (lines 15-94)
- GPIO pin assignments (lines 96-125)
- CAN settings (lines 127-143)
- WiFi settings (lines 145-152)
- LED/shift light settings (lines 154-161)

---

### epic_variables.h
**Purpose**: EPIC ECU variable definitions  
**Content**:
- Variable ID hashes (int32_t)
- Variable names (for logging)
- Variable request array
- Variable response tracking structure

**Example**:
```cpp
#define VAR_ID_TPS_VALUE      1272048601
#define VAR_ID_RPM_VALUE      1699696209
#define VAR_ID_AFR_VALUE      -1093429509
```

---

### sd_logger.h / sd_logger.cpp
**Purpose**: SD card logging with ring buffer  
**Key Features**:
- Ring buffer (4096 bytes) for efficient writes
- Automatic file rotation (LOG0001.csv, LOG0002.csv, ...)
- CSV format with timestamps
- Non-blocking flush with time budgets

**Functions**:
- `sdLoggerInit()`: Initialize SD card and SPI
- `sdLoggerStart()`: Start logging to new file
- `sdLoggerWriteEntry()`: Add entry to ring buffer
- `sdLoggerTask()`: Periodic flush (non-blocking)
- `sdLoggerStop()`: Stop logging and close file

---

### rusefi_dbc.h / rusefi_dbc.cpp
**Purpose**: Parse rusEFI CAN broadcast messages (IDs 512-522)  
**Key Features**:
- Bit-level signal extraction (Motorola format)
- Signed/unsigned handling
- Scaling and offset application
- Optimized paths for common signal sizes

**Structures**:
- `rusefi_base0_t` through `rusefi_base10_t`
- Signal definitions (RPM, TPS, MAP, Lambda, etc.)

**Functions**:
- `dbc_decode_base0()` through `dbc_decode_base10()`
- `dbc_extract_signal_fast()`: Optimized bit extraction

---

## Documentation Files by Category

### Getting Started
- **README.md**: Project overview and introduction
- **QUICK_START.md**: Fast setup guide
- **DEPENDENCIES.md**: Library installation
- **PROJECT_STRUCTURE.md**: This file

### Hardware Assembly
- **ASSEMBLY_BLUEPRINT.md**: Complete assembly guide
- **STEP_BY_STEP_ASSEMBLY.md**: Detailed instructions
- **PIN_DIAGRAM.md**: GPIO pin layout
- **PIN_ASSIGNMENT_AUDIT.md**: Pin selection rationale
- **PRE_ASSEMBLY_CHECKLIST.md**: Safety checklist

### Usage Guides
- **EPIC_LOGGING_GUIDE.md**: EPIC variable logging
- **DBC_INTEGRATION.md**: rusEFI DBC parsing
- **WIFI_SETUP.md**: WiFi configuration
- **DEBUG_GUIDE.md**: Debug system usage

### Technical Documentation
- **PERFORMANCE_OPTIMIZATIONS.md**: Speed improvements
- **COMMUNICATION_FLOW_OPTIMIZATION.md**: Non-blocking design
- **CODE_REVIEW.md**: Code quality analysis

### Debugging & Troubleshooting
- **DEBUG_STATUS.md**: Current debug configuration
- **DEBUG_REMOVAL_BENEFITS.md**: Performance analysis
- **ERRORS_FIXED.md**: Initial errors resolved
- **LOGIC_ERRORS_FIXED.md**: Logic corrections
- **FUNCTIONALITY_VERIFICATION.md**: Feature tests

### Feature Documentation
- Various `.md` files for specific features (shift light, buttons, etc.)

---

## File Naming Conventions

### Source Files
- **`.ino`**: Arduino main sketch
- **`.h`**: Header files (declarations)
- **`.cpp`**: Implementation files

### Documentation Files
- **`README.md`**: Main project documentation
- **`*_GUIDE.md`**: How-to guides
- **`*_BLUEPRINT.md`**: Hardware assembly
- **`*_DIAGRAM.md`**: Visual documentation
- **`*_AUDIT.md`**: Analysis/review
- **`*_FIXED.md`**: Error corrections
- **`*_OPTIMIZATION.md`**: Performance work

---

## Key Constants & Definitions

### Location: epic_can_logger.ino

**Debug Configuration** (lines 15-94):
```cpp
#define DEBUG_ENABLED           1
#define DEBUG_CAN_RX            1
#define DEBUG_CAN_TX            1
// ... etc
```

**GPIO Pins** (lines 96-125):
```cpp
#define CAN_TX_PIN              5
#define CAN_RX_PIN              4
#define SD_CS_PIN               10
// ... etc
```

**CAN Settings** (lines 127-143):
```cpp
#define ECU_ID                  1
#define CAN_SPEED_KBPS          500
#define VAR_REQUEST_INTERVAL_MS 50
```

**WiFi Settings** (lines 145-152):
```cpp
#define WIFI_AP_SSID            "EPIC_CAN_LOGGER"
#define WIFI_AP_PASSWORD        "password123"
```

---

## Code Organization Principles

### 1. Modularity
- **CAN handling**: Separate functions for RX/TX
- **SD logging**: Self-contained module (`sd_logger.*`)
- **DBC parsing**: Separate module (`rusefi_dbc.*`)
- **Variable management**: Separate header (`epic_variables.h`)

### 2. Non-Blocking Design
- Time budgets for all operations
- Priority-based task scheduling
- Ring buffers to decouple producers/consumers
- `yield()` calls during delays

### 3. Performance Optimization
- Inline functions for hot paths
- Direct memory access where safe
- Pre-computed constants
- Minimal Serial output (controlled by debug flags)

### 4. Safety & Reliability
- Bounds checking on arrays
- Validation of received data
- Watchdog timer for recovery
- Error counters with thresholds

---

## Build Output Files

### Compiled Binary
**Location**: `Arduino/build/esp32.esp32s3/` (Arduino IDE)  
**File**: `epic_can_logger.ino.bin`  
**Size**: ~500-800 KB (depending on optimization level)

### Compilation Statistics
Typical output:
```
Sketch uses 623456 bytes (47%) of program storage space.
Global variables use 45678 bytes (13%) of dynamic memory.
```

---

## Version Control Recommendations

### Should Commit
- ✅ All `.ino`, `.h`, `.cpp` source files
- ✅ All `.md` documentation files
- ✅ Configuration templates (if any)

### Should NOT Commit
- ❌ Build artifacts (`build/` directory)
- ❌ Compiled binaries (`.bin` files)
- ❌ IDE-specific files (`.vscode/`, `.idea/`, etc.)
- ❌ Library cache files

### Recommended .gitignore
```
# Build outputs
build/
*.bin
*.elf

# IDE files
.vscode/
.idea/
*.code-workspace

# OS files
.DS_Store
Thumbs.db

# Arduino IDE
*.hex
```

---

## Adding New Features

### 1. Source Code
- Add functions to `epic_can_logger.ino`
- Or create new `.cpp`/`.h` files for modules

### 2. Configuration
- Add constants to configuration section
- Update pin assignments if needed

### 3. Documentation
- Create `FEATURE_NAME.md` guide
- Update `README.md` with feature description
- Add to appropriate category in this file

### 4. Testing
- Enable debug output for feature
- Test in development environment
- Verify no blocking in critical paths

---

**Status**: ✅ Project structure documented and organized for easy navigation.

