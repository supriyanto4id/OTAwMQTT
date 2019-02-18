#include <TaskScheduler.h> //library multitask
#include "DHT.h" //library dht
#include <Adafruit_Sensor.h> //library tamnbahan untuk DHT.h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>



// inisialisasi PIN sensor data DHT 11 to ESP8266 D7
#define DHTPIN D7
//inisialisasi type DHT can use DHT 22, DHT 21, dan DHT 11 
#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);

//membuat koneksi ke WIFI dengan mengisis ssid dan password
const char* ssid ="myapp";
const char* pass ="amanin123";

//isi dengan server mqtt
IPAddress server_ip(35,240,246,168);

//
WiFiClient wclient;
PubSubClient client(wclient, server_ip);

//
void t1Callback();
void t2Callback();
void t3Callback();

//Task
 Task t1(100, TASK_FOREVER, &t1Callback);
 Task t2(15000, TASK_FOREVER, &t2Callback);

 Scheduler runner;


void receive_ota(const MQTT::Publish& pub){
      uint32_t startTime = millis();
      uint32_t size = pub.payload_len();
      if(size == 0)
      return;

      Serial.print("Receiving OTA of");
      Serial.print(size);
      Serial.print("byte....");

       Serial.setDebugOutput(true);
      if (ESP.updateSketch(*pub.payload_stream(), size, true, false)) {
            Serial.println("Clearing retained message.");
            client.publish(MQTT::Publish(pub.topic(), "")
                              .set_retain());
            client.disconnect();

            Serial.printf("Update Success: %u\nRebooting...\n", millis() - startTime);
            ESP.restart();
            delay(10000);
      }
       Update.printError(Serial);
       Serial.setDebugOutput(false);
}

 void t1Callback(){
       Serial.println("task 1 OTA subscribe Firmware");
         if (WiFi.status() == WL_CONNECTED) {
            if (!client.connected()) {
                  // Give ourselves a unique client name
                  if (client.connect(WiFi.macAddress())) {
                        client.set_callback(receive_ota);   // Register our callback for receiving OTA's
                        IPAddress local = WiFi.localIP();
                        String ipaddr = String(local[0]) + "." + String(local[1]) + "." + String(local[2]) + "." + String(local[3]);
                        String topic = "ota/" + ipaddr;
                        Serial.print("Subscribing to topic ");
                        Serial.println(topic);
                        client.subscribe(topic);
           
                  }
             }

      if (client.connected())
      client.loop();
       }
 }

 void t2Callback(){
       Serial.println("task 2 sensing DHT 11 dan publish to Firmware");

        delay(2000);
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      if (isnan(h) || isnan(t)){
            Serial.println(F("Failed to read from DHT"));
            return;
      }

      Serial.print("Humadity:");
      Serial.print(h);
      Serial.print(" % ");
      Serial.print(" Temperature : ");
      Serial.print(t);
      Serial.print(F(" *C "));
      
	client.connect("arduinoClient");  
	client.publish("sensor/temperatur",String(t).c_str()); 
	client.publish("sensor/humidity",String(h).c_str());  
      Serial.println("send to broker");
 }

void setup(){
      Serial.begin(9600);
      Serial.println("Arduino MQTT OTA with dht11 v2 ");
      //initialized Scheduler
      runner.init();

      runner.addTask(t1);

      runner.addTask(t2);

      delay(5000);

      t1.enable();
      t2.enable();
      //memangil dht 
      dht.begin();

      Serial.println(__TIMESTAMP__);

      Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
      Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());
}


void loop(){

     // runner.execute();
      if (WiFi.status() != WL_CONNECTED) {
            Serial.print("Connecting to ");
            Serial.print(ssid);
            Serial.println("...");
            WiFi.begin(ssid, pass);
            if (WiFi.waitForConnectResult() != WL_CONNECTED)
                  return;
                  Serial.print("IP address: ");
                  Serial.println(WiFi.localIP());
      }

      // if (WiFi.status() == WL_CONNECTED){
            runner.execute();
      // }
      

         
}