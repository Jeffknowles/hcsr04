
//// pin settings
const int pingPin = 11; // trigger for sonar pulses
const int echoPin = 12; // return for sonar pulses
const int phonePin1 = 9; //
const int phonePin2 = 10;
const int dialPin = 5;  // analog pin for the dial
//const int modePins[2] = {3, 4}; // pins for the 3way mode switch
//const int buttonPin = 2;  // pin for the tigger button
const int nch = 10; // number of neurons
int ledPins[nch] = {5, 6, 7, 8, 13, 3, 2, 4}; // indicates the arduino pin for each light
const int sensory_factor = 10;
const boolean printout = false;
const boolean pong_only_in_range = true;


const int pingLen = 20.; // ping length in microseconds
const int minIPI = 100; // minimum interping interval in miliseconds
const float sense_thresh_i = 500; // threshold where responses turn on

//
float sense_thresh = sense_thresh_i;
boolean button_pressed = false;
int mode = 0;
float target_distance = sense_thresh;
long last_ping = 0;
long currentIPI = minIPI;

// connection settings - declare connections between neurons
const int maxCon = 5;
const int connections[nch][maxCon] = {  // row i indicates (densly) the connections emmenating from the ith element
  // -1 is a placeholder for no connection.  each row needs macCon entries
  {1, 2, 3, -1, -1  }, // 0's outputs
  {2, 3, -1, -1, -1  }, // 1's outputs
  {3, 4, 5, -1, -1  }, // 2's outputs
  {4, 5, 9, -1, -1  }, // 3's outputs
  {4, 5, 6, 8,  -1  }, // 4's outputs
  {3, 4, 6, 7, 8    }, // 5's outputs
  {3, 5, 7, 8, 9    }, // 6's outputs
  {8, 9, 4, -1, -1  }, // 7's outputs
  {9, 6, 5, -1, -1  }, // 8's outputs
  {6, 7, 8, -1, -1  }
}; // 9's outputs

// neural settings and preallocation
const float k = 1; // magnitude of the leak
const float thresh[nch] = {200,   5,   2,   3, 2.3,  4,   3,   4,   4,   4};
const long spike_len[nch] =     {1,  20,  35,  20,  10, 27,  31,  50,  70, 300};
float v[nch] =            {0,   0,   0,   0,   0,  0,   0,   0,   0,   0};
long last_time = 0; // records the loop time for integration

void setup() {
  Serial.begin(9600);
  // setup() runs once:
  for (int ch = 0; ch <= nch; ch++) {
    pinMode(ledPins[ch], OUTPUT);
  }
  pinMode(pingPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(phonePin1, OUTPUT);
  pinMode(phonePin2, OUTPUT);
//  pinMode(modePins[0], INPUT);
//  pinMode(modePins[1], INPUT);
//  pinMode(buttonPin, INPUT);
//  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, RISING); // maybe should change to rising edge
  interrupts();
}


void loop() {
  // loop() runs repeatedly:

  //read switches
  sense_thresh = sense_thresh_i * analogRead(dialPin) / 1024;
  currentIPI = (sense_thresh * 29 * 2 * sensory_factor) / 1000;
  if (printout)
  {
    Serial.print("target distance: ");
    Serial.print(target_distance);
    Serial.print(" sense thresh: ");
    Serial.print(sense_thresh);
    Serial.print(" mode:");
    Serial.println(mode);
  }
//  boolean mode0 = digitalRead(modePins[0]);
//  boolean mode1 = digitalRead(modePins[1]);
//  if (mode0) {
//    mode = 0; // if mode0 is high, mode is 0
//  }
//  else if (mode1) {
//    mode = 2; // if mode1 is high, mode is 2
//  }
//  else {
//    mode = 1; // if neigher is high, mode is 1
//  }
  mode = 1;

  // measure echo depending on mode
  switch (mode) {
    case 0:
      {
        if (button_pressed)
        {
          target_distance = microsecondsToCentimeters(doPing());
          button_pressed = false;
          last_ping = millis();
        }
      }
      break;
    case 1:
      {
        if (millis() > last_ping + currentIPI)
        {
          target_distance = microsecondsToCentimeters(doPing());
          last_ping = millis();
        }
      }
      break;
    case 2:
      {
        if (millis() > last_ping + minIPI) {
          target_distance = microsecondsToCentimeters(doPing());
          last_ping = millis();
        }
      }
      break;
  }


  //Serial.println(cm);
  // measure timing
  long dt = micros() - last_time;
  last_time = micros();
  // set v[0] based on sonar
  if (target_distance < sense_thresh & v[0] >= 0) {
    v[0] = v[0] + float(1) * sense_thresh / target_distance;
  }

  // loop thru neurons
  for (int ch = 0; ch < nch; ch++) {
    //Serial.print(v[ch]);
    //Serial.print(' ');
    if (v[ch] >= 0) { // if neuron is in integrate mode
      v[ch] = v[ch]  - k * v[ch] * float(dt) / 1000000; // decay v to 0
      v[ch] = max(v[ch], 0);
      // if the neuron crosses threshold, fire and increment outputs
      if (v[ch] > thresh[ch]) {
        if (ledPins[ch] > 0) {
          digitalWrite(ledPins[ch], HIGH);
        }
        v[ch] = -1; // v<0 stores that the neuron is in firing state
        for (int syn = 0; syn < maxCon; syn++) { // loop thru synaptic outputs
          // if connection is real and postsyn element is not in firing, incriment its v
          if (connections[ch][syn] >= 0 & v[connections[ch][syn]] >= 0) {
            v[connections[ch][syn]] ++;
          }
        }
      }
    }
    else { // otherwise if neuron is in spike mode
      if (v[ch] < -1 * spike_len[ch]) { // if the time since spike onset is up, end spike
        v[ch] = 0; // set voltage to 0
        if (ledPins[ch] > 0) {
          digitalWrite(ledPins[ch], LOW);
        }
      }
      else {
        v[ch] = v[ch] - float(dt) / 1000; // otherwise decrment v by dt to record time
      }
    }
  }

} // loop fin


////// Sub functions
void buttonPress()
{
  button_pressed = true;
  Serial.println("button");
}



float doPing() {
  // put out pulse
  float dur;
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(pingLen);
  digitalWrite(pingPin, LOW);
  if (pong_only_in_range) {
    if (target_distance < sense_thresh) {
      digitalWrite(phonePin1, HIGH);
      digitalWrite(phonePin1, LOW);
    }
  }
  else {
    digitalWrite(phonePin1, HIGH);
    digitalWrite(phonePin1, LOW);
  }

  dur = pulseIn(echoPin, HIGH, long(0.90 * minIPI * 1000));
  if (dur < sense_thresh * 2 * 29) {
    digitalWrite(phonePin2, HIGH);
    digitalWrite(phonePin2, LOW);
  }

  // measure echo
  // Serial.println(dur);
  return dur;
}

//float averagePings(int ntrials) {
//  // average the results from ntrials pings
//  float delaySum;
//  for (int i = 0; i < ntrials; i = i + 1) {
//    delaySum = delaySum + doPing();
//    delay(minIPI);
//  }
//  return delaySum / ntrials;
//}

float microsecondsToCentimeters(float microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

