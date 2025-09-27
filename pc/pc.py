import socket, struct, time, signal, sys, os, datetime, platform

debug_mode, last_update_time, last_button_state, running = False, time.time(), 0, True
throttle_interval, gamepad, connected_devices = 0.01, None, {}

# Define constants
KEY_A, KEY_B, KEY_SELECT, KEY_START = 1, 2, 4, 8
KEY_DRIGHT, KEY_DLEFT, KEY_DUP, KEY_DDOWN = 16, 32, 64, 128
KEY_R, KEY_L, KEY_X, KEY_Y, KEY_ZL, KEY_ZR = 256, 512, 1024, 2048, 4096, 8192

os_name = platform.system()
print(f"Detected OS: {os_name}")

if os_name == "Windows":
    try:
        import vgamepad as vg
        gamepad_type = "vgamepad"
    except ImportError:
        os.system('pip install vgamepad')
        try:
            import vgamepad as vg
            gamepad_type = "vgamepad"
        except ImportError:
            print("Failed to install vgamepad. Install it manually: pip install vgamepad")
            print("Make sure ViGEmBus driver is installed: https://github.com/ViGEm/ViGEmBus/releases")
            sys.exit(1)
elif os_name == "Linux":
    try:
        import uinput
        gamepad_type = "uinput"
    except ImportError:
        os.system('pip install python-uinput')
        try:
            import uinput
            gamepad_type = "uinput"
        except ImportError:
            print("Failed to install python-uinput. Install it manually: pip install python-uinput")
            print("You may also need: sudo apt-get install libudev-dev && sudo modprobe uinput")
            sys.exit(1)
else:
    print(f"Unsupported OS: {os_name}")
    sys.exit(1)

def setup_controller():
    global gamepad_type
    try:
        if gamepad_type == "vgamepad":
            gamepad = vg.VX360Gamepad()
            print("Xbox 360 controller initialized")
            gamepad.reset()
            return gamepad
        elif gamepad_type == "uinput":
            events = (
                uinput.BTN_A, uinput.BTN_B, uinput.BTN_X, uinput.BTN_Y, uinput.BTN_TL, uinput.BTN_TR,
                uinput.BTN_START, uinput.BTN_SELECT, uinput.BTN_THUMBL, uinput.BTN_THUMBR,
                uinput.ABS_X + (0, 255, 0, 0), uinput.ABS_Y + (0, 255, 0, 0),
                uinput.ABS_RX + (0, 255, 0, 0), uinput.ABS_RY + (0, 255, 0, 0),
                uinput.ABS_Z + (0, 255, 0, 0), uinput.ABS_RZ + (0, 255, 0, 0),
                uinput.ABS_HAT0X + (-1, 1, 0, 0), uinput.ABS_HAT0Y + (-1, 1, 0, 0)
            )
            device = uinput.Device(events, name="3DS Controller")
            print("Linux uinput controller initialized")
            for axis in [uinput.ABS_X, uinput.ABS_Y, uinput.ABS_RX, uinput.ABS_RY]: device.emit(axis, 128)
            for axis in [uinput.ABS_Z, uinput.ABS_RZ, uinput.ABS_HAT0X, uinput.ABS_HAT0Y]: device.emit(axis, 0)
            return device
    except Exception as e:
        print(f"Failed to initialize controller: {e}")
        if gamepad_type == "vgamepad": print("Make sure ViGEmBus driver is installed")
        elif gamepad_type == "uinput": print("Make sure uinput module is loaded: sudo modprobe uinput")
        return None

def map_axis_value(value, min_in=-140, max_in=140, min_out=-32768, max_out=32767):
    if abs(value) < 20: return 0
    value = max(min_in, min(max_in, value))
    return int(((value - min_in) * (max_out - min_out)) / (max_in - min_in) + min_out)

def map_axis_to_platform(value, target_platform):
    if target_platform == "vgamepad": return value
    elif target_platform == "uinput": return int((value + 32768) * 255 / 65535)
    return value

def signal_handler(sig, frame):
    global running
    print("\nExiting...")
    running = False

def handle_windows_controller(data, gamepad):
    global last_button_state
    buttons = struct.unpack("=I", data[0:4])[0]
    button_changes = buttons ^ last_button_state
    
    for key, vg_btn in [(KEY_A, vg.XUSB_BUTTON.XUSB_GAMEPAD_A), (KEY_B, vg.XUSB_BUTTON.XUSB_GAMEPAD_B), 
                        (KEY_X, vg.XUSB_BUTTON.XUSB_GAMEPAD_X), (KEY_Y, vg.XUSB_BUTTON.XUSB_GAMEPAD_Y),
                        (KEY_L, vg.XUSB_BUTTON.XUSB_GAMEPAD_LEFT_SHOULDER), (KEY_R, vg.XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_SHOULDER),
                        (KEY_START, vg.XUSB_BUTTON.XUSB_GAMEPAD_START), (KEY_SELECT, vg.XUSB_BUTTON.XUSB_GAMEPAD_BACK)]:
        if button_changes & key: gamepad.press_button(vg_btn) if buttons & key else gamepad.release_button(vg_btn)
    
    gamepad.left_trigger(255 if buttons & KEY_ZL else 0)
    gamepad.right_trigger(255 if buttons & KEY_ZR else 0)
    
    dpad_state = 0
    if buttons & KEY_DUP: dpad_state |= vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_UP
    if buttons & KEY_DDOWN: dpad_state |= vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_DOWN
    if buttons & KEY_DLEFT: dpad_state |= vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_LEFT
    if buttons & KEY_DRIGHT: dpad_state |= vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_RIGHT
    
    if dpad_state: gamepad.press_button(dpad_state)
    else: 
        for btn in [vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_UP, vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_DOWN, 
                   vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_LEFT, vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_RIGHT]:
            gamepad.release_button(btn)
    
    circlepad_x, circlepad_y = struct.unpack("=h", data[4:6])[0], struct.unpack("=h", data[6:8])[0]
    cstick_x, cstick_y = struct.unpack("=h", data[12:14])[0] if len(data) >= 14 else 0, struct.unpack("=h", data[14:16])[0] if len(data) >= 16 else 0
    gamepad.left_joystick(map_axis_value(circlepad_x), map_axis_value(-circlepad_y))
    gamepad.right_joystick(map_axis_value(cstick_x), map_axis_value(-cstick_y))
    gamepad.update()
    last_button_state = buttons

def handle_linux_controller(data, gamepad):
    global last_button_state
    buttons = struct.unpack("=I", data[0:4])[0]
    
    for key, ui_btn in [(KEY_A, uinput.BTN_A), (KEY_B, uinput.BTN_B), (KEY_X, uinput.BTN_X), (KEY_Y, uinput.BTN_Y), 
                        (KEY_L, uinput.BTN_TL), (KEY_R, uinput.BTN_TR), (KEY_START, uinput.BTN_START), (KEY_SELECT, uinput.BTN_SELECT)]:
        if (buttons & key) != (last_button_state & key): gamepad.emit(ui_btn, 1 if buttons & key else 0)
    
    gamepad.emit(uinput.ABS_Z, 255 if buttons & KEY_ZL else 0)
    gamepad.emit(uinput.ABS_RZ, 255 if buttons & KEY_ZR else 0)
    gamepad.emit(uinput.ABS_HAT0Y, -1 if buttons & KEY_DUP else 1 if buttons & KEY_DDOWN else 0)
    gamepad.emit(uinput.ABS_HAT0X, -1 if buttons & KEY_DLEFT else 1 if buttons & KEY_DRIGHT else 0)
    
    circlepad_x, circlepad_y = struct.unpack("=h", data[4:6])[0], struct.unpack("=h", data[6:8])[0]
    cstick_x, cstick_y = struct.unpack("=h", data[12:14])[0] if len(data) >= 14 else 0, struct.unpack("=h", data[14:16])[0] if len(data) >= 16 else 0
    gamepad.emit(uinput.ABS_X, map_axis_to_platform(map_axis_value(circlepad_x), "uinput"))
    gamepad.emit(uinput.ABS_Y, map_axis_to_platform(map_axis_value(-circlepad_y), "uinput"))
    gamepad.emit(uinput.ABS_RX, map_axis_to_platform(map_axis_value(cstick_x), "uinput"))
    gamepad.emit(uinput.ABS_RY, map_axis_to_platform(map_axis_value(-cstick_y), "uinput"))
    gamepad.syn()
    last_button_state = buttons

def handle_controller_state(data, addr, gamepad):
    global last_update_time, last_button_state, connected_devices, gamepad_type
    
    ip_address = addr[0]
    device_key = ip_address
    
    if device_key not in connected_devices:
        current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{current_time}] New 3DS connected from {ip_address}")
        connected_devices[device_key] = {"last_seen": time.time(), "name": f"3DS at {ip_address}"}
    else:
        connected_devices[device_key]["last_seen"] = time.time()
    
    current_time = time.time()
    if current_time - last_update_time < throttle_interval:
        return
    last_update_time = current_time
    
    if debug_mode:
        print(f"Received data length: {len(data)} bytes")
        print(f"Raw data: {data.hex()}")
    
    if len(data) == 5 and data == b'ping\x00':
        server_socket.sendto(b'pong', addr)
        if debug_mode:
            print(f"Ping received from {ip_address}, sent pong response")
        return
    
    try:
        if len(data) >= 16:
            if gamepad_type == "vgamepad": handle_windows_controller(data, gamepad)
            elif gamepad_type == "uinput": handle_linux_controller(data, gamepad)
        elif debug_mode:
            print(f"Error: Data too short ({len(data)} bytes)")
    except Exception as e:
        if debug_mode: print(f"Error processing controller data: {e}")

def check_inactive_devices():
    global connected_devices
    current_time = time.time()
    devices_to_remove = []
    
    for addr, device_info in connected_devices.items():
        if current_time - device_info["last_seen"] > 10:
            current_time_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            print(f"[{current_time_str}] 3DS at {addr} disconnected (timeout)")
            devices_to_remove.append(addr)
    
    for addr in devices_to_remove:
        connected_devices.pop(addr)

def main():
    global debug_mode, running, gamepad, gamepad_type, server_socket
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    if len(sys.argv) > 1 and sys.argv[1] == "--debug":
        debug_mode = True
        print("Debug mode enabled")
    
    gamepad = setup_controller()
    if not gamepad:
        print("Cannot continue without virtual controller. Exiting.")
        sys.exit(1)
    
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind(('0.0.0.0', 8888))
    server_socket.setblocking(False)
    
    print("3DS Controller PC Receiver")
    print(f"Running in {gamepad_type} mode for {os_name}")
    print("Listening on port 8888...")
    print("Press Ctrl+C to exit")
    
    last_check_time = time.time()
    
    try:
        while running:
            try:
                data, addr = server_socket.recvfrom(1024)
                if debug_mode: print(f"Received {len(data)} bytes from {addr}")
                handle_controller_state(data, addr, gamepad)
            except BlockingIOError:
                time.sleep(0.001)
                current_time = time.time()
                if current_time - last_check_time > 2:
                    check_inactive_devices()
                    last_check_time = current_time
            except ConnectionResetError as e:
                print(f"Connection error: {e}")
                time.sleep(1)
    finally:
        print("Shutting down...")
        if gamepad and gamepad_type == "vgamepad":
            gamepad.reset()
        server_socket.close()
        print("Socket closed. Exiting.")

if __name__ == "__main__":
    main()