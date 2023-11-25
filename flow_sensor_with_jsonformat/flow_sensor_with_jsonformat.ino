#define LED_BUILTIN 2
#define SENSOR 27
#define RELAY_PIN 16

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
bool flowSensorEnabled = false;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop()
{
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("{\"cmd\":\"")) {
      if (command.endsWith("\"flow?\"}")) {
        flowSensorEnabled = true;
        Serial.println("Flow sensor is now ON.");
      } else if (command.endsWith("\"flow_off\"}")) {
        flowSensorEnabled = false;
        pulseCount = 0;
        Serial.println("Flow sensor is now OFF.");
      } else if (command.endsWith("\"Relayon\"}")) {
        delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(RELAY_PIN, HIGH);
       // Serial.println("Relay/solenoid valve is now ON.");
        Serial.println("{\"Relay/solenoid valve\": \"ON\"}");
      } else if (command.endsWith("\"Relayoff\"}")) {
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(RELAY_PIN, LOW);
       // Serial.println("Relay/solenoid valve is now OFF.");
        Serial.println("{\"Relay/solenoid valve\": \"OFF\"}");
      } else {
        Serial.println("Unknown command.");
      }
    }
  }

  if (flowSensorEnabled) {
    currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
      pulse1Sec = pulseCount;
      pulseCount = 0;
      flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
      previousMillis = millis();
      flowMilliLitres = (flowRate / 60) * 1000;
      totalMilliLitres += flowMilliLitres;
      
      // Print flow rate with units in JSON format
      Serial.print("{\"Flow rate (L/min)\": ");
      Serial.print(int(flowRate));
      Serial.print(", \"outLiquidQuantity (mL)\": ");
      Serial.print(totalMilliLitres);
      Serial.println("}");
      flowSensorEnabled = false;
    }
  }
}

