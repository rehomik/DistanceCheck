#include <LiquidCrystal.h>
#include <SPI.h>
#include <WiFi.h>

LiquidCrystal lcd(9, 8, 5, 4, 3, 2);

// wifi info
char* LOCAL_SSID = "XXX";
const char* LOCAL_PW = "XXX";

// server info
const char* SERVER_URL = "192.168.0.6";
const char* SERVER_PAGE = "/v1/check/checkin";
const int PORT = 8093;

// constant
const long TOGGLE_TIME = 200;
const short DIST_CHAIR = 60;
const long SENSOR_CHECK_DELAY = 10000;
const long SETUP_END_DELAY = 5000;

// digital port
const int GREEN_LED_PORT = 10;
const int BUTTON_PORT = 7;
const int ULTRA_SONIC_PORT = 6;

const int ULTRA_SONIC_RATE = 9600;
const int SENSING_COUNT = 5;
const long DELAY_PER_SENSOR_CHECKING = 1000;
const long WIFI_CONNECTION_WAIT_TIME = 5000;

int wifi_status = WL_IDLE_STATUS;
int _currentState = LOW;
int _prePressed = LOW;
int _checkCount = 0;
unsigned int _failCount = 0;
unsigned int _okCount = 0;

long _sensorCmArray[SENSING_COUNT] = {-1};
long _prePressedTime = 0;

boolean _preState = false;
boolean _wifiAvailable = true;

WiFiClient _client;

enum {
  US_WAITING = 0,
  US_RUNNING = 1
};

void setup()
{
  // LCD setup
  lcd.begin(16, 2);
  
  // ultra sonic setup
  Serial.begin(ULTRA_SONIC_RATE);
  
  // board setup
  pinMode(GREEN_LED_PORT, OUTPUT);
  pinMode(BUTTON_PORT, INPUT);
    
  // wifi setup
  if (WiFi.status() == WL_NO_SHIELD)
  {
    lcd.print("WiFi error");
    
    _wifiAvailable = false;
  }
  else
  {
    lcd.print("WiFi OK");
    
    _wifiAvailable = true;
  }
  
  delay(2000);
  
  while (wifi_status != WL_CONNECTED)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");
    
    wifi_status = WiFi.begin(LOCAL_SSID, LOCAL_PW);
    
    delay(WIFI_CONNECTION_WAIT_TIME);
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected WiFi");
  lcd.setCursor(0, 1);
  lcd.print(LOCAL_SSID);
  
  delay(SETUP_END_DELAY);
  
  lcd.clear();
}

void loop()
{
  int is_pressed = digitalRead(BUTTON_PORT);
  
  long millis_time = millis();
  
  _currentState = toggleButton(is_pressed, millis_time);
  
  if ( (US_WAITING == _currentState) && _wifiAvailable )
  {
    lcd.setCursor(0, 0);
    lcd.print("Wait for signal");
    
    resetCheckCount();
    
    turnOffLED();
  }
  else if ( (US_RUNNING == _currentState) && _wifiAvailable )
  {
    turnOnLED();

    if (checkComplete())
    {
      long result_cm_value = filter(_sensorCmArray);
      
      if (300 >= result_cm_value)
      {
        boolean is_on_chair = checkChair(result_cm_value);
      
        displayResult(result_cm_value);
        
        if (compareChairState(is_on_chair))
        {
          // send to server
          sendDataToServer();
          
          clearStateInfo();
        }
        else
        {
          // dont send to server
          
          // accumulate time of current state.
        }
        
        delay(SENSOR_CHECK_DELAY);
      }
    }
    else
    {
      displayCheckMsg(_checkCount);
    }
  }
}

void clearStateInfo()
{
}

void sendDataToServer()
{
  char sending_string[128];
        
  sprintf(
    sending_string,
    "{\"state\":%d}",
    _preState
  );
  
  _client.stop();
  
  if (_client.connect(SERVER_URL, PORT))
  {
    ++_okCount;
    
    char out_buffer[256];
    
    sprintf(out_buffer, "POST %s HTTP/1.1", SERVER_PAGE);
    _client.println(out_buffer);
    
    _client.println("Host: XXX");
    _client.println("User-Agent: Arduino/1.0");
    _client.println("Content-Type:application/json");
    
    sprintf(out_buffer, "Content-Length: %u", strlen(sending_string));
    _client.println(out_buffer);
    _client.println();
    _client.println(sending_string);
    
    _client.println("Connection:close");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sending success - ");
    lcd.print(_okCount);
  }
  else
  {
    ++_failCount;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sending fail - ");
    lcd.print(_failCount);
  }
}

boolean checkChair(long result_cm_value)
{
  // sit up.
  if (result_cm_value > DIST_CHAIR)
  {
    return false;
  }
  
  // sit down.
  return true;
}

boolean compareChairState(boolean current_state)
{
  if (current_state != _preState)
  {
    _preState = current_state;
    
    return true;
  }
  
  return false;
}

long filter(long* sensor_cm_value_array)
{
//  // limit filter
//  for (int i = 0; i < SENSING_COUNT; ++i)
//  {
//    long value = sensor_cm_value_array[i];
//    
//    if ( (value <= 0) || (value > 100))
//    {
//      sensor_cm_value_array[i] = -1;
//    }
//  }
  
  // min max filter
  long min_value = 99999;
  short min_value_index = -1;
  
  long max_value = -1;
  short max_value_index = -1;
  
  for (int i = 0; i < SENSING_COUNT; ++i)
  {
    int sensor_cm_value = sensor_cm_value_array[i];
    if (-1 == sensor_cm_value)
    {
      continue;
    }
    
    long indexed_min_value = min(min_value, sensor_cm_value);
    if (min_value != indexed_min_value)
    {
      min_value_index = i;
      min_value = indexed_min_value;
    }
    
    long indexed_max_value = max(max_value, sensor_cm_value);
    if (max_value != indexed_max_value)
    {
      max_value_index = i;
      max_value = indexed_max_value;
    }
  }
  
  if (-1 < min_value_index)
  {
    sensor_cm_value_array[min_value_index] = -1;
  }
  
  if (-1 < max_value_index)
  {
    sensor_cm_value_array[max_value_index] = -1;
  }
  
  // avarage
  long result_value = 0;
  short sum_count = 0;
  
  for (int i = 0; i < SENSING_COUNT; ++i)
  {
    long cm_value = sensor_cm_value_array[i];
    
    if (-1 < cm_value)
    {
      result_value += cm_value;
      ++sum_count;
    }
  }
  
  return (result_value / sum_count);
}

void displayResult(long cm)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("cm: ");
  lcd.print(cm);
  lcd.setCursor(0, 1);
  
  lcd.print("ok: ");
  lcd.print(_okCount);
  
  lcd.print("  fail: ");
  lcd.print(_failCount);
}

void displayCheckMsg(int check_count)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Checking");
  
  for (int i = 0; i < check_count; ++i)
  {
    lcd.print(".");
  }
  
  delay(DELAY_PER_SENSOR_CHECKING);
}

int toggleButton(int is_pressed, long millis_time)
{
  if (
      (HIGH == is_pressed) &&
      (LOW == _prePressed) &&
      ((millis_time - _prePressedTime) > TOGGLE_TIME)
      )
  {
    _prePressedTime = millis_time;
    
    if (US_RUNNING == _currentState)
    {
      lcd.clear();
      
      return US_WAITING;
    }
    
    _currentState = US_RUNNING;
    
    return US_RUNNING;
  }
  
  _prePressed = is_pressed;
  
  return _currentState;
}

void turnOffLED()
{
  digitalWrite(GREEN_LED_PORT, LOW);
}

void turnOnLED()
{
  digitalWrite(GREEN_LED_PORT, HIGH);
}

void resetCheckCount()
{
  _checkCount = 0;
}

boolean checkComplete()
{
  if (_checkCount == SENSING_COUNT)
  {
    resetCheckCount();
    
    return true;
  }
  
  pinMode(ULTRA_SONIC_PORT, OUTPUT);
  digitalWrite(ULTRA_SONIC_PORT, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRA_SONIC_PORT, HIGH);
  delayMicroseconds(5);
  digitalWrite(ULTRA_SONIC_PORT, LOW);
  
  pinMode(ULTRA_SONIC_PORT, INPUT);
  
  long sensor_value = pulseIn(ULTRA_SONIC_PORT,  HIGH);
  
  _sensorCmArray[_checkCount] = msToCm(sensor_value);
  
  ++_checkCount;
  
  return false;
}

long msToCm(long ms)
{
  return ms / 29 / 2;
}
