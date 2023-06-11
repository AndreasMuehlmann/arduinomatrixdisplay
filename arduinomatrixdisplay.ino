#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

extern unsigned int __bss_end;
extern void *__brkval;

int freeMemory() {
  int free_memory;
  if ((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);
  return free_memory;
}

const byte PIN = 7;
const byte SIZE = 16;
const byte CHAR_WIDTH = 6;
const byte MAX_OPTIONS = 10;
const byte COLORS_AMOUNT = 8;

class Option;

class OptionList {
public:
    OptionList();
    void add(Option*);
    Option* valueAt(int);
    int length();
private:
    Option* options[MAX_OPTIONS];
    int count;
};

OptionList::OptionList() {
    count = 0;
}

void OptionList::add(Option* option) {
    if (count > MAX_OPTIONS - 1)
        return;
    options[count] = option;
    count += 1;
}

Option* OptionList::valueAt(int index) {
    return options[index];
}

int OptionList::length() {
    return count;
}

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
        NEO_MATRIX_TOP     + NEO_MATRIX_LEFT  +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
        NEO_GRB            + NEO_KHZ800);

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
    OptionList* giveGeneralMenuOptions();
private:
    DisplayState* _state;
    int defaultColorIndex;
    Color* defaultColor;
    int defaultBrightness;
};

Display* display;

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

class Option {
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
    Menu(OptionList* options);
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
    void setPreviousDisplayState(DisplayState*);
    void reset();
private:
    OptionList* options;
    int optionsIndex;
    DisplayState* previousDisplayState;
    bool selected;
    int xPosName;
    int xPosValue;
};

Display::Display() {
    matrix.begin();
    defaultColorIndex = 5;
    defaultColor = colors[defaultColorIndex];
    defaultBrightness = 50;
    matrix.setBrightness(defaultBrightness);
    matrix.setTextColor(defaultColor->getEncodedColor());
    matrix.setTextSize(1);

    turnedOffDisplay = new TurnedOffDisplay();
    nameDisplay = new NameDisplay();
    textDisplay = new TextDisplay();
    animationDisplay = new AnimationDisplay();
    generalMenu = new Menu(giveGeneralMenuOptions());
    _state = nameDisplay;
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

OptionList* Display::giveGeneralMenuOptions() {
    OptionList* options = new OptionList();
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
    delay(1000);
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
    delay(500);
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
}

void TextDisplay::update(Display*) {
    matrix.setTextWrap(true);
    matrix.fillScreen(0);
    matrix.setCursor(xPos, 0);
    matrix.print(text);
    matrix.show();
    xPos -= CHAR_WIDTH;
    if (xPos < (text.length() + 2) * -CHAR_WIDTH) {
        xPos = 0;
    }
    delay(500);
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
    delay(100);
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

Menu::Menu(OptionList* options) {
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
    String name = options->valueAt(optionsIndex)->getName(d);
    matrix.print(name);
    xPosName -= 1;
    if (xPosName < (name.length()) * -CHAR_WIDTH + 2) {
        xPosName = 0;
    }

    matrix.setCursor(xPosValue + 5, 9);
    String value = options->valueAt(optionsIndex)->getValue(d);
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
    if (selected)
        options->valueAt(optionsIndex)->rotationLeft(d);
    else if (optionsIndex > 0) {
        optionsIndex -= 1;
        xPosName = 0;
        xPosValue = 0;
    }
}

void Menu::rotationRight(Display* d) {
    if (selected)
        options->valueAt(optionsIndex)->rotationRight(d);
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

void setup() {
    Serial.begin(9600);
    colors[0] = new Color(F("rot"), 255, 0, 0);
    colors[1] = new Color(F("orange"), 255, 100, 0);
    colors[2] = new Color(F("gelb"), 255, 160, 0);
    colors[3] = new Color(F("gruen"), 0, 255, 0);
    colors[4] = new Color(F("blau"), 0, 0, 255);
    colors[5] = new Color(F("lila"), 190, 115, 150);
    colors[6] = new Color(F("pink"), 255, 80, 120);
    colors[7] = new Color(F("weiss"), 255, 255, 255);
    display = new Display();
}

void loop() {
    int availableMemory = freeMemory();
    Serial.print(F("Free memory: "));
    Serial.println(availableMemory);
    if (Serial.available() > 0) {
        String receivedString = Serial.readStringUntil('\n');
        char receivedChar = receivedString.charAt(0);
        if (receivedChar == 's') {
            display->shortButtonPress();
        } else if (receivedChar == 'l') {
            display->longButtonPress();
        } else if (receivedChar == 'L') {
            display->rotationLeft();
        } else if (receivedChar == 'R') {
            display->rotationRight();
        }
    }
    display->update();
}
