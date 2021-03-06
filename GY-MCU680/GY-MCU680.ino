//GY_MCU680 Air Quality Sensor

#include <Arduino.h>
#include <HardwareSerial.h>
#include "WiFi.h"

HardwareSerial sensorSerial(2);

#define RXD2 16
#define TXD2 17

uint16_t temp1 = 0;
int16_t temp2 = 0;

unsigned char Re_buf[30], counter = 0;
unsigned char sign = 0;
int led = 13;

void setup()
{
	Serial.begin(9600);
	sensorSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
	WiFi.mode(WIFI_MODE_STA);
	Serial.print("Device MAC ID: ");
	Serial.println(WiFi.macAddress());
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
			}
			delay(1000);
		}
	}
}