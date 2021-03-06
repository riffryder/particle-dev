#include "./ArduinoJson-v5.13.2.h"
#include "./Adafruit_ILI9341/Adafruit_ILI9341.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(A2, A1, A0);

//
// ─── CONTENT POSITIONING ────────────────────────────────────────────────────────
//

#define SYMBOL_START_V 72
#define SYMBOL_START_H 112

#define PRICE_START_V 144
#define PRICE_START_H 112


//
// ─── GLOBAL VARIABLES ───────────────────────────────────────────────────────────
//

// Memory pool for JSON object tree.
StaticJsonBuffer<200> jsonBuffer;

String symbol = "null";
String price = "0";
unsigned long ms = 1000;
int blinkLed = D7;
const unsigned char icon[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0xe0, 0x00, 0x00, 0x00,
	0x3f, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0xe0, 0x00,
	0x00, 0x00, 0x0f, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x3f, 0xf8,
	0xe0, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xf8, 0xe0, 0x00, 0x10, 0x01,
	0xff, 0x38, 0xe0, 0x00, 0x38, 0x03, 0xfe, 0x18, 0xe0, 0x00, 0x7c, 0x07, 0xfc, 0x00, 0xe0, 0x00,
	0xfe, 0x0f, 0xf8, 0x00, 0xe0, 0x01, 0xff, 0x1f, 0xf0, 0x00, 0xe0, 0x03, 0xff, 0xbf, 0xe0, 0x00,
	0xe0, 0x07, 0xff, 0xff, 0xc0, 0x00, 0xe0, 0x0f, 0xff, 0xff, 0x80, 0x00, 0xe0, 0x1f, 0xff, 0xff,
	0x00, 0x00, 0xe0, 0x3f, 0xef, 0xfe, 0x00, 0x00, 0xe0, 0x7f, 0xc7, 0xfc, 0x00, 0x00, 0xe0, 0xff,
	0x83, 0xf8, 0x00, 0x00, 0xe1, 0xff, 0x01, 0xf0, 0x00, 0x00, 0xe3, 0xfe, 0x00, 0xe0, 0x00, 0x00,
	0xe3, 0xfc, 0x00, 0x40, 0x00, 0x00, 0xe1, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf0, 0x00, 0x00,
	0x00, 0x00, 0xe0, 0x60, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//
// ─── DEVICE SETUP ──────────────────────────────────────────────────────────────────────
//

void setup() {
	// Setup serial comms for debug
	Serial.begin(9600);
	while (!Serial)
	{
		// wait serial port initialization
	}
	Serial.println("Hello World!");

	// Event listener
	Particle.subscribe("stocks", realtimeHandler);

	// Sync time with particle cloud
	Particle.syncTime();
	waitUntil(Particle.syncTimeDone);

	pinMode(blinkLed, OUTPUT);

	// Initial screen setup
	tft.begin();
	tft.setRotation(1);
	tft.fillScreen(ILI9341_WHITE);

	// Draw top header bar
	drawHeader();
}

//
// ─── MAIN LOOP ──────────────────────────────────────────────────────────────────
//

void loop(void) {

	// render just what changed (time)
	tft.setTextSize(1);
	tft.setFont(CALIBRI_48);
	tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);

	// Stock symbol
	tft.setCursor(SYMBOL_START_H, SYMBOL_START_V);
	tft.println(symbol);
	// Stock price
	tft.setCursor(PRICE_START_H, PRICE_START_V);
	tft.println(price);

	delay(500);

}

//
// ─── UTIL FUNCTIONS ─────────────────────────────────────────────────────────────
//

void realtimeHandler(const char *event, const char *data)
{
	Serial.print("Recieved event: ");
	Serial.println(event);
	if (data) {
		int length = strlen(data) + 1;
		char dataCopy[length];
		strcpy(dataCopy, data);
		Serial.println(dataCopy);
		JsonObject &root = jsonBuffer.parseObject(dataCopy);
		if (!root.success())
		{
			Particle.publish("parseObject() failed");
			return;
		}

		// Update JSON data into our display variables
		symbol = root["Symbol"].asString();
		price = root["Price"].asString();
	}
	else {
		Serial.println("NULL");
	}
	digitalWrite(blinkLed, HIGH);
	delay(500);
	digitalWrite(blinkLed, LOW);
}

void drawHeader()
{
	// Don't edit these
  tft.setCursor(96, 12);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
	tft.setFont(CALIBRI_24);
	tft.drawFastHLine(0, 50, 320, ILI9341_BLACK);
	tft.drawFastVLine(60, 0, 50, ILI9341_BLACK);

	// Edit here
	tft.println("   Stock    Updates");
	tft.drawBitmap(6, 0, icon, 48, 48, ILI9341_GREEN);
}
