#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


//TODO: Specialized Menus
//TODO: RTC


const int PIN = 7;
const int SIZE = 16;
const int CHAR_WIDTH = 6;
const int MAX_OPTIONS = 10;
const int COLORS_AMOUNT = 8;

const int CLK_PIN = 2;
const int DT_PIN = 3;
const int SW_PIN_FOR_FALLING = 18;
const int SW_PIN_FOR_RISING = 19;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(SIZE, SIZE, PIN,
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
    LONGBUTTONPRESS
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

Color* colors[8];

class DisplayState;
class TurnedOffDisplay;
class NameDisplay;
class TextDisplay;
class AnimationDisplay;
class Menu;

class Display {
public:
    TurnedOffDisplay* turnedOffDisplay;
    NameDisplay* nameDisplay;
    TextDisplay* textDisplay;
    AnimationDisplay* animationDisplay;
    Menu* generalMenu;
    Display();
    void update();
    void shortButtonPress();
    void longButtonPress();
    void rotationRight();
    void rotationLeft();
    Color* getDefaultColor();
    int getDefaultColorIndex();
    void setDefaultColor(int);
    int getDefaultBrightness();
    void setDefaultBrightness(int);
private:
    friend class DisplayState;
    void changeState(DisplayState*);
    List* giveGeneralMenuOptions();
private:
    DisplayState* _state;
    int defaultColorIndex;
    Color* defaultColor;
    int defaultBrightness;
};

class DisplayState {
public:
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
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
};

class NameDisplay : public DisplayState {
public:
    NameDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
};

class TextDisplay : public DisplayState {
public:
    TextDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    void reset();
private:
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
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    void reset();
private:
    double startFaktor;
    double softCutOff;
    double hardCutOff;
};

class Option : public Element {
public:
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    virtual String getName(Display*);
    virtual String getValue(Display*);
};

class DefaultBrightness : public Option {
public:
    String name;
    DefaultBrightness();
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    virtual String getName(Display*);
    virtual String getValue(Display*);
};

class DefaultColor : public Option {
public:
    String name;
    DefaultColor();
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    virtual String getName(Display*);
    virtual String getValue(Display*);
};

class Menu : public DisplayState {
public:
    Menu(List* options);
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    void setPreviousDisplayState(DisplayState*);
    void reset();
private:
    List* options;
    int optionsIndex;
    DisplayState* previousDisplayState;
    bool selected;
    int xPosName;
    int xPosValue;
};

Display::Display() {
    matrix.begin();
    matrix.setTextSize(1);

    turnedOffDisplay = new TurnedOffDisplay();
    nameDisplay = new NameDisplay();
    textDisplay = new TextDisplay();
    animationDisplay = new AnimationDisplay();
    generalMenu = new Menu(giveGeneralMenuOptions());
    _state = nameDisplay;

    setDefaultColor(5);
    setDefaultBrightness(50);
}

void Display::update() {
    _state->update(this);
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

List* Display::giveGeneralMenuOptions() {
    List* options = new List();
    options->add(new DefaultBrightness());
    options->add(new DefaultColor());
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

Display* display;

void DisplayState::update(Display*) {}
void DisplayState::shortButtonPress(Display*) {}
void DisplayState::longButtonPress(Display*) {}
void DisplayState::rotationLeft(Display*) {}
void DisplayState::rotationRight(Display*) {}

void DisplayState::changeState(Display* d, DisplayState* s) {
    d->changeState(s);
}

void TurnedOffDisplay::update(Display*) {
    matrix.fillScreen(0);
    matrix.show();
}

void TurnedOffDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void TurnedOffDisplay::longButtonPress(Display* d) {}

void TurnedOffDisplay::rotationLeft(Display* d) {
    changeState(d, d->animationDisplay);
}

void TurnedOffDisplay::rotationRight(Display* d) {
    changeState(d, d->nameDisplay);
}

NameDisplay::NameDisplay() {}

void NameDisplay::update(Display*) {
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
    matrix.setCursor(0, 9);
    matrix.print("i");
    matrix.setCursor(5, 9);
    matrix.print("l");
    matrix.setCursor(10, 9);
    matrix.print("o");
    matrix.drawPixel(15, 2, 0);
    matrix.drawPixel(11, 4, 0);
    matrix.show();
}

void NameDisplay::shortButtonPress(Display* d) {
    d->generalMenu->setPreviousDisplayState(this);
    changeState(d, d->generalMenu);
}

void NameDisplay::longButtonPress(Display*) {}

void NameDisplay::rotationLeft(Display* d) {
    changeState(d, d->turnedOffDisplay);
}
void NameDisplay::rotationRight(Display* d) {
    changeState(d, d->textDisplay);
}

TextDisplay::TextDisplay() {
    xPos = 0;
    text = F(" Danke fuer Ihren Kauf bei Andi Start-Ups. Beehren Sie uns bald wieder <3.");
    tickCount = 0;
}

void TextDisplay::update(Display*) {
    tickCount += 1;
    if (tickCount < 5) {
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

void TextDisplay::longButtonPress(Display*) {}

void TextDisplay::rotationLeft(Display* d) {
    changeState(d, d->nameDisplay);
    reset();
}

void TextDisplay::rotationRight(Display* d) {
    changeState(d, d->animationDisplay);
    reset();
}

void TextDisplay::reset() {
    xPos = 0;
}

AnimationDisplay::AnimationDisplay() {
    startFaktor = 1.0;
    softCutOff = 0.4;
    hardCutOff = 0.2;
}

void AnimationDisplay::update(Display* display) {
    int faktoredRgb[3];
    matrix.fillScreen(0);
    double faktor = startFaktor;
    for (int i = 0; i < SIZE; i++) {
        if (faktor < hardCutOff)
            faktor = 1.0;
        if (faktor > softCutOff) {
            faktoredRgb[0] = display->getDefaultColor()->rgb[0] * faktor;
            faktoredRgb[1] = display->getDefaultColor()->rgb[1] * faktor;
            faktoredRgb[2] = display->getDefaultColor()->rgb[2] * faktor;
            uint16_t encodedColor = matrix.Color(faktoredRgb[0], faktoredRgb[1], faktoredRgb[2]);
            matrix.drawLine(i, 0, i, SIZE, encodedColor);
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
    changeState(d, d->textDisplay);
    reset();
}

void AnimationDisplay::rotationRight(Display* d) {
    changeState(d, d->turnedOffDisplay);
    reset();
}

void AnimationDisplay::reset() {
    startFaktor = 1.0;
}

void Option::rotationLeft(Display*) {}
void Option::rotationRight(Display*) {}

String Option::getName(Display*) {
    return F("not implemented");
}

String Option::getValue(Display*) {
    return F("not implemented");
}

DefaultBrightness::DefaultBrightness() {
    name = F("Helligkeit");
}

void DefaultBrightness::rotationLeft(Display* d) {
    int defaultBrightness = d->getDefaultBrightness();
    if (defaultBrightness - 5 < 10)
        return;
    d->setDefaultBrightness(defaultBrightness - 5);
}

void DefaultBrightness::rotationRight(Display* d) {
    int defaultBrightness = d->getDefaultBrightness();
    if (defaultBrightness + 5 > 255)
        return;
    d->setDefaultBrightness(defaultBrightness + 5);
}

String DefaultBrightness::getName(Display* d) {
    return name;
}

String DefaultBrightness::getValue(Display* d) {
    return String(d->getDefaultBrightness());
}

DefaultColor::DefaultColor() {
    name = F("Farbe");
}

void DefaultColor::rotationLeft(Display* d) {
    int defaultColorIndex = d->getDefaultColorIndex();
    if (defaultColorIndex > 0)
        d->setDefaultColor(defaultColorIndex - 1);
}

void DefaultColor::rotationRight(Display* d) {
    int defaultColorIndex = d->getDefaultColorIndex();
    if (defaultColorIndex < COLORS_AMOUNT - 1)
        d->setDefaultColor(defaultColorIndex + 1);
}

String DefaultColor::getName(Display* d) {
    return name;
}

String DefaultColor::getValue(Display* d) {
    return d->getDefaultColor()->name;
}

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

    matrix.setCursor(xPosName + 5, 0);
    option = options->valueAt(optionsIndex);
    String name = option->getName(d);
    matrix.print(name);
    xPosName -= 1;
    if (xPosName < (name.length()) * -CHAR_WIDTH + 2) {
        xPosName = 0;
    }

    matrix.setCursor(xPosValue + 5, 9);

    option = options->valueAt(optionsIndex);
    String value = option->getValue(d);
    matrix.print(value);
    xPosValue -= 1;
    if (xPosValue < (value.length()) * -CHAR_WIDTH + 2) {
        xPosValue = 0;
    }
    if (selected) {
        matrix.drawLine(0, 8, 15, 8, d->getDefaultColor()->getEncodedColor());
    }
    matrix.show();
    delay(200);
}

void Menu::shortButtonPress(Display* d) {
    if (selected)
        selected = false;
    else
        selected = true;
}

void Menu::longButtonPress(Display* d) {
    reset();
    changeState(d, previousDisplayState);
}

void Menu::rotationLeft(Display* d) {
    if (selected) {
        option = options->valueAt(optionsIndex);
        option->rotationLeft(d);
        xPosValue = 0;
    }
    else if (optionsIndex > 0) {
        optionsIndex -= 1;
        xPosName = 0;
        xPosValue = 0;
    }
}

void Menu::rotationRight(Display* d) {
    if (selected) {
        option = options->valueAt(optionsIndex);
        option->rotationRight(d);
        xPosValue = 0;
    }
    else if (optionsIndex < options->length() - 1) {
        optionsIndex += 1;
        xPosName = 0;
        xPosValue = 0;
    }
}

void Menu::reset() {
    selected = false;
    xPosName = 0;
    xPosValue = 0;
}

void Menu::setPreviousDisplayState(DisplayState* previousDisplayState) {
    this->previousDisplayState = previousDisplayState;
}

void sw_falling_interrupt() {
    startTimeButtonPressed = millis();
}

void sw_rising_interrupt() {
    if (events->length() != 0) {
        event = events->valueAt(events->length() - 1);
        if (event->eventEnum == SHORTBUTTONPRESS || event->eventEnum == LONGBUTTONPRESS)
            return;
    }
    if (millis() - startTimeButtonPressed > 300) {
        events->add(new Event(LONGBUTTONPRESS));
    } else {
        events->add(new Event(SHORTBUTTONPRESS));
    }
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

void setup() {
    colors[0] = new Color(F("rot"), 255, 0, 0);
    colors[1] = new Color(F("orange"), 255, 100, 0);
    colors[2] = new Color(F("gelb"), 255, 160, 0);
    colors[3] = new Color(F("gruen"), 0, 255, 0);
    colors[4] = new Color(F("blau"), 0, 0, 255);
    colors[5] = new Color(F("lila"), 190, 115, 150);
    colors[6] = new Color(F("pink"), 255, 80, 120);
    colors[7] = new Color(F("weiss"), 255, 255, 255);

    firstRotationState = TIMEOUT;
    lastRotationState = TIMEOUT;
    lastTimeRotationInterrupt = 0;
    startTimeButtonPressed = 0;

    attachInterrupt(digitalPinToInterrupt(CLK_PIN), clk_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(DT_PIN), dt_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(SW_PIN_FOR_FALLING), sw_falling_interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(SW_PIN_FOR_RISING), sw_rising_interrupt, RISING);

    display = new Display();
    events = new List();
}

void loop() {
    for (int i = 0; i < events->length(); i++) {
        event = events->valueAt(i);
        if (event->eventEnum == SHORTBUTTONPRESS) {
            display->shortButtonPress();
        }
        else if (event->eventEnum == LONGBUTTONPRESS) {
            display->longButtonPress();
        }
        else if (event->eventEnum == ROTATIONLEFT) {
            display->rotationLeft();
        }
        else if (event->eventEnum == ROTATIONRIGHT) {
            display->rotationRight();
        }
    }
    events->clear();

    display->update();
    delay(100);
}
