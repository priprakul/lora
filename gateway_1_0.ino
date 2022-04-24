/*
  Heltec.LoRa Multiple Communication

  This example provide a simple way to achieve one to multiple devices
  communication.

  Each devices send datas in broadcast method. Make sure each devices
  working in the same BAND, then set the localAddress and destination
  as you want.
  
  Sends a message every half second, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFD as the broadcast address. You can set this address as you
  want.

  Note: while sending, Heltec.LoRa radio is not listening for incoming messages.
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/
#include "heltec.h"
//#include "string

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6


String outgoing;              // outgoing message

byte localAddress = 0xFF;     // address of this device
byte destination = 0xDD;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
int start_time;
int receive_flag = 0;
int cnt;
int check_ack = 0;
String message;
int time_sent;

void setup()
{
  destination = 0xDD;
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Enable*/, true /*Serial Enable*/, true /*PABOOST Enable(*/, BAND /*long BAND*/);
  Heltec.display -> clear();
  
  Serial.println("Heltec.LoRa Duplex");
  start_time = millis();
  Serial.println("The time in setup " +  (String)start_time);
  receive_flag = 0;
  cnt = 0;
  //LoRa.setSpreadingFactor(6);
  while(check_ack < 2 ){
    while(receive_flag == 0){
      if (millis() - lastSendTime > interval)
      {
        start_time = millis();
        Serial.println("inside_setup " );
        sendMessage("gate,",(String)start_time);
        //Serial.println("millis");
        //Serial.println((String)millis());
        lastSendTime = millis();            // timestamp the message
        interval = random(2000) + 1000;    // 2-3 seconds
      }
      //Serial.println((String)millis());
      onReceive(LoRa.parsePacket());
    }
    check_ack++;
    message = "ACK,";
    Serial.println("check_ack");
    Serial.println(check_ack);
    time_sent = millis();
    
    sendMessage(message,(String)time_sent);
    
    Serial.println(message);
    Serial.println((String)time_sent);
    receive_flag = 0;
    if(check_ack == 2)
      return;
  }
}

void loop()
{
  // logic to send ACK when a packet is received
  start_time = millis();
  if (receive_flag == 1)
  {
    if (millis() - lastSendTime > interval)
    {
        message = "Received,";
        int time_sent = millis();
        sendMessage(message,(String)time_sent);
        Serial.println(message);
        Serial.println((String)time_sent);

        lastSendTime = millis();            // timestamp the message
        interval = random(2000) + 1000;    // 2-3 seconds
        
        receive_flag = 0;
    }
  }
  
  onReceive(LoRa.parsePacket());
  
}

void sendMessage(String outgoing, String time_sent)
{
  //Serial.println("entering send");

  Serial.println("outgoing message");
  Serial.println(outgoing);
  Serial.println(time_sent);

  Serial.println("destination");
  Serial.println(destination);

  Serial.println("localAddress");
  Serial.println(localAddress);
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length() + time_sent.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.print(time_sent);
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize)
{
   // Serial.println("entering receive");
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:

  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length())
  {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  destination = sender;
  
  receive_flag = 1;
}
