// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 1. Azure SDK for C --> Handle Azure IoT Hub client Initial
 2. DallasTemperature --> Handle DS18B20 water temperature sensor
 3. ArduinoJson --> Handle MQTT response with JSON format
 4. ESP8266Wifi --> Handle Wifi connections
 5. PubSubClient --> Handle MQTT client and telemetry
 */


//Fan + WaterTemp
#include <OneWire.h> 
#include <DallasTemperature.h> 
#define Temperature D4 
#define FanControl_R D2
#define FanControl_L D3
OneWire oneWire(Temperature);
DallasTemperature sensors(&oneWire);

// C99 libraries
#include <cstdlib>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Libraries for MQTT client, WiFi connection and SAS-token generation.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <bearssl/bearssl.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>

// Azure IoT SDK for C includes
#include <az_core.h>
#include <az_iot.h>
#include <azure_ca.h>

// Additional sample headers
#include "iot_configs.h"
#include <ArduinoJson.h>

// When developing for your own Arduino-based platform,
// please follow the format '(ard;<platform>)'.
#define AZURE_SDK_CLIENT_USER_AGENT "c%2F" AZ_SDK_VERSION_STRING "(ard;esp8266)"

// Utility macros and defines
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_PACKET_SIZE 1024


// Translate iot_configs.h defines into variables used by the sample
static const char* ssid = IOT_CONFIG_WIFI_SSID;
static const char* password = IOT_CONFIG_WIFI_PASSWORD;
static const char* host = IOT_CONFIG_IOTHUB_FQDN;
static const char* device_id = IOT_CONFIG_DEVICE_ID;
static const char* device_key = IOT_CONFIG_DEVICE_KEY;
static const int port = 8883;

// Memory allocated for the sample's variables and structures.
static WiFiClientSecure wifi_client;
static X509List cert((const char*)ca_pem);
static PubSubClient mqtt_client(wifi_client);
static az_iot_hub_client client;
static char sas_token[200];
static uint8_t signature[512];
static unsigned char encrypted_signature[32];
static char base64_decoded_device_key[32];
static unsigned long next_telemetry_send_time_ms = 0;
static char telemetry_topic[128];
static uint8_t telemetry_payload[100];
static uint32_t telemetry_send_count = 0;



//全域變數
char json_output[100];
// char telemetry_json_output[100];
// char direct_method_json_output[100];
int pwm_R=0 ,  pwm_L=0;
float water_temperature;
bool isAutoMode = true;
// Arduino JsonDocument全域宣告
StaticJsonDocument<200> telemetry_doc;
DeserializationError json_error;



int freeRam () 
{
 return ESP.getFreeHeap();
}

static void connectToWiFi()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to WIFI SSID ");
  

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting to WIFI....");
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("password: ");
    Serial.println(password);
    delay(500);
    Serial.print(".");
  }

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

static void initializeTime()
{
  Serial.print("Setting time using SNTP");

  configTime(-5 * 3600, 0, NTP_SERVERS);
  time_t now = time(NULL);
  while (now < 1510592825)
  {
    delay(500);
    Serial.print(".");
    now = time(NULL);
  }
  Serial.println("done!");
}

static char* getCurrentLocalTimeString()
{
  time_t now = time(NULL)+ 60 * 60 * 13;
  return ctime(&now);
}

static void printCurrentTime()
{
  Serial.print("Current time: ");
  Serial.print(getCurrentLocalTimeString());
}


void receivedDirectMethod(char* topic, byte* payload , unsigned int length)
{
  Serial.print("topic:");
  Serial.println(topic);
  
  char* method = filter_methodName(topic);
  if(method != NULL){
    handle_direct_method(method, topic, payload, length);
  }
}

char* filter_methodName(char* topic){
    // Find the position of "/POST/"
    char* start = strstr(topic, "/POST/");
    if (start != NULL) {
        start += strlen("/POST/");
        char* end = strchr(start, '/');
        if (end != NULL) {
            size_t length = end - start;
            char* method = (char*)malloc((length + 1) * sizeof(char));
            strncpy(method, start, length);
            method[length] = '\0';
            return method;
        }
    }
    return NULL;
}

void handle_direct_method(char* method, char* topic, byte* payload , unsigned int length){

  StaticJsonDocument<100> directmethod_doc;
  // directmethod_doc = new_directmethod_doc;
  if (strcmp(method,"ChangeFan") == 0){
    Serial.println("Recieved Direct Method:");
    Serial.println(method);
    char fanspeed[20];
    for (int i = 0; i < length; i++)
    {
      fanspeed[i] = (char)payload[i];
    }
    Serial.print("Before:");
    Serial.print((pwm_R+pwm_L) / 2);
    Serial.print("After:");
    Serial.println(atoi(fanspeed));

    pwm_R = atoi(fanspeed);
    pwm_L = atoi(fanspeed);

    if(atoi(fanspeed) >=0) {
      analogWrite(FanControl_R, atoi(fanspeed));
      analogWrite(FanControl_L, atoi(fanspeed));
      delay(100);
      directmethod_doc["status"] = "OK";
      directmethod_doc["new_fanspeed"] = atoi(fanspeed);
    }
    else{
      directmethod_doc["status"] = "FAIL";
    }
  }
  else if (strcmp(method,"ChangeMode")== 0){
    // 0=ManualMode 1=AutoMode
    Serial.print("Recieved Direct Method:");
    Serial.println(method);


    char mode[20];
    for (int i = 0; i < length; i++)
    {
      mode[i] = (char)payload[i];
    }
    Serial.print("payload:");
    Serial.println((int)mode[0]);
    //0 to ASCII = 48
    if (mode[0]== 0|| mode[0] == '0'){
      isAutoMode = false;
      directmethod_doc["status"] = "OK";
    }
    else if (mode[0]== 1|| mode[0] == '1'){
      isAutoMode = true;
      directmethod_doc["status"] = "OK";
    }
    else{
      directmethod_doc["status"] = "FAIL";
      directmethod_doc["error"] = "Unexpected payload";
    }
  }
  else{
    directmethod_doc["status"] = "FAIL";
    directmethod_doc["error"] = "Unknown method";
  }
  Serial.println(freeRam());
  reply_direct_method(directmethod_doc, topic);
}

void reply_direct_method(const JsonDocument& directmethod_doc, char* topic){
  size_t n = serializeJson(directmethod_doc, json_output);
  char response[40] = "$iothub/methods/res/200/?$rid=";
  char topic_array[40];
  strcpy(topic_array, topic);
  int rid_index;
  //把topic =號後面的rid貼到response
  for(int i = 0; i < strlen(topic_array) ; i++){
      if(topic_array[i] == '='){
          rid_index = i;
          break;
      }
  }
  for(int i = rid_index + 1 ; i < strlen(topic_array); i++){
      int len = strlen(response);
      response[len++] = topic_array[i];
  }
  Serial.print("reponse:");
  Serial.println(response);
  mqtt_client.publish(response, json_output, n);
  delay(50);
  sendTelemetry((pwm_R +pwm_L)/2);
}

static void initializeClients()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

  wifi_client.setTrustAnchors(&cert);
  if (az_result_failed(az_iot_hub_client_init(
          &client,
          az_span_create((uint8_t*)host, strlen(host)),
          az_span_create((uint8_t*)device_id, strlen(device_id)),
          &options)))
  {
    Serial.println("Failed initializing Azure IoT Hub client");
    return;
  }

  mqtt_client.setServer(host, port);
  mqtt_client.setCallback(receivedDirectMethod);
}

/*
 * @brief           Gets the number of seconds since UNIX epoch until now.
 * @return uint32_t Number of seconds.
 */
static uint32_t getSecondsSinceEpoch() { return (uint32_t)time(NULL); }

static int generateSasToken(char* sas_token, size_t size)
{
  az_span signature_span = az_span_create((uint8_t*)signature, sizeofarray(signature));
  az_span out_signature_span;
  az_span encrypted_signature_span
      = az_span_create((uint8_t*)encrypted_signature, sizeofarray(encrypted_signature));

  uint32_t expiration = getSecondsSinceEpoch() + ONE_HOUR_IN_SECS;

  // Get signature
  if (az_result_failed(az_iot_hub_client_sas_get_signature(
          &client, expiration, signature_span, &out_signature_span)))
  {
    Serial.println("Failed getting SAS signature");
    return 1;
  }

  // Base64-decode device key
  int base64_decoded_device_key_length
      = base64_decode_chars(device_key, strlen(device_key), base64_decoded_device_key);

  if (base64_decoded_device_key_length == 0)
  {
    Serial.println("Failed base64 decoding device key");
    return 1;
  }

  // SHA-256 encrypt
  br_hmac_key_context kc;
  br_hmac_key_init(
      &kc, &br_sha256_vtable, base64_decoded_device_key, base64_decoded_device_key_length);

  br_hmac_context hmac_ctx;
  br_hmac_init(&hmac_ctx, &kc, 32);
  br_hmac_update(&hmac_ctx, az_span_ptr(out_signature_span), az_span_size(out_signature_span));
  br_hmac_out(&hmac_ctx, encrypted_signature);

  // Base64 encode encrypted signature
  String b64enc_hmacsha256_signature = base64::encode(encrypted_signature, br_hmac_size(&hmac_ctx));

  az_span b64enc_hmacsha256_signature_span = az_span_create(
      (uint8_t*)b64enc_hmacsha256_signature.c_str(), b64enc_hmacsha256_signature.length());

  // URl-encode base64 encoded encrypted signature
  if (az_result_failed(az_iot_hub_client_sas_get_password(
          &client,
          expiration,
          b64enc_hmacsha256_signature_span,
          AZ_SPAN_EMPTY,
          sas_token,
          size,
          NULL)))
  {
    Serial.println("Failed getting SAS token");
    return 1;
  }

  return 0;
}

static int connectToAzureIoTHub()
{
  size_t client_id_length;
  char mqtt_client_id[128];
  if (az_result_failed(az_iot_hub_client_get_client_id(
          &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    Serial.println("Failed getting client id");
    return 1;
  }

  mqtt_client_id[client_id_length] = '\0';

  char mqtt_username[128];
  // Get the MQTT user name used to connect to IoT Hub
  if (az_result_failed(az_iot_hub_client_get_user_name(
          &client, mqtt_username, sizeofarray(mqtt_username), NULL)))
  {
    printf("Failed to get MQTT clientId, return code\n");
    return 1;
  }

  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  Serial.print("Username: ");
  Serial.println(mqtt_username);

  mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

  while (!mqtt_client.connected())
  {
    time_t now = time(NULL);

    Serial.print("MQTT connecting ... ");

    if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
    {
      Serial.println("connected.");
    }
    else
    {
      Serial.print("failed, status code =");
      Serial.print(mqtt_client.state());
      Serial.println(". Trying again in 5 seconds.");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);
  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC);


  return 0;
}

static void establishConnection()
{
  connectToWiFi();
  initializeTime();
  printCurrentTime();
  initializeClients();

  // The SAS token is valid for 1 hour by default in this sample.
  // After one hour the sample must be restarted, or the client won't be able
  // to connect/stay connected to the Azure IoT Hub.
  if (generateSasToken(sas_token, sizeofarray(sas_token)) != 0)
  {
    Serial.println("Failed generating MQTT password");
  }
  else
  {
    connectToAzureIoTHub();
  }

}



static void sendTelemetry(int pwm)
{
  
  if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
          &client, NULL, telemetry_topic, sizeof(telemetry_topic), NULL)))
  {
    Serial.println("Failed az_iot_hub_client_telemetry_get_publish_topic");
    return;
  }


  telemetry_doc["temperature"] = water_temperature;
  telemetry_doc["pwm"] = pwm;
  telemetry_doc["automode"] = isAutoMode;
  telemetry_doc["timestamp"] = getCurrentLocalTimeString();
  size_t n = serializeJson(telemetry_doc, json_output);
  Serial.println("ESP8266 Sending telemetry . . . ");
  mqtt_client.publish(telemetry_topic, json_output, n);

  delay(100);
}

// Arduino setup and loop main functions.

void setup()
{
  pinMode(FanControl_R, OUTPUT);
  pinMode(FanControl_L, OUTPUT);
  analogWrite(FanControl_R,0);
  analogWrite(FanControl_L,0);
  establishConnection();
  sensors.begin();
}

void run(){
  Serial.print("水溫:");
  sensors.requestTemperatures();
  water_temperature = sensors.getTempCByIndex(0);
  delay(100);
  Serial.print(t);
  Serial.println("℃");
  if(isAutoMode){
    autoMode();
  }
  sendTelemetry((pwm_R +pwm_L)/2);
}

void autoMode(){
    // 溫度超過25度，打開風扇
  if( water_temperature >= 28.5 && t <29){
    pwm_R = 204;
    pwm_L = 204;
    analogWrite(FanControl_R, pwm_R);
    delay(100);
    analogWrite(FanControl_L, pwm_L);
    delay(100);
    Serial.println();
    Serial.println(F("Fan Speed: 80% "));
  }
  if( water_temperature >= 28 && t <28.5){
    pwm_R = 153;
    pwm_L = 153;
    analogWrite(FanControl_R, pwm_R);
    delay(100);
    analogWrite(FanControl_L, pwm_L);
    delay(100);
    Serial.println();
    Serial.println(F("Fan Speed: 60% "));
  }
  if( water_temperature >= 27.5 && t <28){
    pwm_R = 100;
    pwm_L = 100;
    analogWrite(FanControl_R, pwm_R);
    delay(100);
    analogWrite(FanControl_L, pwm_L);
    delay(100);
    Serial.println();
    Serial.println(F("Fan Speed: 40% "));
  }
  if( water_temperature >= 26.8 && t < 27.5){
    pwm_R = 50;
    pwm_L = 50;
    analogWrite(FanControl_R, pwm_R);
    delay(100);
    analogWrite(FanControl_L, pwm_L);
    delay(100);  
    Serial.println(F("Fan Speed: 20% "));
  }
  else if ( water_temperature < 26.8){
    pwm_R = 0;
    pwm_L = 0;
    analogWrite(FanControl_R, pwm_R);
    delay(100);
    analogWrite(FanControl_L, pwm_L);
    delay(100);  
    Serial.println(F("Fan Speed: 0% "));
  }
}
void loop()
{
  if (millis() > next_telemetry_send_time_ms)
  {
    // Check if connected, reconnect if needed.
    if (!mqtt_client.connected())
    {
      establishConnection();
    }
    run();
    next_telemetry_send_time_ms = millis() + TELEMETRY_FREQUENCY_MILLISECS;
  }

  // MQTT loop must be called to process Device-to-Cloud and Cloud-to-Device.
  mqtt_client.loop();
  delay(500);
}
