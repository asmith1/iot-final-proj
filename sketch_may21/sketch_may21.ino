#include <Fishino.h>
#include <FishinoClient.h>
#include <FishinoDebug.h>
#include <FishinoServer.h>
#include <FishinoUdp.h>
#include <DHT.h>

//Connection parameters
#define RETRY_DELAY 2000
#define MAX_CONNECTION_ATTEMPTS 10

//Measurement and prediction parameters
#define MEASURE_PERIOD 3000
#define PREDICT_ORDER_T 5
#define PREDICT_ORDER_H 5

//Actuator parameters
#define TEMP_MAX 30
#define TEMP_MIN 27
#define HUM_MAX 80
#define HUM_MIN 70
#define DHTPIN 2
#define HEATING 1
#define COOLING 3
#define SPRINKLERS 4
#define DRYER 5
#define SYSTEM_LOST_ALARM 6
#define MAX_MEASURE_ATTEMPTS 5

//Prediction variables
float tempHist [PREDICT_ORDER_T];
float humHist [PREDICT_ORDER_H];
bool tempMeasuredOK;
bool humMeasuredOK;
int failCount = 0;


FishinoClient client;
IPAddress server(192, 168, 43, 237);

DHT dht(DHTPIN, DHT11);


// Setup the wireless connection
boolean connectWiFi() {
  char WLAN_SSID[]="MyAsus";
  char WLAN_PASSWD[]="rickandmorty";
  int nattempt=0;
 
   while(!Fishino.reset()) {}
 
  // Set the node as WiFi Client
  Fishino.setMode(STATION_MODE);
  Fishino.begin(WLAN_SSID, WLAN_PASSWD);

  Serial.println(F("Fishino IP Connection attempt ..."));
  // Start the DHCP client service
  Fishino.staStartDHCP();

  nattempt=0;
  // Wait until an IP address is provided
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


//------------------------------------------------
//--------------------SETUP-----------------------
//------------------------------------------------

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (connectWiFi())
    Serial.println(F("Fishino IP Connected ... "));
  else 
    Serial.println(F("Fishino IP NOT Connected ... "));

    dht.begin();
    
}

///-----------------------------------------------
//--------------------LOOP------------------------
//------------------------------------------------


void loop() {
  

  //---------------------Measure data-------------------------------
  float temp = dht.readTemperature(); // reads temperature in Celsius
  float hum = dht.readHumidity(); // reads humidity in percentage

  // check to see if reading failed
  if (isnan(temp)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    tempMeasuredOK = false;
  }
  else {
    tempMeasuredOK = true;
  }
  if (isnan(hum)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    humMeasuredOK = false;
  }
  else {
    humMeasuredOK = true;
  }

  //Check system functioning
  if (tempMeasuredOK && humMeasuredOK){
    failCount = 0;
    digitalWrite(SYSTEM_LOST_ALARM, LOW);
  }
  else{failCount++;}

  //If system is disconnected, stop all actuators and contact user
  if (failCount < MAX_MEASURE_ATTEMPTS){
    
    //Estimate data if necessary  
    if (!tempMeasuredOK){
      int avg=0;
      for(int i = 0; i<PREDICT_ORDER_T; i++){    //In this case by a moving average
        avg = avg + tempHist[i];
      }
      temp=avg/PREDICT_ORDER_T;
    }
    if (!humMeasuredOK){
      int avg=0;
      for(int i = 0; i<PREDICT_ORDER_H; i++){     //In this case by a moving average
        avg = avg + humHist[i];
      }
      hum=avg/PREDICT_ORDER_H;
    }
  
    //Print measurements or predictions
    if (tempMeasuredOK) {
      Serial.print("Measured temperature:");
      Serial.print(temp);
      Serial.println(" *C");
    }
    else {
      Serial.print("Estimated temperature:");
      Serial.print(temp);
      Serial.println(" *C");
    }
    if (humMeasuredOK){
      Serial.print("Measured humidity: ");
      Serial.print(hum);
      Serial.println("%");
    }
    else {
      Serial.print("Estimated humidity: ");
      Serial.print(hum);
      Serial.println("%");
    }
    
    //Update temperature and humidity history.
    for(int aux=0; aux < (PREDICT_ORDER_T - 1); aux++){
      tempHist[aux] = tempHist[aux+1];
    }
    tempHist[PREDICT_ORDER_T - 1] = temp;
    for(int aux=0; aux < (PREDICT_ORDER_H - 1); aux++){
      humHist[aux] = humHist[aux+1];
    }
    humHist[PREDICT_ORDER_H - 1] = hum;
  
    //Print history to check
    Serial.println("History:");
    Serial.print("Temp: ");
    for(int aux=0;aux<PREDICT_ORDER_T;aux++){Serial.print(tempHist[aux]); Serial.print("  ");}
    Serial.println();
    Serial.print("Hum: ");
    for(int aux=0;aux<PREDICT_ORDER_H;aux++){Serial.print(humHist[aux]); Serial.print("  ");}
    Serial.println();
    Serial.println();
    Serial.println();
        
    
  //-------------- Control actuators according to measurements or estimations---------
  
    if(temp < TEMP_MIN){
      digitalWrite(HEATING, HIGH);
    }
    else{
      digitalWrite(HEATING, LOW);      
    }
    if(temp > TEMP_MAX){
      digitalWrite(COOLING, HIGH);
    }
    else{
      digitalWrite(COOLING, LOW);
    }
    if(hum < HUM_MIN){
      digitalWrite(SPRINKLERS, HIGH);
    }
    else{
      digitalWrite(SPRINKLERS, LOW);
    }
    if(hum > HUM_MAX){
      digitalWrite(DRYER, HIGH);
    }
    else{
      digitalWrite(DRYER, LOW);
    }

  //-------------- Send data to database ------------------
  
    // creates proper string to post
    String tempLine = "temp value=" + String((int)temp) + "i";
    String humLine = "hum value=" + String((int)hum) + "i";
  
    // connect to the influxdb port
    if (!client.connect(server, 8086)) 
      Serial.println("Did not connect to influxdb port");
    
    // Make an HTTP request:
    client.println("POST /write?db=mydb HTTP/1.1");
    client.println("Host: 192.168.43.237:8086");
    client.println("User-Agent: Arduino/1.6");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(tempLine.length());
    client.println();
    client.println(tempLine);

    delay(30);

    if (client.available())
    {
      char Response[600];
      client.readBytes(Response, client.available());
    }

    delay(30);
    // connect to the influxdb port
    if (!client.connect(server, 8086)) 
      Serial.println("Did not connect to influxdb port");

    client.println("POST /write?db=mydb HTTP/1.1");
    client.println("Host: 192.168.43.237:8086");
    client.println("User-Agent: Arduino/1.6");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(humLine.length());
    client.println();
    client.println(humLine);

    delay(30);
  
    if (client.available())
    {
      char Response[600];
      client.readBytes(Response, client.available());
    }
  
  }
  else { // we've reached maximum fail count
    Serial.println("System failure, all actuators have been turned off for safety reasons, please check sensors urgently");
    digitalWrite(SYSTEM_LOST_ALARM, HIGH);
    digitalWrite(HEATING, LOW);
    digitalWrite(COOLING, LOW);
    digitalWrite(SPRINKLERS, LOW);
    digitalWrite(DRYER, LOW);
    
    // send a boolean value of failure to another column
    String failureLine = "fail value=1i";
    // connect to the influxdb port
    if (!client.connect(server, 8086)) 
      Serial.println("Did not connect to influxdb port");
    
    // Make an HTTP request:
    client.println("POST /write?db=mydb HTTP/1.1");
    client.println("Host: 192.168.43.237:8086");
    client.println("User-Agent: Arduino/1.6");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(failureLine.length());
    client.println();
    client.println(failureLine);

    delay(30);

    if (client.available())
    {
      char Response[600];
      client.readBytes(Response, client.available());
      //Serial.println(Response);
    }
  }
  delay(MEASURE_PERIOD);

  // let the monitor know something is going on in case nothing is returned.
  Serial.println("loop...");
}

