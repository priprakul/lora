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

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6


String outgoing;              // outgoing message

byte localAddress = 0xBC;     // address of this device
byte destination = 0xFF;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
long offset = 0; 

int receive_flag = 0; 
int time_flag = 0; 
int gateway_time; 
int gateway_offset; 
String gateway_string_time; 
int absolute_time; 
int sent_time; 
int delta_time;  //time wait for ack
int state; 
int resend_flag =0; 
int resent = 0; 
int sent = 0; 
int received = 0; 
void setup()
{
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Enable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Heltec.display -> clear();
  LoRa.setSpreadingFactor(9);
  
  Serial.println("Heltec.LoRa Duplex");
  while (receive_flag  == 0){
    onReceive(LoRa.parsePacket()); 
  }
 
  String message; 
  message = "Received TIME";   // send a message
  delay(4000);
  int count = 0; 
  Serial.println("Sending " + message);
  while(count <3){
  sendMessage(message);
  count++;}

  lastSendTime = millis();            // timestamp the message
  receive_flag = 0; 
  while (receive_flag  == 0){
    onReceive(LoRa.parsePacket()); 
  }
  offset = millis()  - lastSendTime; 

/* last send time */
  Serial.println("Last send time");
  Serial.println(lastSendTime);

/* offset between the node - gateway - node */
  Serial.println("offset");
  Serial.println(offset); 

  Serial.println("Current Time"); 
  Serial.println(String(millis())); 
  Serial.println(offset); 
  
  delta_time = 1000; 
  // TODO: calculate/sync gateway clock 
  gateway_time = gateway_string_time.toInt();

/* gateway time received */
  Serial.println("Printed Received Gateway Time"); 
  Serial.println(gateway_time);

  

  state = 0;
  gateway_time = gateway_time + offset; 
  gateway_offset = millis() - gateway_time;

/* the gateway time calculated */
  Serial.println("Calculated Gateway Time"); 
  Serial.println(gateway_time);
  Serial.println("Gateway Offset"); 
  Serial.println(gateway_offset);
  
 int current_gateway_time = millis() - gateway_offset; 

  absolute_time = current_gateway_time+4500; 
  Serial.println("Estimated Gateway Time"); 
  Serial.println(current_gateway_time); 
  Serial.println("Absolute_time"); 
  Serial.println(absolute_time); 
  Serial.println("Gateway Offset"); 
  Serial.println(gateway_offset);
}

void loop()
{
  String message;

  //Serial.println(String(gateway_offset + millis()));

  if (millis() - gateway_offset > absolute_time){
      if (resend_flag == 1){     
      resent +=1; 
      message = "Resend Hello from B";
      lastSendTime = millis(); 

      Serial.println("Resend Message sent " + String(millis()-gateway_offset));
      sendMessage(message);
      absolute_time= absolute_time + random(1,5) * delta_time;
      resend_flag = 0;
      }
      else {
      //delay(150);
      Serial.println("Message sent " + String(millis()-gateway_offset));
      Serial.println("Absolute Time : " + String(absolute_time));
      message = "Hello from B";
      sendMessage(message);
      Serial.println(message);
      receive_flag = 0; 
      absolute_time= absolute_time + random(1,5) * delta_time;
      lastSendTime = millis();  
      state = 1;  
      sent += 1; 
      Serial.println("Resent:" + String(resent));  
      Serial.println("Sent:" + String(sent));
      Serial.println("Received:" + String(received));
      //delay(150);
      }
    }

   if ((receive_flag == 0) && ((millis() - lastSendTime)>delta_time)){
      resend_flag = 1; 
      /*message = "Resend Hello from B";
      lastSendTime = millis();  
      sendMessage(message);
      delay(4000); 
      resent +=1; 
      Serial.println("Resent:" + String(resent));  
      Serial.println("Resend Message sent " + String(millis()-gateway_offset));*/
      //delay(150); 
    }
   //delay(200);
  onReceive(LoRa.parsePacket()); 

}

void sendMessage(String outgoing)
{
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize)
{
  //Serial.println("entered");
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
  if ((sender == 0xBB) || (recipient == 0xBB)) {
    //Serial.println("This message is not for me.");
    //Serial.println("Received from: 0x" + String(sender, HEX));
    //Serial.println("Sent to: 0x" + String(recipient, HEX));
    return;                             // skip rest of function
  }
  Serial.println("GATEWAY TIME 2");
  gateway_string_time = getValue(incoming,',',1);

  Serial.println(gateway_string_time);

  
  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  receive_flag = 1;
  received +=1; 
  Serial.println("Received:" + String(received));  
}




String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
