# 3DS Controller
An up-to-date 3DS homebrew application that lets you use your Nintendo 3DS as a wireless controller for Windows and Linux.

## Features
- Use your 3DS as a wireless gamepad for PC games and emulators
- Cross-platform support for Windows and Linux
- Support for all buttons, Circle Pad, and C-Stick
- Configurable server IP and port
- Low latency wireless connection
- Battery level indicator
- LCD toggle option to save battery

## Why Create This?
[A similar application](https://github.com/CTurt/3DSController) exists but is no longer maintained or functional.

[![wakatime](https://wakatime.com/badge/user/5089f166-a996-455c-8cbe-f75a0e2076db.svg)](https://wakatime.com/@5089f166-a996-455c-8cbe-f75a0e2076db)

## Requirements

### 3DS
- Nintendo 3DS with custom firmware (CFW)
- Homebrew Launcher access
- Wi-Fi connection (same network as PC)

### PC
- Windows or Linux
- Python 3.6+
- Windows: ViGEmBus driver
- Linux: uinput kernel module

## Installation

### 3DS Setup
1. Copy `3ds_controller.3dsx` to your 3DS SD card in `/3ds/` folder
2. Launch Homebrew Launcher on your 3DS
3. Run the 3DS Controller application

### PC Setup

#### Windows
1. Install [ViGEmBus driver](https://github.com/ViGEm/ViGEmBus/releases)
2. Install Python 3.6+
3. Install required packages:
   ```
   pip install vgamepad
   ```
4. Run the PC receiver:
   ```
   python pc.py
   ```

#### Linux
1. Load uinput module:
   ```
   sudo modprobe uinput
   ```
2. Install dependencies:
   ```
   sudo apt-get install libudev-dev
   pip install python-uinput
   ```
3. Run PC receiver with root privileges:
   ```
   su -
   modprobe python-uinput
   python3 path/to/pc.py
   ```

## Usage
1. Start PC receiver script
2. Launch 3DS Controller on your 3DS
3. Set the IP address in the 3DS app to match your PC's IP
4. Connect and start using your 3DS as a controller

### Control Mapping

| 3DS Input | PC Controller Output |
|-----------|----------------------|
| A/B/X/Y Buttons | A/B/X/Y Buttons |
| Circle Pad | Left Analog Stick |
| C-Stick | Right Analog Stick |
| D-Pad | D-Pad |
| L/R Buttons | LB/RB (Left/Right Bumpers) |
| ZL/ZR Buttons | LT/RT (Left/Right Triggers) |
| Start/Select | Start/Back |

## Configuration

### 3DS
- Config file at `/config.ini` on SD card
- Edit to change server IP and port
- Default port: 8888

### PC
- Listens on all interfaces on port 8888
- Debug mode: `python pc.py --debug`

## Building from Source

### Requirements

#### 3DS Application
- DevkitPro and DevkitARM
- 3DS development libraries (libctru)
- Make utility

#### PC Application
- Python 3.6+
- Windows: ViGEmBus driver
- Linux: uinput module and headers

# Building 3DS Application

1. Install DevkitPro with 3DS support
   ```
   # Windows: Download from https://devkitpro.org/wiki/Getting_Started

   # Linux/macOS
   wget https://apt.devkitpro.org/install-devkitpro-pacman
   chmod +x ./install-devkitpro-pacman
   sudo ./install-devkitpro-pacman
   sudo dkp-pacman -S 3ds-dev
   ```

2. Install CIA building tools (makerom and bannertool)
   ```
   # Windows
   # Download makerom and bannertool from:
   # - makerom: https://github.com/3DSGuy/Project_CTR/releases
   # - bannertool: https://github.com/Steveice10/bannertool/releases
   # Add both to your PATH

   # Linux/macOS
   git clone https://github.com/3DSGuy/Project_CTR.git
   cd Project_CTR/makerom
   make
   sudo cp makerom /usr/local/bin

   git clone https://github.com/Steveice10/bannertool.git
   cd bannertool
   make
   sudo cp output/*/bannertool /usr/local/bin
   ```

3. Set environment variables (if needed)
   ```
   export DEVKITPRO=/opt/devkitpro
   export DEVKITARM=${DEVKITPRO}/devkitARM
   ```

4. Clone repository
   ```
   git clone https://github.com/icicle1133/3ds-controller.git
   cd 3ds-controller
   ```

5. Build application
   ```
   # For 3DSX file
   make clean
   make

   # For CIA file
   make cia
   ```

6. Output files:
   - `3ds_controller.3dsx` (Homebrew format)
   - `3ds_controller.cia` (CIA format for installation via FBI)

# Development Setup

#### Windows
1. Install Python 3.6+
2. Install packages: `pip install vgamepad`
3. Install ViGEmBus driver
4. Run: `python pc.py`

#### Linux
1. Install dependencies: `sudo apt-get install python3-dev libudev-dev`
2. Install packages: `pip install python-uinput`
3. Load uinput: `sudo modprobe uinput`
4. Run: `sudo python pc.py`

# Project Status

### Completed
- [x] Fixed false "connected" status when no server is running
- [x] Implemented connection status checking
- [x] Added battery level indicator
- [x] Added customizable button for turning off LCD

### Pending
- [ ] Add ability to save multiple server configurations
- [ ] Add multiple device connections for local multiplayer
- [ ] Re-add keyboard/touchscreen support like CTurt's 3ds Controller had.

## License
This project is protected by the PolyForm Noncommercial License