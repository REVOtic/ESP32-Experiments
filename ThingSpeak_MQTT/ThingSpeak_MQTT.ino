#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ThingSpeak.h>

HardwareSerial sensorSerial(2);

WiFiClient client;

#define RXD2 16
#define TXD2 17

//----------------  Fill in required Credentails   ---------------------
char ssid[] = "WiFi-SSID";							// WiFi Network SSID (name)
char pass[] = "WiFi-PASS";							// WiFi Network password
unsigned long climateOrbChannel = 757685;			// ClimateOrb Channel ID
const char *myWriteAPIKey = "API-KEY-ClimateOrb"; 	// ThingSpeak Write API Key
//----------------------------------------------------------------------

uint16_t temp1 = 0;
int16_t temp2 = 0;

unsigned char Re_buf[30], counter = 0;
unsigned char sign = 0;
int led = 13;

void setup()
{
	//Initialize serial and wait for port to open:
	Serial.begin(9600);
	sensorSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
	WiFi.mode(WIFI_MODE_STA);
	Serial.print("Device MAC ID: ");
	Serial.println(WiFi.macAddress());
	ThingSpeak.begin(client);
	delay(4000);

	sensorSerial.write(0XA5);
	sensorSerial.write(0X55);
	sensorSerial.write(0X3F);
	sensorSerial.write(0X39);
	delay(100);

	sensorSerial.write(0XA5);
	sensorSerial.write(0X56);
	sensorSerial.write(0X02);
	sensorSerial.write(0XFD);
	delay(100);
}

void loop()
{
	float Temperature, Humidity;
	unsigned char i = 0, sum = 0;
	uint32_t Gas;
	uint32_t Pressure;
	uint16_t IAQ;
	int16_t Altitude;
	uint8_t IAQ_accuracy;
	
	// Connect or reconnect to WiFi
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		while (WiFi.status() != WL_CONNECTED)
		{
			WiFi.begin(ssid, pass);
			Serial.print(".");
			delay(5000);
		}
		Serial.println("\nConnected.");
	}

	while (sensorSerial.available())
	{
		Re_buf[counter] = (unsigned char)sensorSerial.read();

		if (counter == 0 && Re_buf[0] != 0x5A)
			return;
		if (counter == 1 && Re_buf[1] != 0x5A)
		{
			counter = 0;
			return;
		};
		counter++;
		if (counter == 20)
		{
			counter = 0;
			sign = 1;
		}
	}

	if (sign)
	{
		sign = 0;

		if (Re_buf[0] == 0x5A && Re_buf[1] == 0x5A)
		{

			for (i = 0; i < 19; i++)
				sum += Re_buf[i];
			if (sum == Re_buf[i])
			{
				temp2 = (Re_buf[4] << 8 | Re_buf[5]);
				Temperature = (float)temp2 / 100;
				temp1 = (Re_buf[6] << 8 | Re_buf[7]);
				Humidity = (float)temp1 / 100;
				Pressure = ((uint32_t)Re_buf[8] << 16) | ((uint16_t)Re_buf[9] << 8) | Re_buf[10];
				IAQ_accuracy = (Re_buf[11] & 0xf0) >> 4;
				IAQ = ((Re_buf[11] & 0x0F) << 8) | Re_buf[12];
				Gas = ((uint32_t)Re_buf[13] << 24) | ((uint32_t)Re_buf[14] << 16) | ((uint16_t)Re_buf[15] << 8) | Re_buf[16];
				Altitude = (Re_buf[17] << 8) | Re_buf[18];
				Serial.print("Temperature:");
				Serial.print(Temperature);
				Serial.print(" ,Humidity:");
				Serial.print(Humidity);
				Serial.print(" ,Pressure:");
				Serial.print(Pressure);
				Serial.print("  ,IAQ:");
				Serial.print(IAQ);
				Serial.print(" ,Gas:");
				Serial.print(Gas);
				Serial.print("  ,Altitude:");
				Serial.print(Altitude);
				Serial.print("  ,IAQ_accuracy:");
				Serial.println(IAQ_accuracy);

				// Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
				// pieces of information in a channel.  Here, we write to field 1, 2, 3, 4, 5, 6.
				ThingSpeak.setField(1, float(Temperature));
				ThingSpeak.setField(2, float(Humidity));
				ThingSpeak.setField(3, int(Pressure));
				ThingSpeak.setField(4, int(IAQ));
				ThingSpeak.setField(5, int(Gas));
				ThingSpeak.setField(6, int(Altitude));
				ThingSpeak.setField(7, int(IAQ_accuracy));
				if (IAQ_accuracy == 0)
				{
					ThingSpeak.setStatus("Sensor is Stabilizing...");
				}
				else if (IAQ_accuracy == 1)
				{
					ThingSpeak.setStatus("Data too uncertain to Calibrate. Waiting!");
				}
				else if (IAQ_accuracy == 2)
				{
					ThingSpeak.setStatus("Found New Data. Calibrating!");
				}
				else if (IAQ_accuracy == 3)
				{
					ThingSpeak.setStatus("Sensor Calibrated Successfully!!");
				}
				else
				{
					ThingSpeak.setStatus("Error in Data!");
				}

				int uploadResult = ThingSpeak.writeFields(climateOrbChannel, myWriteAPIKey);

				// Check the return code
				if (uploadResult == 200)
				{
					Serial.println("Channel Update Successful.");
				}
				else
				{
					Serial.println("Problem in Updating Channel. HTTP Error Code " + String(uploadResult));
				}
			}
			delay(20000); // Wait 20 seconds before sending a new value
		}
	}
}