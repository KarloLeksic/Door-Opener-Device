/*
  Board used: NodeMCU (ESP32)

  Arduino settings:
  - Board: ESP32 Dev Module
  - IDE version: 1.8.13

  11 February 2023 by Karlo Leksic
*/

// Include needed libraries
#include <ArduinoJson.h>
#include <ESP32_Servo.h>
#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Include files with secrets and settings
#include "secrets.h"
#include "settings.h"

// Needed objects
WiFiClientSecure client;
Servo myservo;

// Bots - each user have your own bot
UniversalTelegramBot admin(ADMIN_TOKEN, client); // Me
UniversalTelegramBot bot2(BOT2_TOKEN, client);   // My neighbor
UniversalTelegramBot bot1(BOT1_TOKEN, client);   // My roommate

// All possible commands
String commands = "/openDoor: Open the door\n"
                  "/getKeyboard: Send only once if you don't have a keyboard";

// If you want to add additional command, you have to make an if statement in the below function.
// The string above is only for sending all possible commands if the unknown command was sent

// Handle a new message for the certain bot you pass in the function
void handleNewMessages(UniversalTelegramBot *bot, int numNewMessages)
{
    String answer;
    for (int i = 0; i < numNewMessages; i++)
    {
        // Store the message
        telegramMessage &msg = bot->messages[i];

        // If it was sent "/openDoor" message
        if (msg.text == "/openDoor")
        {
            // Send the response to the device from which the message is sent
            bot->sendMessage(bot->messages[i].chat_id, "Opening the door for 5 seconds", "Markdown");

            // Open the door
            openDoor();
        }
        // If it was sent "/openDoor" message
        else if (msg.text == "/getKeyboard")
        {
            // Make a custom keyboard (only 1 button for opening the door in this case)
            String keyboardJson = "[[\"/openDoor\"]]";

            // Send the message with reply keyboard. The keyboard will stay in the chat after that.
            bot->sendMessageWithReplyKeyboard(bot->messages[i].chat_id, "Here is the keyboard!", "", keyboardJson,
                                              true);
        }
        // If it was sent an unknown message
        else
        {
            // Send all possible commands stored in the commands string
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
    // Init serial communication
    Serial.begin(115200);

    // Init servo
    myservo.attach(SERVO_PIN);

    // Attempt to connect to Wifi network:
    Serial.println("Connecting to Wifi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);

    // Add root certificate for api.telegram.org
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    // Waiting for WiFi connecting
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
    // Check for each bot
    checkMessage(&admin);
    checkMessage(&bot2);
    checkMessage(&bot1);
}

// Check if there are new messages for the bot passed in the function
void checkMessage(UniversalTelegramBot *bot)
{
    int numNewMessages = bot->getUpdates(bot->last_message_received + 1);

    while (numNewMessages)
    {
        handleNewMessages(bot, numNewMessages);
        numNewMessages = bot->getUpdates(bot->last_message_received + 1);
    }
}
