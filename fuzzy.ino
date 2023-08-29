#include <WiFi.h>
#include "ThingSpeak.h"
#include <DHT.h>
#include <Fuzzy.h>

#define DHTPIN 23      // Pin where the DHT22 sensor is connected
#define DHTTYPE DHT22  // DHT22 Sensor

const char *ssid = "Rookie";         // your network SSID (name)
const char *password = "wifi 2023";  // your network password

Fuzzy *fuzzy = new Fuzzy();

WiFiClient client;

unsigned long myChannelNumber = 2249782;         // Replace with your ThingSpeak Channel number
const char *myWriteAPIKey = "11MJ1VAFUTEEY6H1";  // Replace with your ThingSpeak Write API Key

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  // Delay between updates (30 seconds)

FuzzyInput *temperature = new FuzzyInput(1);
FuzzySet *cold = new FuzzySet(0, 0, 0, 50);       // Definir conjuntos difusos para temperatura
FuzzySet *medium = new FuzzySet(0, 50, 50, 100);  // Definir conjuntos difusos para temperatura
FuzzySet *hot = new FuzzySet(50, 100, 100, 100);  // Definir conjuntos difusos para temperatura


FuzzyInput *humidity = new FuzzyInput(2);
FuzzySet *dry = new FuzzySet(50, 100, 100, 100);  // Definir conjuntos difusos para humedad
FuzzySet *normal = new FuzzySet(0, 50, 50, 100);  // Definir conjuntos difusos para humedad
FuzzySet *wet = new FuzzySet(0, 0, 0, 50);      
  // Definir conjuntos difusos para humedad
FuzzyOutput *speed = new FuzzyOutput(1);
FuzzySet *slow = new FuzzySet(0, 0, 0, 50);         // Conjunto difuso para velocidad lenta
FuzzySet *moderate = new FuzzySet(0, 50, 50, 100);  // Conjunto difuso para velocidad moderada
FuzzySet *fast = new FuzzySet(50, 100, 100, 100);   // Conjunto difuso para velocidad rápida


// Definir conjuntos difusos para velocidad



void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

temperature->addFuzzySet(cold);
temperature->addFuzzySet(medium);
temperature->addFuzzySet(hot);
fuzzy->addFuzzyInput(temperature);


humidity->addFuzzySet(dry);
humidity->addFuzzySet(normal);
humidity->addFuzzySet(wet);
fuzzy->addFuzzyInput(humidity);

speed->addFuzzySet(slow);
speed->addFuzzySet(moderate);
speed->addFuzzySet(fast);
fuzzy->addFuzzyOutput(speed);


  // rule 1
  FuzzyRuleAntecedent *temperatureColdAndHumidityWet = new FuzzyRuleAntecedent();
  temperatureColdAndHumidityWet->joinWithAND(cold, wet);
  FuzzyRuleConsequent *slowSpeed = new FuzzyRuleConsequent();
  slowSpeed->addOutput(slow);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, temperatureColdAndHumidityWet, slowSpeed);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // rule 2
  FuzzyRuleAntecedent *temperatureColdAndHumidityNormal = new FuzzyRuleAntecedent();
  temperatureColdAndHumidityNormal->joinWithAND(cold, normal);
  slowSpeed->addOutput(slow);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, temperatureColdAndHumidityNormal, slowSpeed);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // rule 3
  FuzzyRuleAntecedent *temperatureColdAndHumidityDry = new FuzzyRuleAntecedent();
  temperatureColdAndHumidityDry->joinWithAND(cold, dry);
  FuzzyRuleConsequent *moderateSpeed = new FuzzyRuleConsequent();
  moderateSpeed->addOutput(moderate);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, temperatureColdAndHumidityDry, moderateSpeed);
  fuzzy->addFuzzyRule(fuzzyRule3);

  // rule 4
  FuzzyRuleAntecedent *temperatureMediumAndHumidityWet = new FuzzyRuleAntecedent();
  temperatureMediumAndHumidityWet->joinWithAND(normal, wet);
  slowSpeed->addOutput(slow);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, temperatureMediumAndHumidityWet, slowSpeed);
  fuzzy->addFuzzyRule(fuzzyRule4);

  // rule 5
  FuzzyRuleAntecedent *temperatureMediumAndHumidityNormal = new FuzzyRuleAntecedent();
  temperatureMediumAndHumidityNormal->joinWithAND(medium, normal);
  moderateSpeed->addOutput(moderate);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, temperatureMediumAndHumidityNormal, moderateSpeed);
  fuzzy->addFuzzyRule(fuzzyRule5);

  // rule 6
  FuzzyRuleAntecedent *temperatureMediumAndHumidityDry = new FuzzyRuleAntecedent();
  temperatureMediumAndHumidityDry->joinWithAND(medium, dry);
  FuzzyRuleConsequent *fastSpeed = new FuzzyRuleConsequent();
  fastSpeed->addOutput(fast);
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, temperatureMediumAndHumidityDry, fastSpeed);
  fuzzy->addFuzzyRule(fuzzyRule6);

  // rule 7
  FuzzyRuleAntecedent *temperatureHotAndHumidityWet = new FuzzyRuleAntecedent();
  temperatureHotAndHumidityWet->joinWithAND(hot, wet);
  moderateSpeed->addOutput(moderate);
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, temperatureHotAndHumidityWet, moderateSpeed);
  fuzzy->addFuzzyRule(fuzzyRule7);

  // rule 8
  FuzzyRuleAntecedent *temperatureHotAndHumidityNormal = new FuzzyRuleAntecedent();
  temperatureHotAndHumidityNormal->joinWithAND(hot, normal);
  fastSpeed->addOutput(fast);
  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, temperatureHotAndHumidityNormal, fastSpeed);
  fuzzy->addFuzzyRule(fuzzyRule8);
  
  // rule 9
  FuzzyRuleAntecedent *temperatureHotAndHumidityDry = new FuzzyRuleAntecedent();
  temperatureHotAndHumidityDry->joinWithAND(hot, dry);
  fastSpeed->addOutput(fast);
  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, temperatureHotAndHumidityDry, fastSpeed);
  fuzzy->addFuzzyRule(fuzzyRule9);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to WiFi...");
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        delay(5000);
      }
      Serial.println("Connected to WiFi.");
    }

    float temperatureC = dht.readTemperature();
    float humidityValue = dht.readHumidity();

    Serial.print("Temperature (C): ");
    Serial.println(temperatureC);
    Serial.print("Humidity (%): ");
    Serial.println(humidityValue);

    if (!isnan(temperatureC) && !isnan(humidityValue)) {
      ThingSpeak.setField(1, temperatureC);
      ThingSpeak.setField(2, humidityValue);

      int updateStatus = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

      if (updateStatus == 200) {
        Serial.println("Channel update successful.");
      } else {
        Serial.println("Problem updating channel. HTTP error code " + String(updateStatus));
      }
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }


    Serial.print(cold->getPertinence());

    Serial.print(medium->getPertinence());

    Serial.print(hot->getPertinence());

    Serial.print(dry->getPertinence());

    Serial.print(normal->getPertinence());

    Serial.print(wet->getPertinence());

    fuzzy->setInput(1, temperatureC);
    fuzzy->setInput(2, humidityValue);
    fuzzy->fuzzify();

    int fanSpeed = fuzzy->defuzzify(1);

    Serial.print("Velocidad del ventilador: ");
    Serial.println(fanSpeed);





    lastTime = millis();
  }
}
