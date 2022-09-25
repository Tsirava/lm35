#define sensorPin 36
#include <driver/adc.h>
#define LED_PIN 2
#define TIME_TO_SLEEP 5000 //time to sleep 1 min in the real system, here 5 sec for the demo
#define uS_TO_mS_FACTOR 1000ULL
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
const char* ssid = "pocoSteve";      // Mettre votre SSID Wifi
const char* password = "steve1234";  // Mettre votre mot de passe Wifi

String message = "";

int time_wait = 100;
void setup() {
  // Begin serial communication at a baud rate of 115200:
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_mS_FACTOR); // set up the time for light sleep mode
}
void loop() {

  adc1_config_width(ADC_WIDTH_BIT_12);
  // Configure attenuation of ADC1, Channel 0 to 11dB (full-scale voltage of 3.9V)
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  // Read the voltage on ADC1, Channel 0 (GPIO 36)
  int value = adc1_get_raw(ADC1_CHANNEL_0);
  // Print the read value (0-4095)
  Serial.println("Value: " + String(value));
  // Convert the read value to millivolt, using the full-scale configured with the attenuation (3.9V) and the number of bits (4095)
  float voltage = (float)value * 1100. / 4095.;
  //Caclculation of the temperture with lm35
  float temperature = value / 10;

  // Print the vaoltage in millivolts (0-3.9V)
  Serial.println("Millivolts: " + String(voltage));
  Serial.print(millis());
  Serial.print(" - ");
  Serial.print(temperature);
  Serial.println("°c\n");




  //if temp is above the limit then wait 5sec for next measure
  if (temperature > 21.0) {
    time_wait = 2000;
    //if temp stay above the limit then measure every 1sec then alert the user
    if (temperature >= 21) {
      

      Serial.println("\n");
      WiFi.begin(ssid, password);                // Initialisation avec WiFi.begin / ssid et password
      Serial.print("Attente de connexion ...");  // Message d'attente de connexion
      while (WiFi.status() != WL_CONNECTED)      // Test connexion

      {
        Serial.print(".");  // Affiche des points .... tant que connexion n'est pas OK
      }

      Serial.println("\n");
      Serial.println("Connexion etablie !");  // Affiche connexion établie
      Serial.print("Adresse IP: ");
      Serial.println(WiFi.localIP());
      digitalWrite(LED_PIN, HIGH);
      Serial.println(" Alert \n");
      message = 'Alert';
      sendDataLm35(temperature);

      time_wait = 1000;
    }

  }
  //if temp is under the limit measure only every 5 sec
  else {
    digitalWrite(LED_PIN, LOW);
    //time_wait = 5000;
    esp_light_sleep_start();
  }

  delay(time_wait);  // time to wait  between readings
}

void sendDataLm35(float temperature) {
  if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status

    HTTPClient http;

    http.begin("192.168.40.161", 8080);  //Specify the URL and certificate
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    StaticJsonDocument<200> lm35;

    // Add values in the document
    //
    doc["temperature"] = temperature;
    doc["message"] = "Alert";
    doc["type"] = "lm35";
    lm35["sensor"] = doc;
    
    String requestBody;
    serializeJson(lm35, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {

      String response = http.getString();

      Serial.println(httpResponseCode);
      Serial.println(response);

    } else {

      //   Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(statusCode).c_str());
    }
    //http.end(); //Free the resources
  }

}