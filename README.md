# Morse Code 📠 (i couldn't find a good morse code related emoji so have a fax machine)
## Monica Chan (mochan@mit.edu)

[demo video] (https://youtu.be/16vH2PdPRP8)
## Features Implemented

- HTTP POST request every time the button at pin 34 is pressed.
- Setup appropriately maps inputs from the dash button (pin 39) and dot button (45) and rejects invalid Morse Code characters.
- Setup appropriately differentiates between a new letter prompt (one push on pin 38) and a space character (two pushes on pin 38)
- Prints the current message to the screen. 
- Clears the message after an HTTP post.


## List of Functions (In order as they are in the code, more detailed descriptions can be found in the code)
#### Ones given:
- char_append (used in the http get function to append to the response buffer)
- do_http_request (performs an http post/get request)

#### New ones:
- dot_sm: state machine for detecting whether the button at pin 45 has been pushed and released, if it has, add a dot to the message. 
- dash_sm: state machine for detecting whether the button at pin 39 has been pushed and released, if it has, add a dash to the message. 
- space_sm: state machine for detecting/differentiating between a new letter request and a space character request. 
- http_sm: state machine for the button at pin 34 waiting to see if a new http post request should be made. 
- http_post: performs an http post request with properly json formatted body.

### General Structure/Tracing What Happens:
```cpp
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
```
I start with a digitalRead of the four buttons, and the message is continually printed at each iteration of the loop. There is a little timer at the end for the loop speed.

- The dot_sm and dash_sm are both pretty straightforward; just looking for a press/release sequence and add a corresponding character to the letter character array.
- More spicy stuff in the space_button state machine:
    - Just like the other two state machines, this state machine looks for a press/release sequence. However, in the release state, it waits one second for a second push. If a second push happens, the code should add a space to the message. In the event that there is no push, the current letter array gets concatenated to the message array, and then the letter array gets cleared. In any event, the letter array gets cleared. 
    -  So for instance, if you inputted a dot and dash, then hit the button twice, the code will not add an A and a space, it will just add a space. so don't do that. Input a dot and dash, then press the button once, and after that push the button twice. There was no spec on the design exercise description about this; so that is just how I implemented it. 
- Then, the http_sm runs, waiting for a press/release sequence for pin 34, if one is detected the code properly json formats the message and sends it to the server, clears the message array & letter array. 


### Important Design notes/Thinking
- Initially, I tried performing the double press logic without time altogether, but the state machine got too messy. My best working version of this would basically only add the character to the message once the next character was started. But, after trying to compose a message with this setup I realized UI was *important* and it was a little too confusing. Maybe in the future this is something I can try to improve, since right now there is the 1 second waiting period between adding a character to the message before you can start the new character. 
- I also tried making a decision tree for the mapping of morse code character to letter, but it was also a little too complicated to keep track of everything. I figured that in this case, iterating through an arary is fine since the array is relatively short so O(n) runtime isn't terrible, but in other applications this may not be the case and the O(nlogn) runtime of the decision tree may be more favorable.


