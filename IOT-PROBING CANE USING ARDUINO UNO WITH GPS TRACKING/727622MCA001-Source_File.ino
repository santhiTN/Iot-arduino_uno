#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

const int trigPin = 5;      // Arduino pin connected to the trigger pin of the ultrasonic sensor
const int echoPin = 6;      // Arduino pin connected to the echo pin of the ultrasonic sensor
const int buzzerPin = 7;    // Arduino pin connected to the positive terminal of the buzzer
const int LED = 8;
const int minimumDistance = 25;  // Minimum distance threshold for triggering the buzzer (in cm)

const unsigned long alarmDuration = 10000;   // Alarm duration threshold (in milliseconds)
const unsigned long gpsInterval = 1000;      // GPS location update interval (in milliseconds)

unsigned long previousMillis = 0;   // Variable to store the previous time
unsigned long alarmStartTime = 0;   // Variable to store the start time of the alarm
bool alarmActive = false;           // Flag to track if the alarm is active
float latitude;
float longitude;

SoftwareSerial gpsSerial(10, 11);   // SoftwareSerial for GPS module
SoftwareSerial GSMSerial(4, 3); //Rx Tx
TinyGPSPlus gps;                  // TinyGPSPlus object to handle GPS data  

void setup()
{
  Serial.begin(9600);             // Initialize the serial communication for displaying distance and GPS data
  gpsSerial.begin(9600);          // Initialize the software serial communication for the GPS module
  GSMSerial.begin(9600);          //GSM
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED, OUTPUT);
}

void loop()
{
  // Trigger the ultrasonic sensor by sending a short pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Measure the duration of the echo pulse
  unsigned long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance in centimeters based on the speed of sound
  float distance = duration * 0.034 / 2;
  
  // Display the distance in centimeters
//  Serial.print("Distance: ");
//  Serial.print(distance);
//  Serial.println(" cm");
  
  // Check if the distance is less than the minimum threshold
  if (distance < minimumDistance)
  {
    if (!alarmActive)
	{
      alarmActive = true;
      alarmStartTime = millis();
    } 
	else
	{
      unsigned long currentMillis = millis();
      if (currentMillis - alarmStartTime >= alarmDuration)
	  {
        //getGPSLocation();
		latitude = gps.location.lat();
		longitude = gps.location.lng();
		if((latitude > 0) && (longitude > 0))
		{
			String message = ("Emergency! https://www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6));
			SendSMS_A(message);
		}
		else
		{
			String message = ("Emergency! Please try to reach out the person");
			SendSMS_A(message);
		}
		alarmActive = false;
      }
    }
    soundAlarm();
  }
  else
  {
    alarmActive = false;
  }
  
  // Check if it's time to update the GPS location
    if(gpsSerial.available() > 0)
	{
		Serial.println("updating GPS");
		if (gps.encode(gpsSerial.read()))
		{
		    if (gps.location.isValid())
		    {
//				if (millis() - previousMillis >= gpsInterval)
//				{
					latitude = gps.location.lat();
					Serial.print("Latitude: ");
					Serial.println(latitude, 6);
					longitude = gps.location.lng();
					Serial.print("Longitude: ");
					Serial.println(longitude, 6);
					//previousMillis = millis();
				//}
		    }
		}
	}
}

void soundAlarm()
{
  digitalWrite(buzzerPin, HIGH);    // Turn on the buzzer
  digitalWrite(LED, HIGH);
  delay(25);                      // Delay for 1 second
  digitalWrite(buzzerPin, LOW);     // Turn off the buzzer
  digitalWrite(LED, LOW);
  delay(25);                      // Delay for 1 second
}

void getGPSLocation()
{
  while (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
	{
      if (gps.location.isValid())
	  {
		    latitude = gps.location.lat();
        Serial.print("Latitude: ");
        Serial.println(latitude, 6);
		    longitude = gps.location.lng();
        Serial.print("Longitude: ");
        Serial.println(longitude, 6);
        break;
      }
    }
  }
}


void SendSMS_A(String msg)
{
  GSMSerial.println("AT+CMGF=1");
  delay(1000);
  Serial.print("Sending SMS A\n");
  GSMSerial.println("AT+CMGS=\"+919361754144\"\r");
  delay(1000);
  GSMSerial.println(msg);
  delay(100);
  GSMSerial.println((char)26);
  delay(1000);
  Serial.print("Check SMS in the Mobile phone +919361754144\n");
}
