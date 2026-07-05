/*
MIT License

Copyright (c) 2026 Warmanelcpp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <WiFi.h>
#include "esp_wifi.h"

// Declaration of the internal ESP32 raw transmit function
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

byte channel;

// Custom SSIDs Array - Add or modify names here (keep under 32 characters)
const char* ssid_list[] = {
  "Test1",
  "Test2",
  "Test3",
  "Test4",
  "Test5",
  "Test6",
  "Test7",
  "Test8",
  "Test9",
  "Test10",
  "Test11",
  "Test12",
  "Test13",
  "Test14",
  "Test15",
  "Test16",
  "Test17",
  "Test18",
  "Test19",
  "Test20",
  "Test21",
  "Test22",
  "Test23",
  "Test24",
  "Test25",
  "Test26",
  "Test27",
  "Test28",
  "Test29",
  "Test30",
  "Test31",
  "Test32",
  "Test33",
  "Test34",
  "Test35",
  "Test36",
  "Test37",
  "Test38",
  "Test39",
  "Test40",
};

// Automatically calculate the number of items in the array
const int ssid_count = sizeof(ssid_list) / sizeof(ssid_list[0]);
int current_ssid_index = 0; 

// Base 802.11 Beacon frame buffer template
uint8_t packet[128] = { 
  0x80, 0x00, 0x00, 0x00,               // Frame Control, Duration
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   // Destination address (Broadcast)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   // Source address - overwritten dynamically
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   // BSSID - overwritten dynamically
  0xc0, 0x6c,                           // Sequence Control
  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  0x64, 0x00,                           // Beacon interval (100 TU)
  0x01, 0x04,                           // Capability info
  0x00                                  // Element ID for SSID (0x00)
};

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.println("Starting Beacon transmission from array list...");
  
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_max_tx_power(82);
  
  // Set antenna select pin if using specific hardware variants
  pinMode(21, OUTPUT);
  digitalWrite(21, LOW); // Ceramic/printed antenna on by default
}

void broadcastFromList() {
  // Select a random channel between 1 and 11
  channel = random(1, 12); 
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  delay(1);   

  // Generate a randomized MAC address structure for this transmission
  for (int i = 10; i <= 15; i++) {
    uint8_t rand_byte = random(256);
    packet[i] = rand_byte;       // Source MAC
    packet[i + 6] = rand_byte;   // BSSID
  }

  // Fetch the current string from the array
  const char* current_ssid = ssid_list[current_ssid_index];
  int ssid_len = strlen(current_ssid);
  if (ssid_len > 32) ssid_len = 32; // Enforce maximum 802.11 SSID specification limit

  // Write SSID length into the 802.11 Element length field
  packet[37] = ssid_len;

  // Copy string bytes into the frame buffer immediately following the length field
  for (int i = 0; i < ssid_len; i++) {
    packet[38 + i] = current_ssid[i];
  }

  // Mandatory 802.11 Information Elements that follow the SSID: Supported Rates & DS Parameter Set
  uint8_t postSSID[] = {
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, // Supported Rates
    0x03, 0x01, channel                                         // DS Parameter Set Tag + Current Channel
  };

  // Append information tags sequentially based on the current string length offset
  for (size_t i = 0; i < sizeof(postSSID); i++) {
    packet[38 + ssid_len + i] = postSSID[i];
  }

  // Calculate total active length of the frame layout
  int total_frame_len = 38 + ssid_len + sizeof(postSSID);

  esp_wifi_80211_tx(WIFI_IF_AP, packet, total_frame_len, false);

  // Cycle index position to the next SSID string, wrapping around at array boundaries
  current_ssid_index = (current_ssid_index + 1) % ssid_count;
}

void loop() {
  broadcastFromList();
  delay(10); // Short buffer window for radio scheduler stability
}