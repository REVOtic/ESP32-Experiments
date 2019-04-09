#include "BluetoothSerial.h" //Header File for Serial Bluetooth

BluetoothSerial BTSerial; //Object for Bluetooth

int incoming;
int LED_BUILTIN = 2;

void setup()
{
	Serial.begin(9600);				   //Start Serial monitor in 9600
	BTSerial.begin("BLE_LED_Control"); //Name of your Bluetooth Signal
	Serial.println("Bluetooth Device is Ready to Pair");

	pinMode(LED_BUILTIN, OUTPUT); //Specify that LED pin is output
}

void loop()
{

	if (BTSerial.available()) //Check if we receive anything from Bluetooth
	{
		incoming = BTSerial.read(); //Read what we recevive
		Serial.print("Received:");
		Serial.println(incoming);

		if (incoming == 49)
		{
			digitalWrite(LED_BUILTIN, HIGH);
			BTSerial.println("LED turned ON");
		}

		if (incoming == 48)
		{
			digitalWrite(LED_BUILTIN, LOW);
			BTSerial.println("LED turned OFF");
		}
	}
	delay(20);
}