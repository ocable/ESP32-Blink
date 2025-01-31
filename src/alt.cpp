/****************************************************************************************************************************
  Generic_WebSocketClientSocketIO_WiFiNINA.ino
  For Generic boards using WiFiNINA Shield/Module

  Based on and modified from WebSockets libarary https://github.com/Links2004/arduinoWebSockets
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/WebSockets_Generic
  Licensed under MIT license

  Created on: 06.06.2016
  Original Author: Markus Sattler
 *****************************************************************************************************************************/

#if !defined(ESP32)
#error This code is intended to run only on the ESP32 boards ! Please check your Tools->Board setting.
#endif

#define _WEBSOCKETS_LOGLEVEL_ 4

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient_Generic.h>
#include <SocketIOclient_Generic.h>

WiFiMulti wifiMulti;
SocketIOclient socketIO;

// Select the IP address according to your local network
IPAddress clientIP(192, 168, 2, 232);

// Select the IP address according to your local network
IPAddress serverIP(192, 168, 2, 30);
uint16_t serverPort = 8080;

int status = WL_IDLE_STATUS;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h

char ssid[] = "your_ssid"; // your network SSID (name)
char pass[] = "12345678";  // your network password (use for WPA, or use as key for WEP), length must be 8+

void socketIOEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        Serial.println("[IOc] Disconnected");

        break;

    case sIOtype_CONNECT:
        Serial.print("[IOc] Connected to url: ");
        Serial.println((char *)payload);

        // join default namespace (no auto join in Socket.IO V3)
        socketIO.send(sIOtype_CONNECT, "/");

        break;

    case sIOtype_EVENT:
        Serial.print("[IOc] Get event: ");
        Serial.println((char *)payload);

        break;

    case sIOtype_ACK:
        Serial.print("[IOc] Get ack: ");
        Serial.println(length);

        // hexdump(payload, length);

        break;

    case sIOtype_ERROR:
        Serial.print("[IOc] Get error: ");
        Serial.println(length);

        // hexdump(payload, length);

        break;

    case sIOtype_BINARY_EVENT:
        Serial.print("[IOc] Get binary: ");
        Serial.println(length);

        // hexdump(payload, length);

        break;

    case sIOtype_BINARY_ACK:
        Serial.print("[IOc] Get binary ack: ");
        Serial.println(length);

        // hexdump(payload, length);

        break;

    case sIOtype_PING:
        Serial.println("[IOc] Get PING");

        break;

    case sIOtype_PONG:
        Serial.println("[IOc] Get PONG");

        break;

    default:
        break;
    }
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("WebSockets Client IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void setup()
{
    Serial.begin(115200);

    while (!Serial)
        ;

    delay(200);

    Serial.print("\nStart ESP32_WebSocketClientSocketIO on ");
    Serial.println(ARDUINO_BOARD);
    Serial.println(WEBSOCKETS_GENERIC_VERSION);

    wifiMulti.addAP(ssid, pass);

    Serial.print("Connecting to ");
    Serial.println(ssid);

    // WiFi.disconnect();
    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();

    // Client address
    Serial.print("WebSockets Client started @ IP address: ");
    Serial.println(WiFi.localIP());

    // server address, port and URL
    Serial.print("Connecting to WebSockets Server @ IP address: ");
    Serial.print(serverIP);
    Serial.print(", port: ");
    Serial.println(serverPort);

    // setReconnectInterval to 10s, new from v2.5.1 to avoid flooding server. Default is 0.5s
    socketIO.setReconnectInterval(10000);

    socketIO.setExtraHeaders("Authorization: 1234567890");

    // server address, port and URL
    // void begin(IPAddress host, uint16_t port, String url = "/socket.io/?EIO=4", String protocol = "arduino");
    // To use default EIO=4 from v2.5.1
    socketIO.begin(serverIP, serverPort);

    // event handler
    socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;

void loop()
{
    socketIO.loop();

    uint64_t now = millis();

    if (now - messageTimestamp > 30000)
    {
        messageTimestamp = now;

        // creat JSON message for Socket.IO (event)
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();

        // add evnet name
        // Hint: socket.on('event_name', ....
        array.add("event_name");

        // add payload (parameters) for the event
        JsonObject param1 = array.createNestedObject();
        param1["now"] = (uint32_t)now;

        // JSON to String (serializion)
        String output;
        serializeJson(doc, output);

        // Send event
        socketIO.sendEVENT(output);

        // Print JSON for debugging
        Serial.println(output);
    }
}