# 3DS Controller
An Up-To-Date 3ds homebrew application that lets you use your 3ds as a wireless controller for Windows and Linux

## Features
- Use your 3DS as a wireless gamepad for PC games and emulators
- Cross-platform support for Windows and Linux
- All buttons, Circle Pad, C-Stick and touchscreen support
- Configurable server IP and port
- Low latency wireless connection

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

## Building from Source

### Requirements
- DevkitPro and DevkitARM installed
- 3DS development libraries (libctru)

### Building
1. Clone the repository
2. Run `make` to build the 3DS application
3. The output file will be in the project root directory as `3ds_controller.3dsx`

## Roadmap

### Short-term Goals
- [ ] Fix false "connected" status when no server is running
- [ ] Improve mouse input handling for touchscreen
- [ ] Add ability to save multiple server configurations
- [ ] Implement connection status checking
- [ ] Add battery level indicator

### Medium-term Goals
- [ ] Add GUI improvements with Citro2D/3D
- [ ] Add controller profiles for different games/emulators
- [ ] Implement controller rumble feedback
- [ ] Automatic IP discovery of PC
- [ ] Motion controls support

### Long-term Goals
- [ ] 3D display integration
- [ ] Audio streaming from PC to 3DS
- [ ] Dual-screen emulator support
- [ ] CIA version with Wake-on-WLAN support
- [ ] Multiple controller mode (for local multiplayer)

## Troubleshooting

### Common Issues

**Issue**: 3DS cannot connect to PC  
**Solution**: Make sure both devices are on the same network and check your firewall settings to allow UDP port 8888

**Issue**: Controller inputs are not recognized  
**Solution**: Ensure the ViGEmBus driver is correctly installed on Windows or uinput module is loaded on Linux

**Issue**: Linux version fails with permission errors  
**Solution**: Make sure you're running the script with root privileges (`su -` or `sudo`)

**Issue**: High latency or lag  
**Solution**: Reduce network congestion and ensure your 3DS has a good Wi-Fi signal

## License

This project is protected by the PolyForm Noncommercial License

## Acknowledgements

- Thanks to the devkitPro team for the 3DS development tools
- Thanks to the ViGEm project for the virtual controller drivers
