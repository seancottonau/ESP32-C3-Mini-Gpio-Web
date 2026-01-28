/*
 * ESP32-C3-Mini GPIO Monitor with WiFi Configuration Portal
 * 
 * Features:
 * - WiFi configuration portal if no credentials saved or connection fails
 * - Web server displaying GPIO states (0,1,3,4,5)
 * - GPIO pins configured as INPUT_PULLUP
 * - Credentials saved to NVS (non-volatile storage)
 * - Auto-reconnect functionality
 * 
 * Required Libraries:
 * - WiFi (built-in)
 * - WebServer (built-in)
 * - Preferences (built-in)
 * - DNSServer (built-in)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>

// GPIO pins to monitor
const int GPIO_PINS[] = {0, 1, 3, 4, 5};
const int NUM_PINS = 5;

// Web server on port 80
WebServer server(80);

// DNS server for captive portal
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Preferences for storing WiFi credentials
Preferences preferences;

// WiFi credentials
String ssid = "";
String password = "";

// AP mode credentials
const char* ap_ssid = "ESP32-C3-Config";
const char* ap_password = "12345678";

// Configuration mode flag
bool configMode = false;

// Connection timeout (10 seconds)
const unsigned long WIFI_TIMEOUT = 10000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP32-C3 GPIO Monitor Starting...");
  
  // Initialize GPIO pins as INPUT_PULLUP
  initializeGPIO();
  
  // Load WiFi credentials from NVS
  loadCredentials();
  
  // Try to connect to WiFi
  if (ssid.length() > 0) {
    Serial.println("Attempting to connect to saved WiFi...");
    if (!connectToWiFi(ssid.c_str(), password.c_str())) {
      Serial.println("Failed to connect. Starting configuration portal...");
      startConfigPortal();
    }
  } else {
    Serial.println("No saved credentials. Starting configuration portal...");
    startConfigPortal();
  }
  
  // Setup web server routes
  setupWebServer();
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
  
  // Print IP address
  if (configMode) {
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("Station IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  if (configMode) {
    dnsServer.processNextRequest();
  }
  
  server.handleClient();
  
  // Check WiFi connection and attempt reconnect if needed
  if (!configMode && WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    if (!connectToWiFi(ssid.c_str(), password.c_str())) {
      Serial.println("Reconnection failed. Starting configuration portal...");
      startConfigPortal();
      setupWebServer();
    }
  }
  
  delay(10);
}

void initializeGPIO() {
  Serial.println("Initializing GPIO pins...");
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(GPIO_PINS[i], INPUT_PULLUP);
    Serial.print("GPIO ");
    Serial.print(GPIO_PINS[i]);
    Serial.println(" set as INPUT_PULLUP");
  }
}

void loadCredentials() {
  preferences.begin("wifi", false);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();
  
  if (ssid.length() > 0) {
    Serial.print("Loaded SSID: ");
    Serial.println(ssid);
  } else {
    Serial.println("No saved credentials found");
  }
}

void saveCredentials(String newSSID, String newPassword) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", newSSID);
  preferences.putString("password", newPassword);
  preferences.end();
  
  ssid = newSSID;
  password = newPassword;
  
  Serial.println("Credentials saved to NVS");
}

bool connectToWiFi(const char* ssid, const char* password) {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    configMode = false;
    return true;
  } else {
    Serial.println("\nWiFi connection failed!");
    WiFi.disconnect();
    return false;
  }
}

void startConfigPortal() {
  configMode = true;
  
  // Stop any existing WiFi connection
  WiFi.disconnect();
  delay(100);
  
  // Start Access Point
  Serial.print("Starting Access Point: ");
  Serial.println(ap_ssid);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  delay(500);
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  Serial.println("Configuration portal started");
  Serial.println("Connect to WiFi: " + String(ap_ssid));
  Serial.println("Password: " + String(ap_password));
}

void setupWebServer() {
  // Root page - different behavior for config mode vs normal mode
  server.on("/", HTTP_GET, []() {
    if (configMode) {
      handleConfigPage();
    } else {
      handleRoot();
    }
  });
  
  // GPIO status page (normal mode)
  server.on("/gpio", HTTP_GET, handleRoot);
  
  // WiFi scan page (config mode)
  server.on("/scan", HTTP_GET, handleScan);
  
  // WiFi connect handler (config mode)
  server.on("/connect", HTTP_POST, handleConnect);
  
  // Reset device handler (normal mode)
  server.on("/reset", HTTP_POST, handleReset);
  
  // API endpoint for GPIO status (JSON)
  server.on("/api/gpio", HTTP_GET, handleGPIOAPI);
  
  // Handle all other requests (captive portal)
  server.onNotFound([]() {
    if (configMode) {
      handleConfigPage();
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32-C3 GPIO Monitor</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".gpio-grid { display: grid; gap: 15px; margin: 20px 0; }";
  html += ".gpio-item { background: #f9f9f9; padding: 15px; border-radius: 5px; display: flex; justify-content: space-between; align-items: center; }";
  html += ".gpio-label { font-weight: bold; font-size: 18px; }";
  html += ".gpio-state { padding: 5px 15px; border-radius: 5px; font-weight: bold; }";
  html += ".state-high { background: #4CAF50; color: white; }";
  html += ".state-low { background: #f44336; color: white; }";
  html += ".info { background: #e3f2fd; padding: 10px; border-radius: 5px; margin: 10px 0; font-size: 14px; }";
  html += "button { background: #2196F3; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin: 5px; }";
  html += "button:hover { background: #0b7dda; }";
  html += "</style>";
  html += "<script>";
  html += "function updateGPIO() {";
  html += "  fetch('/api/gpio').then(r => r.json()).then(data => {";
  html += "    data.pins.forEach(pin => {";
  html += "      const elem = document.getElementById('state' + pin.gpio);";
  html += "      if(elem) {";
  html += "        elem.textContent = pin.state;";
  html += "        elem.className = 'gpio-state state-' + pin.state.toLowerCase();";
  html += "      }";
  html += "    });";
  html += "  });";
  html += "}";
  html += "setInterval(updateGPIO, 1000);";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>ESP32-C3 GPIO Monitor</h1>";
  html += "<div class='info'>";
  html += "Connected to: <strong>" + WiFi.SSID() + "</strong><br>";
  html += "IP Address: <strong>" + WiFi.localIP().toString() + "</strong><br>";
  html += "Signal Strength: <strong>" + String(WiFi.RSSI()) + " dBm</strong>";
  html += "</div>";
  html += "<div class='gpio-grid'>";
  
  for (int i = 0; i < NUM_PINS; i++) {
    int pin = GPIO_PINS[i];
    int state = digitalRead(pin);
    String stateStr = (state == HIGH) ? "HIGH" : "LOW";
    String stateClass = (state == HIGH) ? "state-high" : "state-low";
    
    html += "<div class='gpio-item'>";
    html += "<span class='gpio-label'>GPIO " + String(pin) + "</span>";
    html += "<span class='gpio-state " + stateClass + "' id='state" + String(pin) + "'>" + stateStr + "</span>";
    html += "</div>";
  }
  
  html += "</div>";
  html += "<div style='text-align: center; margin-top: 20px;'>";
  html += "<button onclick='updateGPIO()'>Refresh Now</button>";
  html += "<button onclick='location.href=\"/\"'>Home</button>";
  html += "<button onclick='confirmReset()' style='background: #f44336;'>Reset Device</button>";
  html += "</div>";
  html += "<div class='info' style='text-align: center; margin-top: 20px; font-size: 12px;'>";
  html += "Auto-refresh: Every 1 second | Pull-up resistors enabled";
  html += "</div>";
  html += "<script>";
  html += "function confirmReset() {";
  html += "  if (confirm('WARNING: This will erase all WiFi credentials and reboot the device.\\n\\nThe device will restart in configuration mode.\\n\\nAre you sure?')) {";
  html += "    fetch('/reset', {method: 'POST'})";
  html += "      .then(() => {";
  html += "        alert('Device is resetting... It will restart in configuration mode.');";
  html += "        window.location.href = '/';";
  html += "      });";
  html += "  }";
  html += "}";
  html += "</script>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleConfigPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32-C3 WiFi Setup</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".network { background: #f9f9f9; padding: 10px; margin: 5px 0; border-radius: 5px; cursor: pointer; }";
  html += ".network:hover { background: #e0e0e0; }";
  html += "input { width: 100%; padding: 10px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }";
  html += "button { width: 100%; background: #4CAF50; color: white; border: none; padding: 12px; border-radius: 5px; cursor: pointer; margin-top: 10px; }";
  html += "button:hover { background: #45a049; }";
  html += ".scan-btn { background: #2196F3; }";
  html += ".scan-btn:hover { background: #0b7dda; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>WiFi Configuration</h1>";
  html += "<p style='text-align: center;'>Connect your ESP32-C3 to a WiFi network</p>";
  html += "<button class='scan-btn' onclick='location.href=\"/scan\"'>Scan for Networks</button>";
  html += "<form action='/connect' method='POST' style='margin-top: 20px;'>";
  html += "<input type='text' name='ssid' placeholder='WiFi SSID' required>";
  html += "<input type='password' name='password' placeholder='WiFi Password'>";
  html += "<button type='submit'>Connect</button>";
  html += "</form>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleScan() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>WiFi Networks</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".network { background: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 5px; cursor: pointer; display: flex; justify-content: space-between; align-items: center; }";
  html += ".network:hover { background: #e0e0e0; }";
  html += ".signal { font-size: 12px; color: #666; }";
  html += ".loading { text-align: center; padding: 20px; }";
  html += "button { background: #2196F3; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin: 5px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Available Networks</h1>";
  html += "<div class='loading'>Scanning...</div>";
  
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  
  html += "<div id='networks'>";
  
  if (n == 0) {
    html += "<p style='text-align: center;'>No networks found</p>";
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      String encryption = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured";
      
      html += "<div class='network' onclick='selectNetwork(\"" + ssid + "\")'>";
      html += "<div><strong>" + ssid + "</strong><br><small>" + encryption + "</small></div>";
      html += "<div class='signal'>" + String(rssi) + " dBm</div>";
      html += "</div>";
    }
  }
  
  html += "</div>";
  html += "<div style='text-align: center; margin-top: 20px;'>";
  html += "<button onclick='location.href=\"/\"'>Back</button>";
  html += "<button onclick='location.reload()'>Rescan</button>";
  html += "</div>";
  html += "</div>";
  html += "<script>";
  html += "function selectNetwork(ssid) {";
  html += "  const password = prompt('Enter password for ' + ssid + ':');";
  html += "  if (password !== null) {";
  html += "    const form = document.createElement('form');";
  html += "    form.method = 'POST';";
  html += "    form.action = '/connect';";
  html += "    const ssidInput = document.createElement('input');";
  html += "    ssidInput.type = 'hidden';";
  html += "    ssidInput.name = 'ssid';";
  html += "    ssidInput.value = ssid;";
  html += "    const passInput = document.createElement('input');";
  html += "    passInput.type = 'hidden';";
  html += "    passInput.name = 'password';";
  html += "    passInput.value = password;";
  html += "    form.appendChild(ssidInput);";
  html += "    form.appendChild(passInput);";
  html += "    document.body.appendChild(form);";
  html += "    form.submit();";
  html += "  }";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  
  // Clean up
  WiFi.scanDelete();
}

void handleConnect() {
  String newSSID = server.arg("ssid");
  String newPassword = server.arg("password");
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Connecting...</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; text-align: center; }";
  html += ".container { max-width: 500px; margin: 50px auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += ".spinner { border: 4px solid #f3f3f3; border-top: 4px solid #3498db; border-radius: 50%; width: 40px; height: 40px; animation: spin 1s linear infinite; margin: 20px auto; }";
  html += "@keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }";
  html += ".success { color: #4CAF50; font-size: 48px; margin: 20px 0; }";
  html += ".info-box { background: #e3f2fd; padding: 15px; border-radius: 5px; margin: 20px 0; text-align: left; }";
  html += ".step { margin: 10px 0; padding: 10px; background: #f9f9f9; border-left: 4px solid #2196F3; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>WiFi Configuration Saved</h1>";
  html += "<div class='spinner'></div>";
  html += "<p>Connecting to: <strong>" + newSSID + "</strong></p>";
  html += "<div class='info-box'>";
  html += "<h3>Next Steps:</h3>";
  html += "<div class='step'><strong>1.</strong> Wait 10-15 seconds for the ESP32 to connect</div>";
  html += "<div class='step'><strong>2.</strong> Disconnect from the ESP32-C3-Config network</div>";
  html += "<div class='step'><strong>3.</strong> Reconnect to your WiFi network: <strong>" + newSSID + "</strong></div>";
  html += "<div class='step'><strong>4.</strong> Check your Serial Monitor or router for the ESP32's new IP address</div>";
  html += "<div class='step'><strong>5.</strong> Open a browser and go to that IP address</div>";
  html += "</div>";
  html += "<p><small>If connection fails, the ESP32 will restart in configuration mode.</small></p>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  
  delay(1000);
  
  // Save credentials
  saveCredentials(newSSID, newPassword);
  
  // Stop AP mode and DNS server
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  
  // Try to connect
  if (connectToWiFi(newSSID.c_str(), newPassword.c_str())) {
    configMode = false;
    Serial.println("Successfully connected to new WiFi network");
  } else {
    Serial.println("Failed to connect to new network, restarting AP mode");
    startConfigPortal();
  }
  
  // Restart the server with new configuration
  server.stop();
  delay(100);
  setupWebServer();
  server.begin();
}

void handleGPIOAPI() {
  String json = "{\"pins\":[";
  
  for (int i = 0; i < NUM_PINS; i++) {
    int pin = GPIO_PINS[i];
    int state = digitalRead(pin);
    String stateStr = (state == HIGH) ? "HIGH" : "LOW";
    
    if (i > 0) json += ",";
    json += "{\"gpio\":" + String(pin) + ",\"state\":\"" + stateStr + "\"}";
  }
  
  json += "]}";
  
  server.send(200, "application/json", json);
}

void handleReset() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Resetting Device...</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; text-align: center; }";
  html += ".container { max-width: 500px; margin: 50px auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += ".spinner { border: 4px solid #f3f3f3; border-top: 4px solid #f44336; border-radius: 50%; width: 40px; height: 40px; animation: spin 1s linear infinite; margin: 20px auto; }";
  html += "@keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }";
  html += ".warning { color: #f44336; font-size: 18px; margin: 20px 0; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Resetting Device</h1>";
  html += "<div class='spinner'></div>";
  html += "<p class='warning'>All WiFi credentials have been erased.</p>";
  html += "<p>The device is rebooting...</p>";
  html += "<p>Please wait 10 seconds, then connect to:<br><strong>ESP32-C3-Config</strong></p>";
  html += "<p><small>Password: 12345678</small></p>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  
  Serial.println("Reset requested - Clearing WiFi credentials...");
  
  // Clear all WiFi credentials from NVS
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  
  Serial.println("WiFi credentials cleared");
  Serial.println("Rebooting in 2 seconds...");
  
  delay(2000);
  
  // Reboot the device
  ESP.restart();
}
