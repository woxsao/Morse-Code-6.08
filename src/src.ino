#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>

TFT_eSPI tft = TFT_eSPI(); 

//defining button pins
const int DOT_BUTTON = 45;
const int DASH_BUTTON = 39;
const int SPACE_BUTTON = 38;
const int HTTP_BUTTON = 34;

//defining states for the dot_sm below
const int DOT_START = 0;
const int DOT_DOWN = 1;
const int DOT_RELEASE = 2;
int dot_state = DOT_START;

//defining states for the dash_sm below
const int DASH_START = 0;
const int DASH_DOWN = 1;
const int DASH_RELEASE = 2;
int dash_state = DASH_START;

//defining states for the http_sm below
const int HTTP_START = 0;
const int HTTP_DOWN = 1;
const int HTTP_RELEASE = 2;
int http_state = HTTP_START;

//defining states for the space_sm below
const int SPACE_START = 0;
const int SPACE_DOWN = 1;
const int SPACE_RELEASE = 2;
int space_state = SPACE_START;


unsigned long space_timer = 0; //timer for double click
unsigned long button_timer = 0; //timer for main loop

char message[1000];
char letter[50];

char morse[][6] = {" ", ".-", "-...", "-.-.", "-..",".", "..-.", "--.", "....", "..", ".---",
                   "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
                   "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--",
                   "....-", ".....", "-....", "--...", "---..", "----.", "-----"};
char alphabet[][6] = {" ", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
                    "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
                    "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4",
                    "5", "6", "7", "8", "9", "0"};

//WIFI-------------------------------------------------------
const char USER[] = "mochan";
//byte bssid[] = {0x5C, 0x5B, 0x35, 0xEF, 0x59, 0xC3}; //6 byte MAC address of AP you're targeting. Next House 5 west
byte bssid[] = {0x5C, 0x5B, 0x35, 0xEF, 0x59, 0x03}; //3C
//byte bssid[] = {0xD4, 0x20, 0xB0, 0xC4, 0x9C, 0xA3}; //quiet side stud
char network[] = "MIT";
char password[] = "";
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //ms to wait between posting step
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
unsigned long posting_timer;

void setup() {
  tft.init();  //init screen
  tft.setRotation(3); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_WHITE, TFT_BLACK); //set color of font to green foreground, black background
  Serial.begin(115200); //begin serial comms

  //wifi
  WiFi.begin(network, password, 1, bssid);
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  pinMode(DOT_BUTTON, INPUT_PULLUP);
  pinMode(DASH_BUTTON, INPUT_PULLUP);
  pinMode(SPACE_BUTTON, INPUT_PULLUP);
  pinMode(HTTP_BUTTON, INPUT_PULLUP);

}

/*----------------------------------
 * dot_sm Function:
    This state machine's goal is to appropriately add a dot to the letter array if the dot button has been pushed.
 * Arguments:
 *    int button_val: An integer, 0 or 1, corresponding to whether the button has been pushed or not, respectively.
 */
void dot_sm(int button_val){
  switch(dot_state){
    case(DOT_START):
      if(button_val == 0)
        dot_state = DOT_DOWN;
      break;
    case(DOT_DOWN):
      if(button_val == 1)
        dot_state = DOT_RELEASE;
      
      break;
    case(DOT_RELEASE):
      dot_state = DOT_START;
      strcat(letter, ".");
      
      break;
  }
}

/*----------------------------------
 * dash_sm Function:
    This state machine's goal is to appropriately add a dot to the letter array if the dot button has been pushed. 
 * Arguments:
 *    int button_val: An integer, 0 or 1, corresponding to whether the button has been pushed or not, respectively.
 */
void dash_sm(int button_val){
  switch(dash_state){
    case(DASH_START):
      if(button_val == 0)
        dash_state = DASH_DOWN;
      break;
    case(DASH_DOWN):
      if(button_val == 1)
        dash_state = DASH_RELEASE;
      

      break;
    case(DASH_RELEASE):
      dash_state = DASH_START;
      strcat(letter, "-");
      break;
  }
}


/*----------------------------------
 * space_sm Function:
    This state machine's goal is to determine whether to start a new letter (one push) or add a space (two pushes). In either case, this is where the letter array gets cleared
    to start a new letter (since after a space you would be starting a new character anyway).
 * Arguments:
 *    int button_val: AN integer (0 or 1) corresponding to whether the space/new letter button has been pushed or unpushed.
 */
void space_sm(int button_val){
  switch(space_state){
    case(SPACE_START):
      if(button_val == 0)
        space_state = SPACE_DOWN;
      break;
    case(SPACE_DOWN):
      if(button_val == 1)
        space_state = SPACE_RELEASE;
      break;
    case(SPACE_RELEASE):
      space_timer = millis();
      //ended up using time to detect the second push. 
      while(millis()-space_timer <= 1000){
        if(digitalRead(SPACE_BUTTON) == 0){
          strcpy(letter, " ");
        }
      }
      char* final_letter = find_char();
      //adding the final_letter to the message as long as it was not a null (no match found)
      if(final_letter != "\0"){
        strcat(message, final_letter); 
        
      }
      space_state = SPACE_START;
      strcpy(letter, "");

    }
      
}
/*----------------------------------
 * do_http_request Function:
 * Arguments:
 *    char* host: null-terminated char-array containing host to connect to
 *    char* request: null-terminated char-arry containing properly formatted HTTP request
 *    char* response: char-array used as output for function to contain response
 *    uint16_t response_size: size of response buffer (in bytes)
 *    uint16_t response_timeout: duration we'll wait (in ms) for a response from server
 *    uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
 * Return value:
 *    void (none)
 */
void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      //if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}        

/*----------------------------------
 * char_append Function:
 * Arguments:
 *    char* buff: pointer to character array which we will append a
 *    char c: 
 *    uint16_t buff_size: size of buffer buff
 *    
 * Return value: 
 *    boolean: True if character appended, False if not appended (indicating buffer full)
 */
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
        int len = strlen(buff);
        if (len>buff_size) return false;
        buff[len] = c;
        buff[len+1] = '\0';
        return true;
}

/*----------------------------------
 * http_post Function:
   This function's purpose is to perform a post request with the correct username (defined as a global variable) and properly format the body to be in JSON format.
 * Return value: 
 *    
 */
void http_post(){
  if (millis()-posting_timer > POSTING_PERIOD){
    char body[100]; //for body
    sprintf(body,"{\"user\":\"%s\",\"message\":\"%s\"}", USER, message);//generate body, posting to User, 1 step
    int body_len = strlen(body); //calculate body length (for header reporting)
    sprintf(request_buffer,"POST http://608dev.net/sandbox/morse_messenger HTTP/1.1\r\n");
    strcat(request_buffer,"Host: 608dev.net\r\n");
    strcat(request_buffer,"Content-Type: application/json\r\n");
    sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
    strcat(request_buffer,"\r\n"); //new line from header to body
    strcat(request_buffer,body); //body
    strcat(request_buffer,"\r\n"); //new line
    Serial.println(request_buffer);
    do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
    Serial.println(response_buffer); //viewable in Serial Terminal
    //LCD Display:
    char output[80];
    posting_timer = millis();

  }
}

/*----------------------------------
 * http_sm Function:
    This function's goal is to indicate whether an http post should be made. If the button is pushed and then released, run http_post()
    and then reset the message array and clear the screen.
 * Arguments:
 *    int button_val: An integer (1 or 0) representing the button state (unpushed or pushed, respectively)
 *    
 */
void http_sm(int button_val){
  switch(http_state){
    case(HTTP_START):
      if(button_val == 0)
        http_state = HTTP_DOWN;
      break;
    case(HTTP_DOWN):
      if(button_val == 1)
        http_state = HTTP_RELEASE;
      break;
    case(HTTP_RELEASE):
      http_post();
      http_state = HTTP_START;
      strcpy(message, "");
      strcpy(letter, "");
      tft.fillScreen(TFT_BLACK);
      break;
  }
}

/*----------------------------------
 * find_char Function:
    This function's goal is to use the input letter and try to map it to the global morse code alphanumeric list (defined at the top) and return the corresponding
    character if it finds a match. If a match isn't found, return a null string.
 * Return value: 
 *    a character pointer to a character array.
 */
char* find_char(){
  for(int i = 0; i < 37; i++){
    if(strcmp(morse[i], letter)==0){
      Serial.println(i);
      Serial.print("found char; ");
      Serial.println(alphabet[i]);
      return alphabet[i];
    }
  }
  return "\0";
}

void loop() {
  button_timer = millis();
  // put your main code here, to run repeatedly:
  dot_sm(digitalRead(DOT_BUTTON));
  dash_sm(digitalRead(DASH_BUTTON));
  space_sm(digitalRead(SPACE_BUTTON));
  http_sm(digitalRead(HTTP_BUTTON));
  tft.setCursor(0,0,4);
  tft.println(message);
  while(millis()-button_timer < 30);
    
}
