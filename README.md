# 3DS Controller
An Up-To-Date 3ds homebrew application that lets you use your 3ds as a wireless controller for Windows and Linux

## Features
- Use your 3DS as a wireless gamepad for PC games and emulators
- Cross-platform support for Windows and Linux
- All buttons, Circle Pad, C-Stick and touchscreen support
- Configurable server IP and port
- Low latency wireless connection

# But why?
- Well I know [an application like this already exists](https://github.com/CTurt/3DSController), but it seems to have stopped working entirely + it's no longer being mantained. 

## Requirements

### 3DS Requirements
- Nintendo 3DS with custom firmware (CFW)
- Homebrew Launcher access
- Wi-Fi connection (same network as your PC)

### PC Requirements
- Windows or Linux PC
- Python 3.6+ installed
- For Windows: ViGEmBus driver installed
- For Linux: uinput kernel module enabled
- Both devices must be on the same network

## Installation

### 3DS Setup
1. Copy the `3ds_controller.3dsx` file to your 3DS SD card in the `/3ds/` folder
2. Launch the Homebrew Launcher on your 3DS
3. Run the 3DS Controller application

### PC Setup

#### Windows
1. Install the ViGEmBus driver from [here](https://github.com/ViGEm/ViGEmBus/releases)
2. Install Python 3.6 or higher
3. Install required packages:
   ```
   pip install vgamepad
   ```
4. Run the PC receiver script:
   ```
   python pc.py
   ```

#### Linux
1. **Important**: You must run as root for uinput access
2. Ensure uinput module is loaded:
   ```
   sudo modprobe uinput
   ```
3. Install required dependencies:
   ```
   sudo apt-get install libudev-dev
   pip install python-uinput
   ```
4. Run the PC receiver script with root privileges:
   ```
   su -
   python pc.py
   ```
   or
   ```
   sudo python pc.py
   ```

## Usage

1. Start the PC receiver script on your computer
2. Launch the 3DS Controller application on your 3DS
3. Configure the IP address in the 3DS app to match your PC's IP address
4. Connect and start using your 3DS as a controller!

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
| Touch Screen | Currently mapped to mouse movement |

## Configuration

### 3DS Configuration
- The 3DS application creates a config file at `/config.ini` on your SD card
- You can edit this file to change the server IP and port
- Default port is 8888

### PC Configuration
- The PC script listens on all interfaces on port 8888
- Run with `--debug` flag to see detailed logging:
  ```
  python pc.py --debug
  ```

# Building from Source

### Requirements
- DevkitPro and DevkitARM installed
- 3DS development libraries (libctru)

## Building from Source

### Requirements

#### For 3DS Application
- DevkitPro and DevkitARM installed
- 3DS development libraries (libctru)
- Make utility

#### For PC Application
- Python 3.6+
- For Windows: ViGEmBus driver
- For Linux: uinput module and development headers

### Building the 3DS Application

1. Install DevkitPro with 3DS support
   ```
   # Windows
   # Download and run the installer from https://devkitpro.org/wiki/Getting_Started

   # Linux/macOS
   wget https://apt.devkitpro.org/install-devkitpro-pacman
   chmod +x ./install-devkitpro-pacman
   sudo ./install-devkitpro-pacman
   sudo dkp-pacman -S 3ds-dev
   ```

2. Set up environment variables (if not set by installer)
   ```
   # Add to your .bashrc or .zshrc
   export DEVKITPRO=/opt/devkitpro
   export DEVKITARM=${DEVKITPRO}/devkitARM
   ```

3. Clone the repository
   ```
   git clone https://github.com/yourusername/3ds-controller.git
   cd 3ds-controller
   ```

4. Build the application
   ```
   make clean
   make
   ```

5. The output file will be in the project root directory as `3ds_controller.3dsx`

### Building for Windows Development

1. Ensure you have Python 3.6+ installed
2. Install required packages:
   ```
   pip install vgamepad
   ```
3. Install ViGEmBus driver from [here](https://github.com/ViGEm/ViGEmBus/releases)
4. No compilation needed - run the Python script directly:
   ```
   python pc.py
   ```

### Building for Linux Development

1. Install required system dependencies:
   ```
   sudo apt-get install python3-dev libudev-dev
   ```
2. Install Python packages:
   ```
   pip install python-uinput
   ```
3. Ensure uinput module is loaded:
   ```
   sudo modprobe uinput
   ```
4. No compilation needed - run the Python script with root privileges:
   ```
   sudo python pc.py
   ```
5. the 3ds app is the same process as windows, except for installing devkitpro. go follow that.

### Goals
- [ ] Fix false "connected" status when no server is running
- [ ] Improve mouse input handling for touchscreen
- [ ] Add ability to save multiple server configurations
- [ ] Implement connection status checking
- [ ] Add battery level indicator
- [ ] Add a customizable keybind for turning off lcd (to save battery life)
- [ ] Add multiple device connections for a local multiplayer type thing (really meant for emulators)

## License

This project is protected by the PolyForm Noncommercial License
