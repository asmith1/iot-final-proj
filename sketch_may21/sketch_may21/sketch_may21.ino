#include <Fishino.h>
#include <FishinoClient.h>
#include <FishinoDebug.h>
#include <FishinoServer.h>
#include <FishinoUdp.h>
#include <DHT.h>

#define RETRY_DELAY 2000
#define MAX_CONNECTION_ATTEMPTS 10
#define DHTPIN 2

FishinoClient client;
//IPAddress server(192, 168, 1, 1); // BBB
//IPAddress server(5, 168, 128, 141);
IPAddress server(172, 20, 10, 2);
//IPAddress server2(127, 0, 0, 1);

DHT dht(DHTPIN, DHT11);


// Setup the wireless connection
boolean connectWiFi() {
  char WLAN_SSID[]="AshleyiPhone";
  char WLAN_PASSWD[]="123ashleypass";
  int nattempt=0;
 
   while(!Fishino.reset()) {}
 
  // Set the node as WiFi Client
  Fishino.setMode(STATION_MODE);
  Fishino.begin(WLAN_SSID, WLAN_PASSWD);

  Serial.println(F("Fishino IP Connection attempt ..."));
  // Start the DHCP client service
  Fishino.staStartDHCP();

  nattempt=0;
  // Wait till an IP address is provided
  while((Fishino.status() != STATION_GOT_IP)  && (nattempt < MAX_CONNECTION_ATTEMPTS)) {
    nattempt++;
    Serial.println(F("Fishino IP ACQUISITION FAILED, RETRYING ... "));
    delay(RETRY_DELAY);
  }


  if (nattempt == MAX_CONNECTION_ATTEMPTS) 
     return false;
  
  else {
     IPAddress ip = Fishino.localIP();
     Serial.print(F("My IP is ..."));
     Serial.println(ip);
  }
  
  return true;
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (connectWiFi())
    Serial.println(F("Fishino IP Connected ... "));
  else 
    Serial.println(F("Fishino IP NOT Connected ... "));

    dht.begin();
}


void loop() {
  // connect to the influxdb port
  if (!client.connect(server, 8086)) 
    Serial.println("Did not connect to influxdb port");

  float temp = dht.readTemperature(); // reads temperature in Celsius
  float hum = dht.readHumidity(); // reads humidity in percentage

  // check to see if reading failed
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else {
    // test to make sure we read these values correctly
    Serial.print("Temperature:");
    Serial.print(temp);
    Serial.println(" *C");
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.println("%");
  
  
    // creates proper string to post
    String tempLine = "temperature value=" + String(temp);
    String humLine = "humidity value=" + String(hum);
    String combinedLine = tempLine + ", " + humLine;
  
    // print the lines that will be posted to the screen
    Serial.println(tempLine);
    Serial.println(humLine);
  
  
    // Make an HTTP request:
    client.println("POST /write?db=mydb HTTP/1.1");
    client.println("Host: 172.20.10.2:8086");
    client.println("User-Agent: Arduino/1.6");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(tempLine.length() + humLine.length());
    client.println();
    client.println(tempLine);
//    client.println(tempLine.length() + humLine.length());
//    client.println();
//    client.println(combinedLine);
  }
  
  delay(30);

  if (client.available())
  {
    char Response[200];
    client.readBytes(Response, client.available());
    Serial.println(Response);
  }


  delay(10000);

  // let the monitor know something is going on in case nothing is returned.
  Serial.println("loop...");
}
