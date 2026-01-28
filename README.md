# ESP32-C3-Mini GPIO Monitor with WiFi Portal

Complete Arduino sketch for ESP32-C3-Mini that monitors GPIO pins and provides a WiFi configuration portal.

## Features

✅ **WiFi Configuration Portal** - Automatic AP mode if no credentials saved or connection fails  
✅ **Web Server** - Real-time GPIO monitoring via web interface  
✅ **GPIO Monitoring** - Monitors pins 0, 1, 3, 4, 5 (configured as INPUT_PULLUP)  
✅ **Non-Volatile Storage** - WiFi credentials saved to flash memory  
✅ **Auto-Reconnect** - Automatically reconnects if WiFi connection drops  
✅ **Network Scanner** - Scan and select from available WiFi networks  
✅ **Responsive UI** - Mobile-friendly web interface  
✅ **Real-time Updates** - Auto-refresh GPIO states every second  

## Hardware Requirements

- ESP32-C3-Mini module
- USB connection for programming and power
- Optional: Buttons/switches connected to GPIO pins 0, 1, 3, 4, 5

## Software Requirements

### Arduino IDE Setup

1. **Install Arduino IDE** 
   - Download from: https://www.arduino.cc/en/software

2. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to **File → Preferences**
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to **Tools → Board → Board Manager**
   - Search for "esp32"
   - Install "esp32 by Espressif Systems" (version 2.0.11 or later)

3. **Select Board**
   - Go to **Tools → Board → ESP32 Arduino**
   - Select **ESP32C3 Dev Module**

4. **Configure Board Settings**
   - **USB CDC On Boot:** Enabled
   - **CPU Frequency:** 160MHz
   - **Flash Size:** 4MB (or your module's size)
   - **Partition Scheme:** Default 4MB with spiffs
   - **Upload Speed:** 921600

## Installation

1. **Download the sketch**: `esp32_gpio_monitor.ino`

2. **Open in Arduino IDE**

3. **Verify/Compile** the sketch (Ctrl+R / Cmd+R)

4. **Upload** to your ESP32-C3-Mini (Ctrl+U / Cmd+U)
   - Put the board in download mode if necessary (hold BOOT button while pressing RESET)

5. **Open Serial Monitor** (Ctrl+Shift+M) at 115200 baud to view status messages

## First-Time Setup

### Initial WiFi Configuration

1. **Power on the ESP32-C3**
   - On first boot, no WiFi credentials are saved
   - The device automatically starts in Access Point mode

2. **Connect to the ESP32-C3 AP**
   - SSID: `ESP32-C3-Config`
   - Password: `12345678`

3. **Configure WiFi**
   - Open a web browser
   - You should be automatically redirected to the configuration page
   - If not, navigate to: `http://192.168.4.1`

4. **Select Your Network**
   - Click "Scan for Networks"
   - Click on your WiFi network from the list
   - Enter the password when prompted
   - Click "Connect"

5. **Connection**
   - The ESP32-C3 will attempt to connect to your network
   - If successful, it will switch to Station mode
   - The credentials are saved to flash memory
   - The Serial Monitor will display the new IP address

## Usage

### Normal Operation (Station Mode)

Once connected to your WiFi network:

1. **Find the IP Address**
   - Check your router's DHCP client list
   - Or use a network scanner app

2. **Access the Web Interface**
   - Open a web browser
   - Navigate to the ESP32-C3's IP address (e.g., `http://192.168.1.100`)

3. **Monitor GPIO Pins**
   - The web page displays real-time status of all monitored pins
   - GREEN = HIGH (pulled up, no connection to ground)
   - RED = LOW (connected to ground)
   - Auto-refreshes every second

### GPIO Pin Configuration

All monitored pins (0, 1, 3, 4, 5) are configured as:
- **INPUT_PULLUP** - Internal pull-up resistor enabled
- **Default State:** HIGH (when nothing connected)
- **Active State:** LOW (when connected to ground)

**Typical Usage:**
```
ESP32-C3 GPIO Pin → [Button/Switch] → GND

When button is pressed: Pin reads LOW
When button is released: Pin reads HIGH
```

### Web Interface Features

**Main Dashboard:**
- Current WiFi SSID and signal strength
- IP address information
- Real-time GPIO status for all pins
- Manual refresh button
- Auto-refresh (every 1 second)

**API Endpoint:**
- JSON endpoint available at: `/api/gpio`
- Returns GPIO states in JSON format
- Useful for integration with other systems

Example JSON response:
```json
{
  "pins": [
    {"gpio": 0, "state": "HIGH"},
    {"gpio": 1, "state": "LOW"},
    {"gpio": 3, "state": "HIGH"},
    {"gpio": 4, "state": "HIGH"},
    {"gpio": 5, "state": "LOW"}
  ]
}
```

## Troubleshooting

### Cannot Connect to WiFi

**Symptom:** ESP32-C3 stays in AP mode after entering credentials

**Solutions:**
1. Verify WiFi password is correct
2. Ensure WiFi network is 2.4GHz (ESP32-C3 doesn't support 5GHz)
3. Check if network has MAC address filtering
4. Try moving ESP32-C3 closer to the router
5. Check Serial Monitor for error messages

### Configuration Portal Not Appearing

**Symptom:** Cannot see the ESP32-C3-Config network

**Solutions:**
1. Power cycle the device
2. Check Serial Monitor to confirm AP mode started
3. Erase flash memory and re-upload:
   ```
   Tools → Erase Flash → All Flash Contents
   ```
4. Verify the board is powered properly

### Web Page Not Loading

**Symptom:** Cannot access the web interface

**Solutions:**
1. Verify ESP32-C3 is connected to WiFi (check Serial Monitor)
2. Ping the IP address to verify network connectivity
3. Try accessing via `http://` (not `https://`)
4. Clear browser cache
5. Try a different browser or device

### GPIO Readings Incorrect

**Symptom:** Pin shows wrong state

**Solutions:**
1. Verify physical connections
2. Check for loose wires
3. Ensure switches/buttons connect to GND (not VCC)
4. Check Serial Monitor to verify pin initialization
5. Test with a multimeter to confirm pin voltage

### Serial Monitor Shows Garbled Text

**Symptom:** Cannot read Serial Monitor output

**Solutions:**
1. Set baud rate to 115200
2. Select correct COM port
3. Ensure USB CDC is enabled in board settings
4. Try a different USB cable

## Resetting WiFi Credentials

To clear saved WiFi credentials and return to configuration mode:

**Method 1: Via Code**
```cpp
// Add this to setup(), upload, then remove it
preferences.begin("wifi", false);
preferences.clear();
preferences.end();
```

**Method 2: Erase Flash**
- In Arduino IDE: **Tools → Erase Flash → All Flash Contents**
- Then re-upload the sketch

## Customization

### Change Monitored GPIO Pins

Edit this line in the code:
```cpp
const int GPIO_PINS[] = {0, 1, 3, 4, 5};  // Change to your desired pins
const int NUM_PINS = 5;                    // Update count
```

### Change AP Credentials

Edit these lines:
```cpp
const char* ap_ssid = "ESP32-C3-Config";     // Change SSID
const char* ap_password = "12345678";        // Change password (min 8 chars)
```

### Change Connection Timeout

Edit this line:
```cpp
const unsigned long WIFI_TIMEOUT = 10000;  // Timeout in milliseconds
```

### Disable Auto-Refresh

Remove or comment out this line in the HTML:
```javascript
setInterval(updateGPIO, 1000);
```

## Security Considerations

⚠️ **Important Security Notes:**

1. **Default AP Password:** Change the default AP password (`12345678`) to something more secure
2. **No Authentication:** The web interface has no password protection
3. **Local Network Only:** Do not expose directly to the internet
4. **WiFi Credentials:** Stored in plaintext in NVS (consider encryption for production)

## Advanced Features

### Adding Authentication

To add basic authentication to the web server, add this to `setup()`:
```cpp
server.on("/", HTTP_GET, [](){
  if (!server.authenticate("admin", "password")) {
    return server.requestAuthentication();
  }
  handleRoot();
});
```

### Static IP Configuration

To use a static IP instead of DHCP:
```cpp
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);

WiFi.config(local_IP, gateway, subnet, primaryDNS);
```

## Technical Details

### Memory Usage

- **Flash:** ~1MB (program + WiFi libraries)
- **RAM:** ~40KB (web server + buffers)
- **NVS:** ~4KB (WiFi credentials)

### Power Consumption

- **Active (WiFi on):** ~80-120mA
- **AP Mode:** ~100-150mA
- **Deep Sleep (not implemented):** ~5µA

### Network Protocols

- **HTTP:** Port 80 (web server)
- **DNS:** Port 53 (captive portal)
- **DHCP:** Client mode

## Support

For issues or questions:
1. Check the Serial Monitor output for error messages
2. Verify your ESP32-C3 board is genuine
3. Ensure you have the latest ESP32 Arduino core
4. Check ESP32 Arduino GitHub issues for known problems

## License

This code is provided as-is for educational and personal use.

## Version History

- **v1.0** - Initial release
  - WiFi configuration portal
  - GPIO monitoring
  - Web server interface
  - NVS credential storage

- **v1.1** - Stability and Reset Function

  New Features:
  - Reset Device Button

  ** Red button on the main GPIO Monitor page
  ** Includes a JavaScript confirmation dialog with a warning message

  When confirmed, it:
    - Clears all WiFi credentials from non-volatile storage (NVS)
    - Shows a confirmation page with instructions
    - Reboots the ESP32 after 2 seconds
    - Device restarts in configuration mode (AP mode)

  Safety Features:
    - Double confirmation (browser confirm dialog before action)
    - Clear warning message about what will happen
    - Instructions on how to reconnect after reset
    - Visual feedback with a red-colored button to indicate this is a destructive action
  
  User Flow:
    - User clicks "Reset Device" button
    - Browser shows confirmation: "WARNING: This will erase all WiFi credentials and reboot the device..."
    - If confirmed, credentials are cleared and device reboots
    - User sees message to reconnect to "ESP32-C3-Config" network
    - Device boots up in configuration mode, ready to be reconfigured
