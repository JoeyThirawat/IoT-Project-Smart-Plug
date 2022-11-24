#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

#define SW_1 12
#define LED_1 14
//
#define SW_2 0
#define LED_2 10
//
#define RELAY_1 15
#define RELAY_2 13


const char* ssid = "joeyrai";
const char* password = "xxxxxxxxxx";
const char* mqtt_server = "xxx.xxx.xxx.xxx";

long now = millis();
long lastMeasure = 0;

static int sw1 = 0, sw2 = 0;
float v1, i1, p1, e1, f1, pf1;
float v2, i2, p2, e2, f2, pf2;
double elecCharge_1 = 0, elecCharge_2 = 0, elecCharge_All = 0, elecCharge_1_note = 0, elecCharge_2_note = 0;

static char sw1_sc[7], sw2_sc[7];
static char v1_sc[7], i1_sc[7], p1_sc[7], e1_sc[7], f1_sc[7], pf1_sc[7];
static char v2_sc[7], i2_sc[7], p2_sc[7], e2_sc[7], f2_sc[7], pf2_sc[7];
static char elecCharge_1_sc[13], elecCharge_2_sc[13], elecCharge_All_sc[13];

WiFiClient espClient;
PubSubClient client(espClient);

PZEM004Tv30 pzem_1(5, 4);
PZEM004Tv30 pzem_2(2, 16);



// Sets mqtt broker and sets the callback function
// The callback function is what receives messages
void setup() {
  pinMode(SW_1, INPUT);
  pinMode(SW_2, INPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);

  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  //---------------------------------Callback setup
}



void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop()) {
    client.connect("ESP8266Client");
  }

  SW_and_Relay();
  //sw1 = 0;
  //sw2 = 0;

  // pzem_1
  v1 = pzem_1.voltage();
  i1 = pzem_1.current();
  p1 = pzem_1.power();   // use power to calculate electric units usage.
  e1 = pzem_1.energy();
  f1 = pzem_1.frequency();
  pf1 = pzem_1.pf();
  /*v1 = 10.11;
    i1 = 21.22;
    p1 = 35.33;
    e1 = 40.44;
    f1 = 59.55;
    pf1 = 60.66;*/

  // pzem_2
  v2 = pzem_2.voltage();
  i2 = pzem_2.current();
  p2 = pzem_2.power();
  e2 = pzem_2.energy();
  f2 = pzem_2.frequency();
  pf2 = pzem_2.pf();
  /*v2 = 12.21;
    i2 = 22.22;
    p2 = 32.23;
    e2 = 42.24;
    f2 = 52.25;
    pf2 = 62.26;*/

  //elecCharge_1 = 11.22;
  //elecCharge_2 = 34.6;
  //elecCharge_All = 55.58;

  now = millis();

  if (now - lastMeasure > 1000)
  {
    lastMeasure = now;

    isnanCheck();

    if ((sw1 == 1) && (sw2 == 0))
    {
      elecCharge_1_note = p1 / 3600000;
      elecCharge_1 = elecCharge_1 + elecCharge_1_note;
    }
    else if ((sw1 == 0) && (sw2 == 1))
    {
      elecCharge_2_note = p2 / 3600000;
      elecCharge_2 = elecCharge_2 + elecCharge_2_note;
    }
    else if ((sw1 == 1) && (sw2 == 1))
    {
      elecCharge_1_note = p1 / 3600000;
      elecCharge_1 = elecCharge_1 + elecCharge_1_note;

      elecCharge_2_note = p2 / 3600000;
      elecCharge_2 = elecCharge_2 + elecCharge_2_note;
    }

    elecCharge_All = elecCharge_1 + elecCharge_2;

    computesFloat2StaticChar_and_publish();

    Serial.print("sw1:"); Serial.print(sw1); Serial.print("  ");
    Serial.print("sw2:"); Serial.print(sw2); Serial.print(" |  ");

    Serial.print("v1:"); Serial.print(v1); Serial.print("  ");
    Serial.print("i1:"); Serial.print(i1); Serial.print("  ");
    Serial.print("p1:"); Serial.print(p1); Serial.print("  ");
    Serial.print("e1:"); Serial.print(e1); Serial.print("  ");
    Serial.print("f1:"); Serial.print(f1); Serial.print("  ");
    Serial.print("pf1:"); Serial.print(pf1); Serial.print(" |  ");


    Serial.print("v2:"); Serial.print(v2); Serial.print("  ");
    Serial.print("i2:"); Serial.print(i2); Serial.print("  ");
    Serial.print("p2:"); Serial.print(p2); Serial.print("  ");
    Serial.print("e2:"); Serial.print(e2); Serial.print("  ");
    Serial.print("f2:"); Serial.print(f2); Serial.print("  ");
    Serial.print("pf2:"); Serial.print(pf2); Serial.print(" |  ");

    Serial.print("eC_1:"); Serial.print(elecCharge_1, 8); Serial.print("  ");
    Serial.print("eC_2:"); Serial.print(elecCharge_2, 8); Serial.print("  ");
    Serial.print("eC_All:"); Serial.print(elecCharge_All, 8); Serial.print("\n");

    //else
    //{
    // Serial.print("Error reading v1 or i1\n");
    //}
  }
}

void SW_and_Relay()
{
  if (digitalRead(SW_1) == 0)
  {
    delay(70);
    if (sw1 == 0)
    {
      while (digitalRead(SW_1) == 0)
      {
        digitalWrite(LED_1, LOW);
        digitalWrite(RELAY_1, HIGH);
        sw1 = 1;
      }
    }
    else
    {
      while (digitalRead(SW_1) == 0)
      {
        digitalWrite(LED_1, HIGH);
        digitalWrite(RELAY_1, LOW);
        sw1 = 0;
      }
    }
  }

  if (digitalRead(SW_2) == 0)
  {
    delay(70);
    if (sw2 == 0)
    {
      while (digitalRead(SW_2) == 0)
      {
        digitalWrite(LED_2, LOW);
        digitalWrite(RELAY_2, HIGH);
        sw2 = 1;
      }
    }
    else
    {
      while (digitalRead(SW_2) == 0)
      {
        digitalWrite(LED_2, HIGH);
        digitalWrite(RELAY_2, LOW);
        sw2 = 0;
      }
    }
  }
  //Serial.print("SW_1 = ");
  //Serial.print(led_status_1);
  //Serial.print("\tSW_2 = ");
  //Serial.print(led_status_2);
  //Serial.print("\n");
}

void isnanCheck()
{
  if (isnan(v1)) {
    v1 = 0;
  }
  if (isnan(i1)) {
    i1 = 0;
  }
  if (isnan(p1)) {
    p1 = 0;
  }
  if (isnan(e1)) {
    e1 = 0;
  }
  if (isnan(f1)) {
    f1 = 0;
  }
  if (isnan(pf1)) {
    pf1 = 0;
  }

  if (isnan(v2)) {
    v2 = 0;
  }
  if (isnan(i2)) {
    i2 = 0;
  }
  if (isnan(p2)) {
    p2 = 0;
  }
  if (isnan(e2)) {
    e2 = 0;
  }
  if (isnan(f2)) {
    f2 = 0;
  }
  if (isnan(pf2)) {
    pf2 = 0;
  }

}

void computesFloat2StaticChar_and_publish()
{
  dtostrf(sw1, 6, 2, sw1_sc);
  dtostrf(sw2, 6, 2, sw2_sc);

  dtostrf(v1, 6, 2, v1_sc);
  dtostrf(i1, 6, 2, i1_sc);
  dtostrf(p1, 6, 2, p1_sc);
  dtostrf(e1, 6, 2, e1_sc);
  dtostrf(f1, 6, 2, f1_sc);
  dtostrf(pf1, 6, 2, pf1_sc);

  dtostrf(v2, 6, 2, v2_sc);
  dtostrf(i2, 6, 2, i2_sc);
  dtostrf(p2, 6, 2, p2_sc);
  dtostrf(e2, 6, 2, e2_sc);
  dtostrf(f2, 6, 2, f2_sc);
  dtostrf(pf2, 6, 2, pf2_sc);

  dtostrf(elecCharge_1, 12, 8, elecCharge_1_sc);
  dtostrf(elecCharge_2, 12, 8, elecCharge_2_sc);
  dtostrf(elecCharge_All, 12, 8, elecCharge_All_sc);

  // Publishes values ----------------------------------------------------------------
  client.publish("smart_plug/sw1", sw1_sc);
  client.publish("smart_plug/sw2", sw2_sc);

  client.publish("smart_plug/v1", v1_sc);
  client.publish("smart_plug/i1", i1_sc);
  client.publish("smart_plug/p1", p1_sc);
  client.publish("smart_plug/e1", e1_sc);
  client.publish("smart_plug/f1", f1_sc);
  client.publish("smart_plug/pf1", pf1_sc);

  client.publish("smart_plug/v2", v2_sc);
  client.publish("smart_plug/i2", i2_sc);
  client.publish("smart_plug/p2", p2_sc);
  client.publish("smart_plug/e2", e2_sc);
  client.publish("smart_plug/f2", f2_sc);
  client.publish("smart_plug/pf2", pf2_sc);

  client.publish("smart_plug/elecCharge_1", elecCharge_1_sc);
  client.publish("smart_plug/elecCharge_2", elecCharge_2_sc);
  client.publish("smart_plug/elecCharge_All", elecCharge_All_sc);
}

// This functions is executed when some device publishes a message to a topic that ESP8266 is subscribed to
void callback(String topic, byte* message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageIn;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageIn += (char)message[i];
  }
  Serial.println();

  if (topic == "smart_plug/sw1_sub") {
    Serial.print("Changing sw1 to =>  ");
    if (messageIn == "sw1_on") {
      sw1 = 1;
      digitalWrite(LED_1, LOW);
      digitalWrite(RELAY_1, HIGH);
      Serial.print("sw1 = 1");
    }
    else if (messageIn == "sw1_off") {
      sw1 = 0;
      digitalWrite(LED_1, HIGH);
      digitalWrite(RELAY_1, LOW);
      Serial.print("sw1 = 0");
    }
  }
  else if (topic == "smart_plug/sw2_sub") {
    Serial.print("Changing sw2 to =>  ");
    if (messageIn == "sw2_on") {
      sw2 = 1;
      digitalWrite(LED_2, LOW);
      digitalWrite(RELAY_2, HIGH);
      Serial.print("sw2 = 1");
    }
    else if (messageIn == "sw2_off") {
      sw2 = 0;
      digitalWrite(LED_2, HIGH);
      digitalWrite(RELAY_2, LOW);
      Serial.print("sw2 = 0");
    }
  }
  Serial.println();
  //return(sw1);
}


// Connects ESP8266 to router
void setup_wifi()
{
  delay(10);
  // Start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// Reconnects ESP8266 to MQTT broker
// Change the function below to subscribe to more topics with ESP8266
void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      client.subscribe("smart_plug/sw1_sub");
      client.subscribe("smart_plug/sw2_sub");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
