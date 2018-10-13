#include <FastLED.h>
#include <SPI.h> // SPI library included for SparkFunLSM9DS1
#include <Wire.h> // I2C library included for SparkFunLSM9DS1
//#include <SparkFunLSM9DS1.h> // SparkFun LSM9DS1 library <- this was used to try and get a fluid simulation but sensor was not very accurate
#include <Adafruit_NeoPixel.h>
#include <math.h>


#define LED_PIN     6     //Pin to go to first LED data in
#define NUM_LEDS    64
#define analogin A0       //Goes to potentiometer to adjust brightness
#define PIN_ENVELOPE A1   //Microphone input
int BRIGHTNESS;
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
const int button1Pin = 4; //button to change pattern
const int button2Pin = 2; //optional...used for second input to change pattern
int pattern = 0;
int patternSelect = 0;

//for "rain"
byte startVal[NUM_LEDS];
byte minHue =0;                 // Keeps the hue within a certain range - this is the lower limit
byte maxHue =30;                // Keeps the hue within a certain range - this is upper limit
byte maxBrightness = 200;       // Limits the brightness of the LEDs
byte brightness[NUM_LEDS];      // The brightness of each LED is controlled individually through this array
byte hue[NUM_LEDS];  
 


#define UPDATES_PER_SECOND 100


CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(64, LED_PIN, NEO_GRB + NEO_KHZ800);
int scale;


void setup() {
  delay(1000);
    scale = 1.0;
    Serial.begin(115200);
    strip.begin();
    strip.show();

    
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    pinMode(button1Pin, INPUT);
    pinMode(button2Pin, INPUT);
    
    currentPalette = RainbowColors_p;   //sets first start up pattern to rainbow
    currentBlending = LINEARBLEND;
} 


void loop()
{
    int button1State = digitalRead(button1Pin);
    int button2State = digitalRead(button2Pin);
    if(button1State == LOW || button2State == LOW){    //used to change the pattern of cube
      do{
        button1State = digitalRead(button1Pin);
        button2State = digitalRead(button2Pin);
        pattern = 1;
   
      }while(button1State==LOW || button2State == LOW);
    }
    
    if(pattern==1){   //changes pattern if button is pushed
      patternSelect++;
      pattern=0;
      if(patternSelect==13){
        patternSelect=0;
      }
      
    }
    
BRIGHTNESS=map(analogRead(analogin),0,1023,0,255);
    if(patternSelect > 2 || patternSelect == 0){/*Selection of patterns*/
    FastLED.setBrightness(  BRIGHTNESS );
    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
    }

    
    else if(patternSelect==2){/*Rain Effect*/
      for(int i = 0; i<NUM_LEDS; i++){
    
    //The brightness of each LED is based on the startValue, 
    //and has a sinusoidal pattern
    startVal[i]++;
    brightness[i] = sin8(startVal[i]);
    
    //Set the hue and brightness of each LED
    leds[i] = CHSV(hue[i], 255, map(brightness[i],0,255,0,maxBrightness));
    
    //This helps to further randomise the pattern
    if(random(1000)<100){
      startVal[i] = startVal[i]+2;
    }
    
    //The hue will only change when the LED is "off"
    if(brightness[i] <3){
      //Randomise the hue for the next on off cycle
      hue[i] = random(minHue, maxHue); 
      
      //Each time an LED turns off, the hue "range" is incremented.
      //This allows the LEDs to transition through each colour of the rainbow
      minHue++;
      maxHue++;
    }
  }
  
  FastLED.show();   // Show the next frame of the LED pattern
  delay(10);
    }

    
    else{/*audio*/
 int value;
  value = analogRead(PIN_ENVELOPE);
  
  for(int k=0; k<4; k++){    //sets brightness of cube based on intensity of sound
    for(int j=0; j<4; j++){
      for(int i=0; i<4; i++){
        double brightness = abs(1/(scale*k-3.0/2)*1/(scale*j-3.0/2)*1/(scale*i-3.0/2)*255.0/20860*BRIGHTNESS/10*(value));
        strip.setPixelColor(mapping(k,j,i), 0, 0, brightness);
      }
    }
  }
  strip.show();
    }
}
int mapping(int k, int j, int i){  //used to map led cube wiring pattern to a workable matrix
  if(j % 2==0){
   return k*16+j*4+i;
  }
  else {
    return k*16+(j+1)*4-(i+1);
    }
      
}

void FillLEDsFromPaletteColors( uint8_t colorIndex) //desplays selected pattern
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


void ChangePalettePeriodically()  //used to cycle through patterns
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

   
        if( patternSelect == 0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        //1 is ummited as this is the sound intensity pattern
        if( patternSelect == 2)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( patternSelect == 3)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( patternSelect == 4)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( patternSelect == 5)  { SetupChrisPalette();                      currentBlending = LINEARBLEND; }
        if( patternSelect == 6)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( patternSelect == 7)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( patternSelect == 8)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( patternSelect == 9)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( patternSelect == 10)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( patternSelect == 11) { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
        if( patternSelect == 12) { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND;} 
    
}


void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}


void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}


void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

void SetupRain()
{
    CRGB blue = CHSV( HUE_BLUE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   blue, black, black, black,
                                   black, black, black,black,
                                   black, black, black,black,
                                   black, black, black,black);
}

void SetupChrisPalette()
{
    CRGB red = CHSV( HUE_RED, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   red,   red,   green, green,
                                   green, green, red,   red,
                                   red,   red,   green, green,
                                   green, green, red,   red
                                    );
}


const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
