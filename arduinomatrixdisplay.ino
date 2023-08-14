#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>


//TODO: personalized Texts

const int MATRIXDATAPIN = 7;
const int SIZE = 16;
const int CHAR_WIDTH = 6;
const int MAX_OPTIONS = 20;
const int COLORS_AMOUNT = 8;
const int AMOUNT_DISPLAY_STATES = 4;

const int DT_PIN = 2;
const int CLK_PIN = 3;
const int SW_PIN_FOR_FALLING = 18;
const int SW_PIN_FOR_RISING = 19;

bool allowSleeping = true;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(SIZE, SIZE, MATRIXDATAPIN,
        NEO_MATRIX_TOP     + NEO_MATRIX_LEFT  +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
        NEO_GRB            + NEO_KHZ800);

enum RotationInterruptStates {
  TIMEOUT,
  CLK,
  DT
};

volatile RotationInterruptStates firstRotationState;
volatile RotationInterruptStates lastRotationState;
volatile unsigned long lastTimeRotationInterrupt, startTimeButtonPressed;


class Element {};

enum EventEnum {
    ROTATIONLEFT,
    ROTATIONRIGHT,
    SHORTBUTTONPRESS,
    LONGBUTTONPRESS,
    VERYLONGBUTTONPRESS
};

class Event : public Element {
public:
    Event(EventEnum);
    EventEnum eventEnum;
};

Event::Event(EventEnum eventEnum) {
    this->eventEnum = eventEnum;
}

Event* event;

class Option;
Option* option;

class List {
public:
    List();
    void add(Element*);
    Element* valueAt(int);
    int length();
    void clear();
private:
    Element* elements[MAX_OPTIONS];
    int count;
};

List::List() {
    count = 0;
}

void List::add(Element* element) {
    if (count > MAX_OPTIONS - 1)
        return;
    elements[count] = element;
    count += 1;
}

Element* List::valueAt(int index) {
    return elements[index];
}

int List::length() {
    return count;
}

void List::clear() {
    count = 0;
}

volatile List* events;

class Color {
public:
    String name;
    int rgb[3];
    Color(String, int, int, int);
    uint16_t getEncodedColor();
    uint16_t getFaktoredEncodedColor(float);
};

Color::Color(String name, int r, int g, int b) {
    this->name = name;
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

uint16_t Color::getEncodedColor() {
    return matrix.Color(rgb[0], rgb[1], rgb[2]);
}

uint16_t Color::getFaktoredEncodedColor(float faktor) {
    return matrix.Color(rgb[0] * faktor, rgb[1] * faktor, rgb[2] * faktor);
}

Color* colors[8];

class NumberDrawer5By3 {
public:
    void drawNumbers5By3(String, int, int, uint16_t);
private:
    void drawZero(int, int, uint16_t);
    void drawOne(int, int, uint16_t);
    void drawTwo(int, int, uint16_t);
    void drawThree(int, int, uint16_t);
    void drawFour(int, int, uint16_t);
    void drawFive(int, int, uint16_t);
    void drawSix(int, int, uint16_t);
    void drawSeven(int, int, uint16_t);
    void drawEight(int, int, uint16_t);
    void drawNine(int, int, uint16_t);
};

void NumberDrawer5By3::drawNumbers5By3(String numbers, int x, int y, uint16_t color) {
    for (int i = 0; i < numbers.length(); i++) {
        char number = numbers.charAt(i);
        switch (number) {
            case '0':
                drawZero(x, y, color);
                break;
            case '1':
                drawOne(x, y, color);
                break;
            case '2':
                drawTwo(x, y, color);
                break;
            case '3':
                drawThree(x, y, color);
                break;
            case '4':
                drawFour(x, y, color);
                break;
            case '5':
                drawFive(x, y, color);
                break;
            case '6':
                drawSix(x, y, color);
                break;
            case '7':
                drawSeven(x, y, color);
                break;
            case '8':
                drawEight(x, y, color);
                break;
            case '9':
                drawNine(x, y, color);
                break;
            default:
                break;
        }
        x += 4;
    }
}

void NumberDrawer5By3::drawZero(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x, y + 4, color);
    matrix.drawLine(x + 2, y, x + 2, y + 4, color);
    matrix.drawPixel(x + 1, y, color);
    matrix.drawPixel(x + 1, y + 4, color);
}

void NumberDrawer5By3::drawOne(int x, int y, uint16_t color) {
    matrix.drawLine(x + 2, y, x + 2, y + 4, color);
}

void NumberDrawer5By3::drawTwo(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x, y + 2, x + 2, y + 2, color);
    matrix.drawLine(x, y + 4, x + 2, y + 4, color);
    matrix.drawPixel(x + 2, y + 1, color);
    matrix.drawPixel(x, y + 3, color);
}

void NumberDrawer5By3::drawThree(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x, y + 2, x + 2, y + 2, color);
    matrix.drawLine(x, y + 4, x + 2, y + 4, color);
    matrix.drawPixel(x + 2, y + 1, color);
    matrix.drawPixel(x + 2, y + 3, color);
}

void NumberDrawer5By3::drawFour(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x, y + 2, color);
    matrix.drawLine(x + 2, y, x + 2, y + 4, color);
    matrix.drawPixel(x + 1, y + 2, color);
}

void NumberDrawer5By3::drawFive(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x, y + 2, x + 2, y + 2, color);
    matrix.drawLine(x, y + 4, x + 2, y + 4, color);
    matrix.drawPixel(x, y + 1, color);
    matrix.drawPixel(x + 2, y + 3, color);
}

void NumberDrawer5By3::drawSix(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x, y, x, y + 4, color);
    matrix.drawLine(x + 2, y + 2, x + 2, y + 4, color);
    matrix.drawPixel(x + 1, y + 2, color);
    matrix.drawPixel(x + 1, y + 4, color);
}

void NumberDrawer5By3::drawSeven(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x + 2, y, x + 2, y + 4, color);

}

void NumberDrawer5By3::drawEight(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x, y + 2, x + 2, y + 2, color);
    matrix.drawLine(x, y + 4, x + 2, y + 4, color);
    matrix.drawLine(x, y, x, y + 4, color);
    matrix.drawLine(x + 2, y, x + 2, y + 4, color);
}

void NumberDrawer5By3::drawNine(int x, int y, uint16_t color) {
    matrix.drawLine(x, y, x + 2, y, color);
    matrix.drawLine(x, y + 2, x + 2, y + 2, color);
    matrix.drawLine(x, y + 4, x + 2, y + 4, color);
    matrix.drawLine(x + 2, y, x + 2, y + 4, color);
    matrix.drawPixel(x, y + 1, color);
}

class DisplayState;
class TurnedOffDisplay;
class NameDisplay;
class TimeDisplay;
class TextDisplay;
class AnimationDisplay;
class Menu;
class RealTimeClock;

class Display {
public:
    Menu* generalMenu;
    TimeDisplay* timeDisplay;
    Menu* timeDisplayMenu;
    TextDisplay* textDisplay;
    Menu* textDisplayMenu;
    TurnedOffDisplay* turnedOffDisplay;
    RealTimeClock* realTimeClock;
    DisplayState* displayStates[AMOUNT_DISPLAY_STATES];
    Display();
    void update();
    void shortButtonPress();
    void longButtonPress();
    void rotationRight();
    void rotationLeft();
    void drawNumbers5By3(String numbers, int x, int y, Color* color);
    Color* getDefaultColor();
    int getDefaultColorIndex();
    void setDefaultColor(int);
    int getDefaultBrightness();
    void setDefaultBrightness(int);
    int getTurnOffHour();
    void setTurnOffHour(int);
    int getTurnOnHour();
    void setTurnOnHour(int);
    void setDisplayState(int);
    void displayStateUp();
    void displayStateDown();
    void turnOff();
    void turnOn();
    bool isTurnedOff();
private:
    friend class DisplayState;
    void changeState(DisplayState*);
    List* giveGeneralMenuOptions();
    List* giveTimeDisplayMenuOptions();
    List* giveTextDisplayMenuOptions();
private:
    DisplayState* _state;
    int displayStateIndex;
    NumberDrawer5By3* numberDrawer5By3;
    int defaultColorIndex;
    Color* defaultColor;
    int defaultBrightness;
    int turnOffHour;
    int turnOnHour;
    bool turnedOff;
};

class DisplayState {
public:
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    virtual void reset(Display*);
protected:
    void changeState(Display*, DisplayState*);
};

class TurnedOffDisplay : public DisplayState {
public:
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    void reset(Display*);
private:
    bool turnedOff;
};

class NameDisplay : public DisplayState {
public:
    NameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
};

class TassiloNameDisplay : public DisplayState {
public:
    TassiloNameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
};

class JohannaNameDisplay : public DisplayState {
public:
    JohannaNameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
};

class LuisaNameDisplay : public DisplayState {
public:
    LuisaNameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
};

class StefanieNameDisplay : public DisplayState {
public:
    StefanieNameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
};

class AndiNameDisplay : public DisplayState {
public:
    AndiNameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
};

class TimeDisplay : public DisplayState {
public:
    TimeDisplay(Display*);
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    String addZeroToSingleDigit(String);
    void setColorIndex(Display*, int, int);
    int getColorIndex(int);
    Color* getColor(int);
private:
    RealTimeClock* realTimeClock;
    int x;
    int y;
    int hourToTwelve;
    int timeDisplayColorIndecis[6];
    Color* timeDisplayColors[6];
};

class TextDisplay : public DisplayState {
public:
    TextDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    void rotationLeft(Display*);
    void rotationRight(Display*);
    void reset(Display*);
    void setText(String text);
    void setTextSpeed(int);
    int getTextSpeed();
private:
    int textSpeed;
    int tickCount;
    int xPos;
    String text;
};

class AnimationDisplay : public DisplayState {
public:
    AnimationDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    void rotationLeft(Display*);
    void rotationRight(Display*);
    void reset(Display*);
private:
    double startFaktor;
    double softCutOff;
    double hardCutOff;
    double faktor;
};

class Option : public Element {
public:
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
};

class DefaultBrightness : public Option {
public:
    DefaultBrightness();
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
private:
    String name;
};

class DefaultColor : public Option {
public:
    DefaultColor();
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
private:
    String name;
};

class TurnOffHour : public Option {
public:
    TurnOffHour();
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
private:
    String name;
};

class TurnOnHour : public Option {
public:
    TurnOnHour();
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
private:
    String name;
};

class TimeDisplayColor : public Option {
public:
    TimeDisplayColor(String, int);
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
private:
    String name;
    int index;
};

class TimeDisplayValue : public Option {
public:
    TimeDisplayValue();
    void rotationLeft(Display*, DisplayState*);
    void rotationRight(Display*, DisplayState*);
    String getName(Display*, DisplayState*);
    String getValue(Display*, DisplayState*);
    void reset(Display*, DisplayState*);
    int giveIncrementedValue(int);
    virtual DateTime giveNewDateTime(DateTime, int);
    virtual int giveTimeValue(DateTime);
protected:
    String name;
    int increment;
    int maxValue;
    int minValue;
};

class TimeDisplayHour : public TimeDisplayValue {
public:
    TimeDisplayHour();
    DateTime giveNewDateTime(DateTime, int);
    int giveTimeValue(DateTime);
};

class TimeDisplayMinute : public TimeDisplayValue {
public:
    TimeDisplayMinute();
    DateTime giveNewDateTime(DateTime, int);
    int giveTimeValue(DateTime);
};

class TimeDisplaySecond : public TimeDisplayValue {
public:
    TimeDisplaySecond();
    DateTime giveNewDateTime(DateTime, int);
    int giveTimeValue(DateTime);
};

class TimeDisplayDay : public TimeDisplayValue {
public:
    TimeDisplayDay();
    DateTime giveNewDateTime(DateTime, int);
    int giveTimeValue(DateTime);
};

class TimeDisplayMonth : public TimeDisplayValue {
public:
    TimeDisplayMonth();
    DateTime giveNewDateTime(DateTime, int);
    int giveTimeValue(DateTime);
};

class TextSpeed : public Option {
public:
    TextSpeed();
    virtual void rotationLeft(Display*, DisplayState*);
    virtual void rotationRight(Display*, DisplayState*);
    virtual String getName(Display*, DisplayState*);
    virtual String getValue(Display*, DisplayState*);
    virtual void reset(Display*, DisplayState*);
private:
    String name;
};

class Menu : public DisplayState {
public:
    Menu(List* options);
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    void rotationLeft(Display*);
    void rotationRight(Display*);
    void setPreviousDisplayState(DisplayState*);
    void reset(Display*);
private:
    List* options;
    int optionsIndex;
    DisplayState* previousDisplayState;
    bool selected;
    int xPosName;
    int xPosValue;
};

class RealTimeClock {
public:
    RealTimeClock();
    void setTime(DateTime);
    DateTime getTime();
private:
    RTC_DS3231 rtc;
};

Display::Display() {
    matrix.begin();
    matrix.setTextSize(1);

    timeDisplay = new TimeDisplay(this);
    timeDisplayMenu = new Menu(giveTimeDisplayMenuOptions());
    textDisplay = new TextDisplay();
    textDisplayMenu = new Menu(giveTextDisplayMenuOptions());
    turnedOffDisplay = new TurnedOffDisplay();
    displayStateIndex = 0;
    displayStates[0] = new NameDisplay();
    displayStates[1] = timeDisplay;
    displayStates[2] = textDisplay;
    displayStates[3] = new AnimationDisplay();
    generalMenu = new Menu(giveGeneralMenuOptions());
    realTimeClock = new RealTimeClock();
    _state = displayStates[displayStateIndex];
    numberDrawer5By3 = new NumberDrawer5By3();

    setDefaultColor(4);
    setDefaultBrightness(10);

    turnOffHour = 21;
    turnOnHour = 10;

    turnedOff = false;
}

void Display::update() {
    DateTime now = realTimeClock->getTime();
    if (now.hour() == turnOffHour && now.minute() == 0  && now.second() < 15 && !isTurnedOff()) {
        turnOff();
    } else if (now.hour() == turnOnHour && now.minute() == 0 && now.second() < 15 && isTurnedOff()) {
        turnOn();
    }
    _state->update(this);
    delay(100);
}

void Display::shortButtonPress() {
    _state->shortButtonPress(this);
}

void Display::longButtonPress() {
    _state->longButtonPress(this);
}

void Display::rotationLeft() {
    _state->rotationLeft(this);
}

void Display::rotationRight() {
    _state->rotationRight(this);
}

void Display::drawNumbers5By3(String numbers, int x, int y, Color* color) {
    numberDrawer5By3->drawNumbers5By3(numbers, x, y, color->getEncodedColor());
}

List* Display::giveGeneralMenuOptions() {
    List* options = new List();
    options->add(new DefaultBrightness());
    options->add(new DefaultColor());
    options->add(new TurnOffHour());
    options->add(new TurnOnHour());
    return options;
}

List* Display::giveTimeDisplayMenuOptions() {
    List* options = new List();
    options->add(new TimeDisplayColor(F("Stunde Farbe"), 0));
    options->add(new TimeDisplayColor(F("Minute Farbe"), 1));
    options->add(new TimeDisplayColor(F("Sekunde Farbe"), 2));
    options->add(new TimeDisplayColor(F("Tag Farbe"), 3));
    options->add(new TimeDisplayColor(F("Monat Farbe"), 4));
    options->add(new TimeDisplayColor(F("Uhr Farbe"), 5));
    options->add(new TimeDisplayHour());
    options->add(new TimeDisplayMinute());
    options->add(new TimeDisplaySecond());
    options->add(new TimeDisplayDay());
    options->add(new TimeDisplayMonth());
    return options;
}

List* Display::giveTextDisplayMenuOptions() {
    List* options = new List();
    options->add(new TextSpeed());
    return options;
}

void Display::changeState(DisplayState* s) {
    _state = s;
}

int Display::getDefaultColorIndex() {
    return defaultColorIndex;
};

Color* Display::getDefaultColor() {
    return defaultColor;
};

void Display::setDefaultColor(int index) {
    defaultColorIndex = index;
    defaultColor = colors[index];
    matrix.setTextColor(defaultColor->getEncodedColor());
};

int Display::getDefaultBrightness() {
    return defaultBrightness;
};

void Display::setDefaultBrightness(int brightness) {
    defaultBrightness = brightness;
    matrix.setBrightness(defaultBrightness);
};

int Display::getTurnOffHour() {
    return turnOffHour; 
};

void Display::setTurnOffHour(int turnOffHour) {
    this->turnOffHour = turnOffHour; 
};

int Display::getTurnOnHour() {
    return turnOnHour; 
};

void Display::setTurnOnHour(int turnOnHour) {
    this->turnOnHour = turnOnHour; 
};

void Display::setDisplayState(int index) {
    displayStateIndex = index;
    _state->reset(this);
    _state = displayStates[displayStateIndex];
};

void Display::displayStateUp() {
    displayStateIndex += 1;
    if (displayStateIndex > AMOUNT_DISPLAY_STATES - 1)
        displayStateIndex = 0;
    _state->reset(this);
    _state = displayStates[displayStateIndex];
};

void Display::displayStateDown() {
    displayStateIndex -= 1;
    if (displayStateIndex < 0)
        displayStateIndex = AMOUNT_DISPLAY_STATES - 1;
    _state->reset(this);
    _state = displayStates[displayStateIndex];
};

void Display::turnOn() {
    if (!turnedOff)
        return;
    turnedOff = false;
    _state->reset(this);
    _state = displayStates[displayStateIndex];
}

void Display::turnOff() {
    if (turnedOff)
        return;
    turnedOff = true;
    _state->reset(this);
    _state = turnedOffDisplay;
}

bool Display::isTurnedOff() {
    return turnedOff;
}

Display* d;

void DisplayState::update(Display*) {}
void DisplayState::shortButtonPress(Display*) {}
void DisplayState::longButtonPress(Display*) {}

void DisplayState::rotationLeft(Display*) {
    d->displayStateDown(); 
}

void DisplayState::rotationRight(Display* d) {
    d->displayStateUp(); 
}

void DisplayState::changeState(Display* d, DisplayState* s) {
    d->changeState(s);
}

void DisplayState::reset(Display* d) {}


void TurnedOffDisplay::update(Display*) {
    if (!turnedOff) {
        matrix.fillScreen(0);
        matrix.show();
        turnedOff = true;
    }
    if (allowSleeping)
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void TurnedOffDisplay::shortButtonPress(Display* d) {}
void TurnedOffDisplay::longButtonPress(Display* d) {}
void TurnedOffDisplay::rotationLeft(Display*) {}
void TurnedOffDisplay::rotationRight(Display* d) {}

void TurnedOffDisplay::reset(Display* d) {
    turnedOff = false;
}

NameDisplay::NameDisplay() {}

void NameDisplay::update(Display*) {
    matrix.setTextWrap(true);
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print("Name");
    matrix.show();
}

void NameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void NameDisplay::longButtonPress(Display*) {}

TassiloNameDisplay::TassiloNameDisplay() {}

void TassiloNameDisplay::update(Display* d) {
    matrix.setTextWrap(false);
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print("T");
    matrix.setCursor(5, 0);
    matrix.print("a");

    matrix.setCursor(11, -2);
    matrix.print("s");
    matrix.setCursor(11, 0);
    matrix.print("s");
    matrix.drawPixel(15, 2, 0);
    matrix.drawPixel(11, 4, 0);

    matrix.setCursor(0, 9);
    matrix.print("i");

    matrix.drawLine(4, 10, 7, 15, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(7, 14, 2, 2, d->getDefaultColor()->getEncodedColor());

    matrix.setCursor(11, 6);
    matrix.print("o");
    matrix.drawLine(9, 12, 10, 11, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(9, 14, 10, 13, d->getDefaultColor()->getEncodedColor());
    matrix.show();
}

void TassiloNameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void TassiloNameDisplay::longButtonPress(Display*) {}

JohannaNameDisplay::JohannaNameDisplay() {}

void JohannaNameDisplay::update(Display*) {
    matrix.setTextWrap(false);
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print("J");

    matrix.setCursor(5, 0);
    matrix.print("o");
    matrix.drawLine(5, 2, 9, 2, 0);
    matrix.drawLine(5, 3, 10, 3, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(10, 4, 10, 3, d->getDefaultColor()->getEncodedColor());
    matrix.drawPixel(6, 1, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(8, 1, 8, 0, d->getDefaultColor()->getEncodedColor());

    matrix.setCursor(11, 0);
    matrix.print("h");
    matrix.setCursor(1, 6);
    matrix.print("a");
    matrix.setCursor(4, 11);
    matrix.print("n");
    matrix.setCursor(7, 6);
    matrix.print("n");
    matrix.setCursor(11, 9);
    matrix.print("a");

    matrix.show();
}

void JohannaNameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void JohannaNameDisplay::longButtonPress(Display*) {}

LuisaNameDisplay::LuisaNameDisplay() {}

void LuisaNameDisplay::update(Display*) {
    matrix.setTextWrap(false);
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print("L");
    matrix.setCursor(2, -2);
    matrix.print("u");
    matrix.setCursor(6, 3);
    matrix.print("i");
    matrix.setCursor(10, 4);
    matrix.print("s");
    matrix.setCursor(11, 9);
    matrix.print("a");

    matrix.drawLine(2, 13, 5, 10, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(4, 11, 2, 2, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(5, 12, 2, 2, d->getDefaultColor()->getEncodedColor());
    matrix.drawPixel(6, 14, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(3, 10, 2, 2, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(2, 9, 2, 2, d->getDefaultColor()->getEncodedColor());
    matrix.drawPixel(1, 9, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(2, 12, 3, 13, d->getDefaultColor()->getEncodedColor());

    matrix.drawLine(12, 5, 12, 1, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(10, 0, 2, 2, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(10, 3, 2, 2, d->getDefaultColor()->getEncodedColor());
    matrix.fillRect(13, 1, 2, 2, d->getDefaultColor()->getEncodedColor());

    matrix.show();
}

void LuisaNameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void LuisaNameDisplay::longButtonPress(Display*) {}

StefanieNameDisplay::StefanieNameDisplay() {}

void StefanieNameDisplay::update(Display*) {
    matrix.setTextWrap(false);
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print("S");
    
    matrix.drawLine(6, 7, 11, 7, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(10, 6, 10, 8, d->getDefaultColor()->getEncodedColor());
    matrix.drawPixel(5, 6, d->getDefaultColor()->getEncodedColor());
    matrix.drawPixel(6, 5, d->getDefaultColor()->getEncodedColor());

    matrix.setCursor(7, -2);
    matrix.print("e");
    matrix.setCursor(11, 0);
    matrix.print("f");

    matrix.setCursor(0, 9);
    matrix.print("a");
    
    matrix.drawLine(6, 13, 3, 10, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(4, 10, 5, 9, d->getDefaultColor()->getEncodedColor());
    matrix.drawLine(6, 9, 8, 11, d->getDefaultColor()->getEncodedColor());

    matrix.setCursor(11, 7);
    matrix.print("e");

    matrix.drawLine(10, 15, 13, 15, d->getDefaultColor()->getEncodedColor());
    matrix.drawPixel(15, 15, d->getDefaultColor()->getEncodedColor());

    matrix.fillRect(8, 13, 2, 2, d->getDefaultColor()->getEncodedColor());

    matrix.show();
}

void StefanieNameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void StefanieNameDisplay::longButtonPress(Display*) {}

AndiNameDisplay::AndiNameDisplay() {}

void AndiNameDisplay::update(Display*) {
    matrix.setTextWrap(false);
    matrix.fillScreen(0);
    matrix.setCursor(2, 0);
    matrix.print("An");
    matrix.setCursor(2, 9);
    matrix.print("di");
    matrix.show();
}

void AndiNameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void AndiNameDisplay::longButtonPress(Display*) {}

TimeDisplay::TimeDisplay(Display* d) {
    for (int i = 0; i < 6; i++) {
        setColorIndex(d, -1, i);
    }
}

void TimeDisplay::setColorIndex(Display* d, int colorIndex, int index) {
    timeDisplayColorIndecis[index] = colorIndex;
    if (colorIndex >= 0) {
        timeDisplayColors[index] = colors[colorIndex];
    } else {
        timeDisplayColors[index] = d->getDefaultColor();
    }
}

int TimeDisplay::getColorIndex(int index) {
    return timeDisplayColorIndecis[index];
}

Color* TimeDisplay::getColor(int index) {
    return timeDisplayColors[index];
}

void TimeDisplay::update(Display* d) {
    for (int i = 0; i < 6; i++) {
        setColorIndex(d, getColorIndex(i), i);
    }
    matrix.fillScreen(0);
    DateTime now = d->realTimeClock->getTime();
    
    d->drawNumbers5By3(addZeroToSingleDigit(String(now.hour())), 0, 0, timeDisplayColors[0]);
    d->drawNumbers5By3(addZeroToSingleDigit(String(now.minute())), 9, 0, timeDisplayColors[1]);
    d->drawNumbers5By3(addZeroToSingleDigit(String(now.second())), 9, 6, timeDisplayColors[2]);

    d->drawNumbers5By3(addZeroToSingleDigit(String(now.day())), 0, 11, timeDisplayColors[3]);
    matrix.drawPixel(7, 15, timeDisplayColors[3]->getEncodedColor());
    d->drawNumbers5By3(addZeroToSingleDigit(String(now.month())), 8, 11, timeDisplayColors[4]);
    matrix.drawPixel(15, 15, timeDisplayColors[4]->getEncodedColor());

    matrix.drawCircle(3, 8, 2, timeDisplayColors[5]->getEncodedColor());
    if (now.hour() > 12)
        hourToTwelve = now.hour() - 12;
    else
        hourToTwelve = now.hour();
    if (hourToTwelve == 12) {
        x = 0;
        y = 1;
    } else if (hourToTwelve == 1 || hourToTwelve == 2) {
        x = 1;
        y = 1;
    } else if (hourToTwelve == 3) {
        x = 1;
        y = 0;
    } else if (hourToTwelve == 4 || hourToTwelve == 5) {
        x = 1;
        y = -1;
    } else if (hourToTwelve == 6) {
        x = 0;
        y = -1;
    } else if (hourToTwelve == 7 || hourToTwelve == 8) {
        x = -1;
        y = -1;
    } else if (hourToTwelve == 9) {
        x = -1;
        y = 0;
    } else if (hourToTwelve == 10 || hourToTwelve == 11) {
        x = -1;
        y = 1;
    }

    matrix.drawLine(3, 8, 3 + x, 8 - y, timeDisplayColors[5]->getEncodedColor());
    matrix.show();
}

void TimeDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void TimeDisplay::longButtonPress(Display* d) {
    d->timeDisplayMenu->setPreviousDisplayState(this);
    changeState(d, d->timeDisplayMenu);
}

String TimeDisplay::addZeroToSingleDigit(String number) {
    if (number.length() == 1) {
        return "0" + number;
    }
    return number;
}

TextDisplay::TextDisplay() {
    xPos = 0;
    text = F(" Danke fuer Ihren Kauf bei Andi Start-Ups. Beehren Sie uns bald wieder <3.");
    tickCount = 0;
    textSpeed = 3;
}

void TextDisplay::update(Display*) {
    tickCount += 1;
    if (tickCount < textSpeed) {
        return;
    }
    tickCount = 0; 
    matrix.setTextWrap(true);
    matrix.fillScreen(0);
    matrix.setCursor(xPos, 0);
    matrix.print(text);
    matrix.show();
    xPos -= CHAR_WIDTH;
    if (xPos < (text.length() + 2) * -CHAR_WIDTH) {
        xPos = 0;
    }
}

void TextDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void TextDisplay::longButtonPress(Display* d) {
    d->textDisplayMenu->setPreviousDisplayState(this);
    changeState(d, d->textDisplayMenu);
}

void TextDisplay::rotationLeft(Display* d) {
    d->displayStateDown();
    reset(d);
}

void TextDisplay::rotationRight(Display* d) {
    d->displayStateUp();
    reset(d);
}

void TextDisplay::reset(Display*) {
    xPos = 0;
}

void TextDisplay::setText(String text) {
    this->text = text;
}

void TextDisplay::setTextSpeed(int textSpeed) {
    this->textSpeed = textSpeed;
}

int TextDisplay::getTextSpeed() {
    return textSpeed;
}

AnimationDisplay::AnimationDisplay() {
    startFaktor = 1.0;
    softCutOff = 0.4;
    hardCutOff = 0.2;
}

void AnimationDisplay::update(Display* d) {
    matrix.fillScreen(0);
    faktor = startFaktor;
    for (int i = 0; i < SIZE; i++) {
        if (faktor < hardCutOff)
            faktor = 1.0;
        if (faktor > softCutOff) {
            matrix.drawLine(i, 0, i, SIZE, d->getDefaultColor()->getFaktoredEncodedColor(faktor));
        }
        else
            matrix.drawLine(i, 0, i, SIZE, 0);
        faktor -= 1.0 / (double) SIZE;
    }
    startFaktor -= 1.0 / (double) SIZE;
    if (startFaktor < hardCutOff)
        startFaktor = 1.0;
    matrix.show();
}

void AnimationDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void AnimationDisplay::longButtonPress(Display*) {}

void AnimationDisplay::rotationLeft(Display* d) {
    d->displayStateDown();
    reset(d);
}

void AnimationDisplay::rotationRight(Display* d) {
    d->displayStateUp();
    reset(d);
}

void AnimationDisplay::reset(Display*) {
    startFaktor = 1.0;
}

void Option::rotationLeft(Display*, DisplayState*) {}
void Option::rotationRight(Display*, DisplayState*) {}

String Option::getName(Display*, DisplayState*) {
    return F("not implemented");
}

String Option::getValue(Display*, DisplayState*) {
    return F("not implemented");
}

DefaultBrightness::DefaultBrightness() {
    name = F("Helligkeit");
}

void DefaultBrightness::rotationLeft(Display* d, DisplayState*) {
    int defaultBrightness = d->getDefaultBrightness();
    if (defaultBrightness - 1 < 1)
        return;
    d->setDefaultBrightness(defaultBrightness - 1);
}

void DefaultBrightness::rotationRight(Display* d, DisplayState*) {
    int defaultBrightness = d->getDefaultBrightness();
    if (defaultBrightness + 1 > 255)
        return;
    d->setDefaultBrightness(defaultBrightness + 1);
}

String DefaultBrightness::getName(Display* d, DisplayState*) {
    return name;
}

String DefaultBrightness::getValue(Display* d, DisplayState*) {
    return String(d->getDefaultBrightness());
}

void DefaultBrightness::reset(Display*, DisplayState*) {}

DefaultColor::DefaultColor() {
    name = F("Farbe");
}

void DefaultColor::rotationLeft(Display* d, DisplayState*) {
    int defaultColorIndex = d->getDefaultColorIndex();
    if (defaultColorIndex > 0)
        d->setDefaultColor(defaultColorIndex - 1);
}

void DefaultColor::rotationRight(Display* d, DisplayState*) {
    int defaultColorIndex = d->getDefaultColorIndex();
    if (defaultColorIndex < COLORS_AMOUNT - 1)
        d->setDefaultColor(defaultColorIndex + 1);
}

String DefaultColor::getName(Display* d, DisplayState*) {
    return name;
}

String DefaultColor::getValue(Display* d, DisplayState*) {
    return d->getDefaultColor()->name;
}

void DefaultColor::reset(Display*, DisplayState*) {}


TurnOffHour::TurnOffHour() {
    name = F("Abschaltzeit");
}

void TurnOffHour::rotationLeft(Display* d, DisplayState*) {
    int turnOffHour = d->getTurnOffHour();
    if (turnOffHour - 1 < -1)
        return;
    d->setTurnOffHour(turnOffHour - 1);
}

void TurnOffHour::rotationRight(Display* d, DisplayState*) {
    int turnOffHour = d->getTurnOffHour();
    if (turnOffHour + 1 > 24)
        return;
    d->setTurnOffHour(turnOffHour + 1);
}

String TurnOffHour::getName(Display* d, DisplayState*) {
    return name;
}

String TurnOffHour::getValue(Display* d, DisplayState*) {
    return String(d->getTurnOffHour());
}

void TurnOffHour::reset(Display*, DisplayState*) {}

TurnOnHour::TurnOnHour() {
    name = F("Anschaltzeit");
}

void TurnOnHour::rotationLeft(Display* d, DisplayState*) {
    int turnOnHour = d->getTurnOnHour();
    if (turnOnHour - 1 < -1)
        return;
    d->setTurnOnHour(turnOnHour - 1);
}

void TurnOnHour::rotationRight(Display* d, DisplayState*) {
    int turnOnHour = d->getTurnOnHour();
    if (turnOnHour + 1 > 24)
        return;
    d->setTurnOnHour(turnOnHour + 1);
}

String TurnOnHour::getName(Display* d, DisplayState*) {
    return name;
}

String TurnOnHour::getValue(Display* d, DisplayState*) {
    return String(d->getTurnOnHour());
}

void TurnOnHour::reset(Display*, DisplayState*) {}

TimeDisplayColor::TimeDisplayColor(String name, int index) {
    this->name = name;
    this->index = index;
}

void TimeDisplayColor::rotationLeft(Display* d, DisplayState*) {
    int colorIndex = d->timeDisplay->getColorIndex(index);
    if (colorIndex >= 0)
        d->timeDisplay->setColorIndex(d, colorIndex - 1, index);
}

void TimeDisplayColor::rotationRight(Display* d, DisplayState*) {
    int colorIndex = d->timeDisplay->getColorIndex(index);
    if (colorIndex < COLORS_AMOUNT - 1)
        d->timeDisplay->setColorIndex(d, colorIndex + 1, index);
}

String TimeDisplayColor::getName(Display* d, DisplayState*) {
    return name;
}

String TimeDisplayColor::getValue(Display* d, DisplayState*) {
    if (d->timeDisplay->getColorIndex(index) == -1)
        return "Standard";
    return d->timeDisplay->getColor(index)->name;
}

void TimeDisplayColor::reset(Display*, DisplayState*) {}

TimeDisplayValue::TimeDisplayValue() {
    name = "Value";
    increment = 0;
    maxValue = 1;
    minValue = 0;
}

void TimeDisplayValue::rotationLeft(Display*, DisplayState*) {
    if (increment <= -maxValue + minValue)
        return ;
    increment -= 1;
}

void TimeDisplayValue::rotationRight(Display*, DisplayState*) {
    if (increment >= maxValue - minValue)
        return;
    increment += 1;
}

String TimeDisplayValue::getName(Display*, DisplayState*) {
    return name;
}

String TimeDisplayValue::getValue(Display* d, DisplayState*) {
    DateTime now = d->realTimeClock->getTime();
    return String(giveIncrementedValue(giveTimeValue(now)));
}

void TimeDisplayValue::reset(Display* d, DisplayState*) {
    if (increment == 0)
        return;
    DateTime now = d->realTimeClock->getTime();
    DateTime newDateTime = giveNewDateTime(now, giveIncrementedValue(giveTimeValue(now)));
    d->realTimeClock->setTime(newDateTime);
    increment = 0;
}

int TimeDisplayValue::giveTimeValue(DateTime now) {
    return 0;
}

int TimeDisplayValue::giveIncrementedValue(int current) {
    int incrementedValue = current + increment;
    if (incrementedValue > maxValue)
        incrementedValue = minValue + (incrementedValue - maxValue);
    else if (incrementedValue < minValue)
        incrementedValue = maxValue + (incrementedValue - minValue + 1);
    return incrementedValue;
}

DateTime TimeDisplayValue::giveNewDateTime(DateTime now, int incrementedValue) {
    DateTime newDateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    return newDateTime;
}

TimeDisplayHour::TimeDisplayHour() {
    name = "Stunde";
    increment = 0;
    maxValue = 23;
    minValue = 0;
}

DateTime TimeDisplayHour::giveNewDateTime(DateTime now, int incrementedValue) {
    DateTime newDateTime(now.year(), now.month(), now.day(), incrementedValue, now.minute(), now.second());
    return newDateTime;
}

int TimeDisplayHour::giveTimeValue(DateTime now) {
    return now.hour();
}

TimeDisplayMinute::TimeDisplayMinute() {
    name = "Minute";
    increment = 0;
    maxValue = 59;
    minValue = 0;
}

DateTime TimeDisplayMinute::giveNewDateTime(DateTime now, int incrementedValue) {
    DateTime newDateTime(now.year(), now.month(), now.day(), now.hour(), incrementedValue, now.second());
    return newDateTime;
}

int TimeDisplayMinute::giveTimeValue(DateTime now) {
    return now.minute();
}

TimeDisplaySecond::TimeDisplaySecond() {
    name = "Sekunde";
    increment = 0;
    maxValue = 59;
    minValue = 0;
}

DateTime TimeDisplaySecond::giveNewDateTime(DateTime now, int incrementedValue) {
    DateTime newDateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), incrementedValue);
    return newDateTime;
}

int TimeDisplaySecond::giveTimeValue(DateTime now) {
    return now.second();
}

TimeDisplayDay::TimeDisplayDay() {
    name = "Tag";
    increment = 0;
    maxValue = 31;
    minValue = 1;
}

DateTime TimeDisplayDay::giveNewDateTime(DateTime now, int incrementedValue) {
    DateTime newDateTime(now.year(), now.month(), incrementedValue, now.hour(), now.minute(), now.second());
    return newDateTime;
}

int TimeDisplayDay::giveTimeValue(DateTime now) {
    return now.day();
}

TimeDisplayMonth::TimeDisplayMonth() {
    name = "Monat";
    increment = 0;
    maxValue = 12;
    minValue = 1;
}

DateTime TimeDisplayMonth::giveNewDateTime(DateTime now, int incrementedValue) {
    DateTime newDateTime(now.year(), incrementedValue, now.month(), now.hour(), now.minute(), now.second());
    return newDateTime;
}

int TimeDisplayMonth::giveTimeValue(DateTime now) {
    return now.month();
}

TextSpeed::TextSpeed() {
    name = F("Geschwindigkeit Text");
}

void TextSpeed::rotationLeft(Display* d, DisplayState*) {
    int textSpeed = d->textDisplay->getTextSpeed();
    if (textSpeed + 1 > 10)
        return;
    d->textDisplay->setTextSpeed(textSpeed + 1);
}

void TextSpeed::rotationRight(Display* d, DisplayState*) {
    int textSpeed = d->textDisplay->getTextSpeed();
    if (textSpeed - 1 < 1)
        return;
    d->textDisplay->setTextSpeed(textSpeed - 1);
}

String TextSpeed::getName(Display* d, DisplayState*) {
    return name;
}

String TextSpeed::getValue(Display* d, DisplayState*) {
    return String(11 - d->textDisplay->getTextSpeed());
}

void TextSpeed::reset(Display*, DisplayState*) {}

Menu::Menu(List* options) {
    this->options = options;
    optionsIndex = 0;
    selected = false;
    xPosName = 0;
    xPosValue = 0;
}

void Menu::update(Display* d) {
    matrix.setTextWrap(false);
    matrix.fillScreen(0);
    if (selected) {
        matrix.fillRect(0, 0, 16, 8, d->getDefaultColor()->getFaktoredEncodedColor(0.5));
    }

    matrix.setCursor(xPosName + 5, 0);
    option = options->valueAt(optionsIndex);
    String name = option->getName(d, previousDisplayState);
    matrix.print(name);
    xPosName -= 1;
    if (xPosName < (name.length()) * -CHAR_WIDTH + 2) {
        xPosName = 0;
    }

    matrix.setCursor(xPosValue + 5, 8);

    option = options->valueAt(optionsIndex);
    String value = option->getValue(d, previousDisplayState);
    matrix.print(value);
    xPosValue -= 1;
    if (xPosValue < (value.length()) * -CHAR_WIDTH + 2) {
        xPosValue = 0;
    }
    matrix.show();
}

void Menu::shortButtonPress(Display* d) {
    if (selected)
        selected = false;
    else
        selected = true;
}

void Menu::longButtonPress(Display* d) {
    option = options->valueAt(optionsIndex);
    option->reset(d, previousDisplayState);
    reset(d);
    previousDisplayState->reset(d);
    changeState(d, previousDisplayState);
}

void Menu::rotationLeft(Display* d) {
    if (selected) {
        option = options->valueAt(optionsIndex);
        option->rotationLeft(d, previousDisplayState);
        xPosValue = 0;
    }
    else if (optionsIndex > 0) {
        option = options->valueAt(optionsIndex);
        option->reset(d, previousDisplayState);
        optionsIndex -= 1;
        xPosName = 0;
        xPosValue = 0;
    }
}

void Menu::rotationRight(Display* d) {
    if (selected) {
        option = options->valueAt(optionsIndex);
        option->rotationRight(d, previousDisplayState);
        xPosValue = 0;
    }
    else if (optionsIndex < options->length() - 1) {
        option = options->valueAt(optionsIndex);
        option->reset(d, previousDisplayState);
        optionsIndex += 1;
        xPosName = 0;
        xPosValue = 0;
    }
}

void Menu::reset(Display*) {
    selected = false;
    xPosName = 0;
    xPosValue = 0;
}

void Menu::setPreviousDisplayState(DisplayState* previousDisplayState) {
    this->previousDisplayState = previousDisplayState;
}

RealTimeClock::RealTimeClock() {
    Wire.begin();
    if (!rtc.begin()) {
        // Serial.println("RTC nicht gefunden!");
    }
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void RealTimeClock::setTime(DateTime dateTime) {
    rtc.adjust(dateTime);
}

DateTime RealTimeClock::getTime() {
    return rtc.now();
}
void sw_falling_interrupt() {
    startTimeButtonPressed = millis();
    allowSleeping = false;
}

void sw_rising_interrupt() {
    if (events->length() != 0) {
        event = events->valueAt(events->length() - 1);
        if (event->eventEnum == SHORTBUTTONPRESS || event->eventEnum == LONGBUTTONPRESS)
            return;
    }
    if (millis() - startTimeButtonPressed > 3000) {
        events->add(new Event(VERYLONGBUTTONPRESS));
    } else if (millis() - startTimeButtonPressed > 300) {
        events->add(new Event(LONGBUTTONPRESS));
    } else {
        events->add(new Event(SHORTBUTTONPRESS));
    }
    allowSleeping = true;
}

void clk_interrupt() {
    if (millis() - lastTimeRotationInterrupt > 10) {
        firstRotationState = TIMEOUT;
        lastRotationState = TIMEOUT;
    }
    if (firstRotationState == TIMEOUT) {
        firstRotationState = CLK;
    }
    if (lastRotationState == CLK) {
        lastTimeRotationInterrupt = millis();
        return;
    }
    if (lastRotationState == DT && firstRotationState == DT) {
        events->add(new Event(ROTATIONLEFT));
    }
    lastRotationState = CLK;
    lastTimeRotationInterrupt = millis();
}

void dt_interrupt() {
    if (millis() - lastTimeRotationInterrupt > 10) {
        firstRotationState = TIMEOUT;
        lastRotationState = TIMEOUT;
    }
    if (firstRotationState == TIMEOUT) {
        firstRotationState = DT;
    }
    if (lastRotationState == DT) {
        lastTimeRotationInterrupt = millis();
        return;
    }
    if (lastRotationState == CLK && firstRotationState == CLK) {
        events->add(new Event(ROTATIONRIGHT));
    }
    lastRotationState = DT;
    lastTimeRotationInterrupt = millis();
}

void tassiloSetup(Display* d) {
    d->displayStates[0] = new TassiloNameDisplay();
    d->setDisplayState(0);
    d->setDefaultColor(1);
    d->textDisplay->setText("I'm a Pfadfinder!");
}

void johannaSetup(Display* d) {
    d->displayStates[0] = new JohannaNameDisplay();
    d->setDisplayState(0);
    d->setDefaultColor(6);
    d->textDisplay->setText("I live in a one square meter room!");
}

void luisaSetup(Display* d) {
    d->displayStates[0] = new LuisaNameDisplay();
    d->setDisplayState(0);
    d->setDefaultColor(5);
    d->textDisplay->setText("I like nature!");
}

void stefanieSetup(Display* d) {
    d->displayStates[0] = new StefanieNameDisplay();
    d->setDisplayState(0);
    d->setDefaultColor(2);
    d->textDisplay->setText("Yeaah goal!!!!");
}

void andiSetup(Display* d) {
    d->displayStates[0] = new AndiNameDisplay();
    d->setDisplayState(0);
    d->setDefaultColor(4);
}

void setup() {
    //Serial.begin(9600);

    firstRotationState = TIMEOUT;
    lastRotationState = TIMEOUT;
    lastTimeRotationInterrupt = 0;
    startTimeButtonPressed = 0;

    attachInterrupt(digitalPinToInterrupt(SW_PIN_FOR_FALLING), sw_falling_interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(SW_PIN_FOR_RISING), sw_rising_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_PIN), clk_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(DT_PIN), dt_interrupt, RISING);

    events = new List();

    colors[0] = new Color(F("rot"), 255, 0, 0);
    colors[1] = new Color(F("orange"), 255, 100, 0);
    colors[2] = new Color(F("gelb"), 255, 160, 0);
    colors[3] = new Color(F("gruen"), 0, 255, 0);
    colors[4] = new Color(F("blau"), 0, 0, 255);
    colors[5] = new Color(F("lila"), 255, 0, 255);
    colors[6] = new Color(F("pink"), 255, 0, 160);
    colors[7] = new Color(F("weiss"), 255, 255, 255);

    d = new Display();
    // tassiloSetup(d);
    johannaSetup(d);
    // luisaSetup(d);
    // stefanieSetup(d);
    // andiSetup(d);
}

void loop() {
    for (int i = 0; i < events->length(); i++) {
        event = events->valueAt(i);
        if (event->eventEnum == SHORTBUTTONPRESS) {
            d->shortButtonPress();
        }
        else if (event->eventEnum == LONGBUTTONPRESS) {
            d->longButtonPress();
        }
        else if (event->eventEnum == VERYLONGBUTTONPRESS) {
            if (d->isTurnedOff()) {
                d->turnOn();
            } else {
                d->turnOff();
            }
            break;
        }
        else if (event->eventEnum == ROTATIONLEFT) {
            d->rotationLeft();
        }
        else if (event->eventEnum == ROTATIONRIGHT) {
            d->rotationRight();
        }
    }
    events->clear();

    d->update();
}
