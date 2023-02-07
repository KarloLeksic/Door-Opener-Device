/*
  Board used: WEMOS D1 mini (ESP8266)

  Arduino settings:
  - Board: LOLIN (WEMOS) D1 R2 & mini
  - IDE version: 1.8.13

  7 February 2023 by Karlo Leksic
*/

// Include needed libraries
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Servo.h>

/*-------------------------CHANGE HERE---------------------------*/

// Network credentials
const char *ssid = "";
const char *password = "";

// Bot Tokens (Get from Botfather)
#define ADMIN_TOKEN "" 
#define BOT2_TOKEN  ""

// Chat ID by the IDBot
#define ADMIN_ID ""

// Time between scan messages
const unsigned long BOT_MTBS = 1000;

// Pin on which the servo is connected
#define SERVO_PIN D3

// Time to hold door opening button in milliseconds
#define DOOR_OPENING_DURATION 5000

// Position of the servo motor when the button is pressed
#define SERVO_POSITION_BUTTON_PRESSED 25

// Position of the servo motor when the button is released
#define SERVO_POSITION_BUTTON_RELEASED 120

/*----------------------------------------------------------------*/

// Last time messages' scan has been done
unsigned long botLastTime;

// Objects needs to work properly
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
Servo myservo;

// Bots
UniversalTelegramBot admin(ADMIN_TOKEN, client); 
UniversalTelegramBot bot2(BOT2_TOKEN, client);

// All possible commands
String commands = "/openDoor: Open the door\n"
                  "/getKeyboard: Send only once if you don't have a keyboard";

// Handle a new message for the bot you pass in the function
void handleNewMessages(UniversalTelegramBot *bot, int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = bot->messages[i];
    Serial.println("Received " + msg.text);
    if (msg.text == "/openDoor")
    {
      bot->sendMessage(bot->messages[i].chat_id, "Opening the door", "Markdown");
      openDoor();
      bot->sendMessage(bot->messages[i].chat_id, "Door closed!", "Markdown");
    }
    else if (msg.text == "/getKeyboard")
    {
      String keyboardJson = "[[\"/openDoor\"]]";
      bot->sendMessageWithReplyKeyboard(bot->messages[i].chat_id, "Here is the keyboard!", "", keyboardJson, true);
    }
    else
    {
      answer = "Invalid command!\n\nTry one of these:\n";
      answer += commands;
      bot->sendMessage(bot->messages[i].chat_id, answer, "Markdown");
    }
  }
}

// Press the door opening button with the servo motor and hold DOOR_OPENING_DURATION milliseconds
void openDoor()
{
  myservo.write(SERVO_POSITION_BUTTON_PRESSED); // Press the button
  delay(DOOR_OPENING_DURATION);
  myservo.write(SERVO_POSITION_BUTTON_RELEASED); // Release the button
}

void setup()
{
  // Begin serial communication
  Serial.begin(115200);

  // Init servo
  myservo.attach(SERVO_PIN);

  // Attempt to connect to Wifi network:
  Serial.println("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  // Print IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Add root certificate for api.telegram.org
  client.setTrustAnchors(&cert);

  // Get UTC time via NTP
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  // Send welcome message and set the reply keyboard to the admin
  String keyboardJson = "[[\"/openDoor\"]]";
  admin.sendMessageWithReplyKeyboard(ADMIN_ID, "Bot started up", "", keyboardJson, true);
}

void loop()
{
  // Every BOT_MTBS milliseconds check if there is any incoming message
  if ((unsigned long)millis() - botLastTime > BOT_MTBS)
  {
    // Check for each bot
    checkMessage(&admin);
    checkMessage(&bot2);

    // Remember the time when the bot last checked the messages
    botLastTime = millis();
  }
}

// Check if there are new messages for the bot passed in the function
void checkMessage(UniversalTelegramBot *bot)
{
  int numNewMessages = bot->getUpdates(bot->last_message_received + 1);

  while (numNewMessages)
  {
    Serial.println("got response");
    handleNewMessages(bot, numNewMessages);
    numNewMessages = bot->getUpdates(bot->last_message_received + 1);
  }
}
