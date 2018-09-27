// AC Light System

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <EEPROM.h>
#include <Arduino.h>

RtcDS3231<TwoWire> Rtc(Wire);


LiquidCrystal lcd(A0, A1, A2, A3, 12, 13);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {5, 4, 10, 11}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int readPin = 2;
int pulsePin = 3;

volatile boolean acStatus = 0;
boolean flag = 0;

int onStatus = 0;

unsigned long timerPulse = 0;
unsigned long timerClock = 0;

bool pulseState = 0;

int page = 1;
int place = 0;
int number = 0;

int displayPower = 0;

String val;
char key;

unsigned int holdTime = 1000;
unsigned int debounce = 10;
unsigned long acDelay = 8500;


int powerMax = 0;
unsigned long minMax = 0;
unsigned long maxMin = 0;
unsigned long personalIntervalOn = 0;
unsigned long personalIntervalOff = 0;
unsigned long dayNightIntervalOn = 0;
unsigned long dayNightIntervalOff = 0;

extern volatile unsigned long timer0_millis;

unsigned long currentMillis = 0;
long currentMillis1 = 0;
int power = 0;

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u:%02u"),

             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
  val = datestring;
}


void setup() {
  // put your setup code here, to run once:

  pinMode(readPin, INPUT_PULLUP);
  pinMode(pulsePin , OUTPUT);

  attachInterrupt( digitalPinToInterrupt(readPin), ISR_pulse, FALLING );

  Serial.begin(9600);
  Rtc.Begin();
  lcd.begin(16, 2);
  lcd.clear();

  //  RtcDateTime cd(2018, 9, 7, 21, 23, 0);
  //  Rtc.SetDateTime(cd);
  //auto date = Rtc.GetDateTime();
  //printDateTime(date);


  customKeypad.setHoldTime( holdTime );
  customKeypad.setDebounceTime( debounce );
  customKeypad.addEventListener( keypadEvent );

  bootup();

  delay( 500 );
  lcd.clear();
  page9();
  delay( 500 );
  lcd.clear();
  page1( 'E' );


  Serial.println( personalIntervalOn );
  Serial.println(  personalIntervalOff );
  Serial.println(  dayNightIntervalOn );
  Serial.println(  dayNightIntervalOff );

}

void loop() {
  // put your main code here, to run repeatedly:

  if ( millis() > 2592000000 ) {
    setMillis( 0 );
    currentMillis = millis();
    currentMillis1 = millis();
  }


  if ( onStatus == 1 && currentMillis1 + 60000 < millis() ) {

    currentMillis1 = millis();


    long realTime = timeFromRTC();



    if ( realTime > dayNightIntervalOn && realTime < ( dayNightIntervalOff - minMax - maxMin ) ) {

      currentMillis = millis();
      currentMillis1 = millis();
      power = acDelay;
      while ( currentMillis + minMax > millis() ) {
        if ( currentMillis1 + 5 < millis() ) {
          currentMillis1 = millis();
          power = map(  millis() , currentMillis , currentMillis + minMax , acDelay , powerMax );
        }
        if ( power < 0 ) {
          power = 0;
        }
        if ( power > acDelay ) {
          power = acDelay;
        }
        powerAc ( power );
      }

      realTime = timeFromRTC();
      currentMillis = millis();
      power = powerMax;
      if ( power < 0 ) {
        power = 0;
      }
      if ( power > acDelay ) {
        power = acDelay;
      }
      while (  currentMillis + ( dayNightIntervalOff - realTime - maxMin - minMax )  > millis() ) {
        powerAc( power );
      }

      currentMillis = millis();
      currentMillis1 = millis();
      while ( currentMillis + maxMin > millis() ) {
        if ( currentMillis1 + 5 < millis() ) {
          currentMillis1 = millis();
          power = map(  millis() , currentMillis , currentMillis + minMax , powerMax , acDelay );
        }
        if ( power < 0 ) {
          power = 0;
        }
        if ( power > acDelay ) {
          power = acDelay;
        }
        powerAc ( power );
      }


      //      date = Rtc.GetDateTime();
      //      printDateTime(date);
      //      str1 = val.substring( 0 , 2 );
      //      n = str1.toInt();
      //      str2 = val.substring( 3 , 5 );
      //      m = str2.toInt();
      //      setMillis = ( n * 3600000 ) + ( m * 60000 );
      //
      //      currentMillis = millis();
      //      if ( setMillis > dayNightIntervalOff ) {
      //        while ( (setMillis - dayNightIntervalOff) < (millis() - currentMillis) ) {
      //          digitalWrite(pulsePin , LOW );
      //        }
      //      }
    }
  }

  else if ( onStatus == 2 ) {

    power = acDelay;
    currentMillis = millis();
    currentMillis1 = millis();
    while ( currentMillis + minMax > millis() ) {
      if ( currentMillis1 + 5 < millis() ) {
        currentMillis1 = millis();
        power = map(  millis() , currentMillis , currentMillis + minMax , acDelay , powerMax );
      }
      if ( power < 0 ) {
        power = 0;
      }
      if ( power > acDelay ) {
        power = acDelay;
      }
      powerAc ( power );
    }

    currentMillis = millis();
    power = powerMax;
    if ( power < 0 ) {
      power = 0;
    }
    if ( power > acDelay ) {
      power = acDelay;
    }
    while ( ( currentMillis + ( (personalIntervalOn - minMax) - maxMin )) > millis() ) {
      powerAc( power );
    }

    currentMillis = millis();
    currentMillis1 = millis();
    while ( currentMillis + maxMin > millis() ) {
      if ( currentMillis1 + 5 < millis() ) {
        currentMillis1 = millis();
        power = map(  millis() , currentMillis , currentMillis + minMax , powerMax , acDelay );
      }
      if ( power < 0 ) {
        power = 0;
      }
      if ( power > acDelay ) {
        power = acDelay;
      }
      powerAc ( power );
    }

    currentMillis = millis();
    while ( currentMillis + personalIntervalOff > millis() ) {
      digitalWrite(pulsePin , LOW );
    }

  }

  else {
    key = customKeypad.getKey();
  }
}


void ISR_pulse() {
  acStatus = 1;
}

void powerAc( int x ) {
  if ( acStatus ) {
    //    if ( micros() < 4294937295 ) {
    timerPulse = micros();
    //    }
    acStatus = 0;
    flag = 1;
  }

  if ( micros() >= timerPulse + x && flag == 1 ) {
    pulseState = 1;
    digitalWrite(pulsePin , pulseState );
    flag = 0;
  }

  if ( micros() > ( timerPulse + x + 500 ) &&  pulseState == 1 ) {
    pulseState = 0;
    digitalWrite(pulsePin , pulseState );
  }

}

long timeFromRTC() {
  auto date = Rtc.GetDateTime();
  printDateTime(date);
  String str1 = val.substring( 0 , 2 );
  int n = str1.toInt();
  String str2 = val.substring( 3 , 5 );
  int m = str2.toInt();
  long setMillis = ( n * 3600000 ) + ( m * 60000 );

  return setMillis;
}

void bootup() {

  powerMax = EEPROMReadint(0);

  if ( powerMax < 1 || powerMax > 100 ) {
    powerMax = 50 ;
  }
  displayPower = powerMax;
  powerMax = map( powerMax , 0 , 100 , acDelay , 0 );

  minMax = (EEPROMReadint(2) * 60000);

  if ( minMax < 60000 || minMax > 5940000 ) {
    minMax = 60000;
  }

  maxMin = ( EEPROMReadint(4) * 60000 );

  if ( maxMin < 60000 || maxMin > 5940000 ) {
    maxMin = 60000;
  }

  personalIntervalOn = EEPROMReadlong(6);

  if ( personalIntervalOn < 600000 || personalIntervalOn > 86400000 ) {
    personalIntervalOn = 600000;
  }

  personalIntervalOff = EEPROMReadlong(10);

  if ( personalIntervalOff  < 600000 || personalIntervalOff  > 86400000 ) {
    personalIntervalOff  = 600000;
  }
  if ( personalIntervalOn  <=  (minMax + maxMin) ) {
    personalIntervalOn  = minMax + maxMin + 600000;
  }

  dayNightIntervalOn = EEPROMReadlong(14);

  if ( dayNightIntervalOn < 600000 || dayNightIntervalOn > 86400000 ) {
    dayNightIntervalOn = 28800000;
  }

  dayNightIntervalOff = EEPROMReadlong(18);

  if ( dayNightIntervalOff  < 600000 || dayNightIntervalOff  > 86400000 ) {
    dayNightIntervalOff  = 64800000;
  }

  if ( dayNightIntervalOff < ( dayNightIntervalOn + minMax + maxMin ) ) {
    dayNightIntervalOn = 28800000;
    dayNightIntervalOff  =  64800000;
  }

}

void setMillis ( unsigned long value ) {
  uint8_t oldSREG = SREG;

  cli();

  timer0_millis = value;

  SREG = oldSREG;

}


void keypadEvent(KeypadEvent key) {
  switch (customKeypad.getState()) {
    case PRESSED:
      Serial.print(F("Key Pressed: "));
      Serial.println(key);
      switch (key) {
        case 'A':
          if ( onStatus == 0 ) {
            page++;

            if ( page > 8 || page < 1 ) {
              page = 1;
            }

            place = 0;

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.noBlink();

            displayPage( page , 'E' );
          }
          break;

        case 'B':
          break;

        case 'C':
          break;

        case 'D':
          onStatus = 0;
          break;

        case '*':
          break;

        case '#':
          break;

        default:
          if ( isDigit( key ) && onStatus == 0 ) {
            Serial.println(F("Digit Entered"));
            displayPage( page , key );
          }
          break;
      }
      break;

    case HOLD:
      switch (key) {
        case 'D':
          if ( onStatus == 0 ) {
            displayPage( page , 'S' );
          }
          break;
      }
      break;
  }

}


void displayPage( int page , char key ) {

  switch ( page ) {
    case 1:
      Serial.println("Page One Executed");
      page1( key );
      break;

    case 2:
      Serial.println("Page Two Executed");
      page2( key );
      break;

    case 3:
      Serial.println("Page Three Executed");
      page3( key );
      break;

    case 4:
      Serial.println("Page Four Executed");
      page4( key );
      break;

    case 5:
      Serial.println("Page Five Executed");
      page5( key );
      break;

    case 6:
      Serial.println("Page Six Executed");
      page6( key );
      break;

    case 7:
      Serial.println("Page Seven Executed");
      page7( key );
      break;

    case 8:
      Serial.println("Page Eight Executed");
      page8( key );
      break;

    case 9:
      Serial.println("Page Nine Executed");
      page9();
      break;

    case 10:
      Serial.println("Page Ten Executed");
      page10();
      break;

    case 11:
      Serial.println("Page Eleven Executed");
      page11();
      break;
  }
}


void page1( char inKey ) {
  if ( inKey == 'E') {
    lcd.print("Day / Night");
    lcd.setCursor(0, 1);
    lcd.print("Stop");
  }
  else if ( inKey == 'S' ) {
    lcd.setCursor(0, 1);
    lcd.print("Run!");
    onStatus = 1;
  }
}

void page2( char inKey ) {
  if ( inKey == 'E') {
    lcd.print("Personal");
    lcd.setCursor(0, 1);
    lcd.print("Stop");
  }
  else if ( inKey == 'S' ) {
    lcd.setCursor(0, 1);
    lcd.print("Run!");
    onStatus = 2;
  }
}

void page3( char inKey ) {
  if ( inKey == 'E' ) {
    lcd.print("Day / Night Set");
    lcd.setCursor(0, 1);

    String abc;
    if ( (dayNightIntervalOn / 3600000 ) < 24 && (dayNightIntervalOn / 3600000 ) > 0 ) {
      abc  = String ( (dayNightIntervalOn / 3600000 ) ) ;
    }
    if ( abc.length() < 1 ) {
      abc = "00" + abc;
    }
    else if ( abc.length() < 2 ) {
      abc = "0" + abc;
    }
    abc = abc + ":";
    String abc1 = String((dayNightIntervalOn % 3600000 ) / 60000 );

    if ( abc1.length() < 1 ) {
      abc1 = "00" + abc1;
    }
    else if ( abc1.length() < 2 ) {
      abc1 = "0" + abc1;
    }


    String abc2;
    if ( (dayNightIntervalOff / 3600000 ) < 24 && (dayNightIntervalOff / 3600000 ) > 0 ) {
      abc2  = String ( (dayNightIntervalOff / 3600000 ) ) ;
    }
    if ( abc2.length() < 1 ) {
      abc2 = "00" + abc2;
    }
    else if ( abc2.length() < 2 ) {
      abc2 = "0" + abc2;
    }
    abc2 = abc2 + ":";

    String abc3;
    abc3 = String( (dayNightIntervalOff % 3600000 ) / 60000 );

    if ( abc3.length() < 1 ) {
      abc3 = "00" + abc3;
    }
    else if ( abc3.length() < 2 ) {
      abc3 = "0" + abc3;
    }

    val = abc + abc1 + "/" + abc2 + abc3;

    lcd.print(val);
  }

  else if ( isDigit(inKey) ) {

    if ( place > 10) {
      place = 0;
    }
    if ( place == 2 || place == 5 || place == 8 ) {
      place++;
    }

    String checkInputNumbers;

    val.setCharAt( (place) , inKey );
    Serial.println(val);
    lcd.setCursor( 0 , 1 );
    lcd.print(val);
    lcd.setCursor( place , 1 );
    lcd.blink();

    checkInputNumbers = val.substring(0, 2);
    int inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 24 ) {

      val.setCharAt( 0, '0' );
      val.setCharAt( 1, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(3, 1);
      place++;
    }


    checkInputNumbers = val.substring(3, 5);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 59 ) {

      val.setCharAt( 3, '0' );
      val.setCharAt( 4, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(6, 1);
      place++;
    }

    checkInputNumbers = val.substring(6, 8);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 24 ) {

      val.setCharAt( 6, '0' );
      val.setCharAt( 7, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(9, 1);
      place++;
    }


    checkInputNumbers = val.substring(9, 11);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 59 ) {

      val.setCharAt( 9, '0' );
      val.setCharAt( 10, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(10, 1);
      place++;
    }

    place++;
  }
  else if ( inKey == 'S' ) {
    String str1 = val.substring( 0 , 2 );
    int x = str1.toInt();
    long q = (long)x;
    String str2 = val.substring( 3 , 5 );
    int y = str2.toInt();
    long w = (long)y;
    String str3 = val.substring( 6 , 8 );
    int z = str3.toInt();
    long e = (long)z;
    String str4 = val.substring( 9 , 11 );
    int v = str4.toInt();
    long r = (long)v;

    unsigned long sumOn = (q * 3600000) + (w * 60000);
    unsigned long sumOff = (e * 3600000) + (r * 60000);

    Serial.println(sumOn);
    Serial.println(sumOff);
    if ( sumOn <= 86400000 && sumOff <= 86400000 ) {
      EEPROMWritelong( 14 , sumOn );
      EEPROMWritelong( 18 , sumOff );
      lcd.clear();
      page10();
    }
    else {
      lcd.clear();
      page11();
    }
  }
}

void page4( char inKey ) {
  if ( inKey == 'E' ) {
    lcd.print("Personal Set");
    lcd.setCursor(0, 1);
    //


    String abc;
    if ( (personalIntervalOn / 3600000 ) < 24 && (personalIntervalOn / 3600000 ) > 0 ) {
      abc  = String ( (personalIntervalOn / 3600000 ) ) ;
    }
    if ( abc.length() < 1 ) {
      abc = "00" + abc;
    }
    else if ( abc.length() < 2 ) {
      abc = "0" + abc;
    }
    abc = abc + ":";
    String abc1 = String((personalIntervalOn % 3600000 ) / 60000 );

    if ( abc1.length() < 1 ) {
      abc1 = "00" + abc1;
    }
    else if ( abc1.length() < 2 ) {
      abc1 = "0" + abc1;
    }


    String abc2;
    if ( (personalIntervalOff / 3600000 ) < 24 && (personalIntervalOff / 3600000 ) > 0 ) {
      abc2  = String ( (personalIntervalOff / 3600000 ) ) ;
    }
    if ( abc2.length() < 1 ) {
      abc2 = "00" + abc2;
    }
    else if ( abc2.length() < 2 ) {
      abc2 = "0" + abc2;
    }
    abc2 = abc2 + ":";

    String abc3;
    abc3 = String((personalIntervalOff % 3600000 ) / 60000 );

    if ( abc3.length() < 1 ) {
      abc3 = "00" + abc3;
    }
    else if ( abc3.length() < 2 ) {
      abc3 = "0" + abc3;
    }

    val = abc + abc1 + "/" + abc2 + abc3;


    //
    lcd.print(val);
  }

  else if ( isDigit(inKey) ) {

    if ( place > 10) {
      place = 0;
    }
    if ( place == 2 || place == 5 || place == 8 ) {
      place++;
    }

    String checkInputNumbers;

    val.setCharAt( (place) , inKey );
    Serial.println(val);
    lcd.setCursor( 0 , 1 );
    lcd.print(val);
    lcd.setCursor( place , 1 );
    lcd.blink();

    checkInputNumbers = val.substring(0, 2);
    int inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 24 ) {

      val.setCharAt( 0, '0' );
      val.setCharAt( 1, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(3, 1);
      place++;
    }


    checkInputNumbers = val.substring(3, 5);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 59 ) {

      val.setCharAt( 3, '0' );
      val.setCharAt( 4, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(6, 1);
      place++;
    }

    checkInputNumbers = val.substring(6, 8);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 24 ) {

      val.setCharAt( 6, '0' );
      val.setCharAt( 7, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(9, 1);
      place++;
    }


    checkInputNumbers = val.substring(9, 11);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 59 ) {

      val.setCharAt( 9, '0' );
      val.setCharAt( 10, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(10, 1);
      place++;
    }

    place++;
  }
  else if ( inKey == 'S' ) {
    String str1 = val.substring( 0 , 2 );
    int x = str1.toInt();
    long q = (long)x;
    String str2 = val.substring( 3 , 5 );
    int y = str2.toInt();
    long w = (long)y;
    String str3 = val.substring( 6 , 8 );
    int z = str3.toInt();
    long e = (long)z;
    String str4 = val.substring( 9 , 11 );
    int v = str4.toInt();
    long r = (long)v;

    unsigned long sumOn = (q * 3600000) + (w * 60000);
    unsigned long sumOff = (e * 3600000) + (r * 60000);

    Serial.println(sumOn);
    Serial.println(sumOff);

    if ( sumOn <= 86400000 && sumOff <= 86400000 ) {
      EEPROMWritelong( 6 , sumOn );
      EEPROMWritelong( 10 , sumOff );
      lcd.clear();
      page10();
    }
    else {
      lcd.clear();
      page11();
    }
  }
}

void page5( char inKey ) {

  if ( inKey == 'E' ) {
    lcd.print("Clock Set");
    lcd.setCursor(0, 1);
    auto date = Rtc.GetDateTime();
    printDateTime(date);
    lcd.print(val);
  }

  else if ( isDigit( inKey )) {

    if ( place > 7) {
      place = 0;
    }
    if ( place == 2 || place == 5) {
      place++;
    }

    val.setCharAt( (place) , inKey );
    Serial.println(val);
    lcd.setCursor( 0 , 1 );
    lcd.print(val);
    lcd.setCursor( place , 1 );
    lcd.blink();

    String checkInputNumbers = val.substring(0, 2);
    int inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 24  ) {

      val.setCharAt( 0, '2' );
      val.setCharAt( 1, '4' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(0, 1);
      place--;
    }

    checkInputNumbers = val.substring(3, 5);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 59  ) {

      val.setCharAt( 3, '0' );
      val.setCharAt( 4, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(3, 1);
      place--;
    }

    checkInputNumbers = val.substring(6, 8);
    inputNumber = checkInputNumbers.toInt();
    Serial.println(inputNumber);

    if ( inputNumber > 59  ) {

      val.setCharAt( 6, '0' );
      val.setCharAt( 7, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(6, 1);
      place--;
    }


    place++;

  }
  else if ( inKey == 'S' ) {
    String str1 = val.substring( 0 , 2 );
    int x = str1.toInt();
    String str2 = val.substring( 3 , 5 );
    int y = str2.toInt();
    String str3 = val.substring( 6 , 8 );
    int z = str3.toInt();
    RtcDateTime cd(2018, 0, 0, x , y , z );
    Rtc.SetDateTime(cd);
    lcd.clear();
    page10();
  }
}


void page6( char inKey ) {
  if ( inKey == 'E' ) {
    lcd.print("Max Power Set");
    lcd.setCursor(0, 1);
    val = String( displayPower );
    if ( val.length() < 3 ) {
      val = "0" + val;
    }
    val += "%";
    lcd.print(val);
  }
  else if ( isDigit( inKey )) {

    if ( place > 2) {
      place = 0;
    }

    val.setCharAt( (place) , inKey );
    Serial.println(val);
    lcd.setCursor( 0 , 1 );
    lcd.print(val);
    lcd.setCursor( place , 1 );
    lcd.blink();

    String checkInputNumbers;
    checkInputNumbers = val.substring(0, 3);
    int inputNumber = checkInputNumbers.toInt();

    if ( inputNumber > 100  ) {

      val.setCharAt( 0, '1' );
      val.setCharAt( 1, '0' );
      val.setCharAt( 2, '0' );
      lcd.setCursor(0, 1);
      lcd.print(val);
      lcd.setCursor(0, 1);
      place = -1;
    }

    String getint = val.substring(0, 2);
    number = val.toInt();
    Serial.println(number);

    place++;
  }
  else if ( inKey = 'S' ) {
    String str1 = val.substring( 0 , 3 );
    int x = str1.toInt();
    if ( x >= 0 && x <= 100 ) {
      EEPROMWriteint( 0 , x );
      lcd.clear();
      page10();
    }
    else {
      lcd.clear();
      page11();
    }
  }
}

void page7( char inKey ) {
  if ( inKey == 'E' ) {
    lcd.print("Min-Max Set");
    lcd.setCursor(0, 1);
    val = String( (minMax / 60000) );
    if ( val.length() < 2 ) {
      val = "0" + val;
    }
    val += "min";
    lcd.print(val);
  }

  else if ( isDigit( inKey )) {
    if ( place > 1) {
      place = 0;
    }

    val.setCharAt( (place) , inKey );
    Serial.println(val);
    lcd.setCursor( 0 , 1 );
    lcd.print(val);
    lcd.setCursor( place , 1 );
    lcd.blink();

    String getInt = val.substring(0, 2);
    number = val.toInt();
    Serial.println(number);

    place++;
  }
  else if ( inKey = 'S' ) {
    String str1 = val.substring( 0 , 2 );
    int x = str1.toInt();
    if ( x >= 0 && x <= 99 ) {
      EEPROMWriteint( 2 , x );
      lcd.clear();
      page10();
    }
    else {
      lcd.clear();
      page11();
    }
  }
}

void page8( char inKey ) {
  if ( inKey == 'E' ) {
    lcd.print("Max-Min Set");
    lcd.setCursor(0, 1);
    val = String( (maxMin / 60000) );
    if ( val.length() < 2 ) {
      val = "0" + val;
    }
    val += "min";
    lcd.print(val);
  }

  else if ( isDigit( inKey )) {
    if ( place > 1) {
      place = 0;
    }

    val.setCharAt( (place) , inKey );
    Serial.println(val);
    lcd.setCursor( 0 , 1 );
    lcd.print(val);
    lcd.setCursor( place , 1 );
    lcd.blink();

    String getInt = val.substring(0, 2);
    number = val.toInt();
    Serial.println(number);

    place++;
  }
  else if ( inKey = 'S' ) {
    String str1 = val.substring( 0 , 2 );
    int x = str1.toInt();
    if ( x >= 0 && x <= 99 ) {
      EEPROMWriteint( 4 , x );
      lcd.clear();
      page10();
    }
    else {
      lcd.clear();
      page11();
    }
  }
}

void page9() {
  lcd.print("Start Device");
  lcd.setCursor(0, 1);
  lcd.noBlink();
}

void page10() {
  lcd.print("Save Config");
  lcd.setCursor(0, 1);
  lcd.noBlink();
  delay(3000);
  bootup();
  delay(1000);
  lcd.clear();
  page = 1;
  page1( 'E' );
}

void page11() {
  lcd.print("ERROR");
  lcd.setCursor(0, 1);
  lcd.noBlink();
  delay(3000);
  bootup();
  delay(1000);
  lcd.clear();
  page = 1;
  page1( 'E' );
}



//This function will write a 2 byte (16bit) long to the eeprom at
//the specified address to address + 2.
void EEPROMWriteint(int address, int value)
{
  //Decomposition from a long to 2 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  //      byte two = ((value >> 16) & 0xFF);
  //      byte one = ((value >> 24) & 0xFF);

  //Write the 2 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  //      EEPROM.write(address + 2, two);
  //      EEPROM.write(address + 3, one);
}

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
int EEPROMReadint(int address)
{
  //Read the 4 bytes from the eeprom memory.
  //      int four = EEPROM.read(address);
  //      int three = EEPROM.read(address + 1);
  int two = EEPROM.read(address);
  int one = EEPROM.read(address + 1);

  //Return the recomposed long by using bitshift.
  return ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);
}

unsigned long EEPROMReadlong(int address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, unsigned long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

unsigned long eeprom_crc(void) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (int index = 0 ; index < (EEPROM.length() - 4)  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

