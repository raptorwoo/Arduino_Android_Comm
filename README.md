# Arduino_Android_Comm
Communication code for Arduino - Android. 

This program was made to produce a prototype using Arduino, and to check
how BLE communication works with an Android device. The data that Arduino 
sends to the Android device was hard-coded to be incremented by 1 each time 
button was pressed. 

The main reason binary code was used to communicate was because parsing 
using ArduinoJSON required large resource which Arduino did not have. 
As a result, the machine failed to communicate successfully once in 10 communications.

Libraries Used
Bounce2
ArduinoJSON -  Benoit BLANCHON
