#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


const int PIN = 7;
const int SIZE = 16;
const int CHAR_WIDTH = 6;

struct Color {
   String name;
   int rgb[3];
};

Color colors[8];

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
        NEO_MATRIX_TOP     + NEO_MATRIX_LEFT  +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
        NEO_GRB            + NEO_KHZ800);

uint16_t rgbToEncodedColor(int rgb[3]) {
    return matrix.Color(rgb[0], rgb[1], rgb[2]);
}

class DisplayState;
class TurnedOffDisplay;
class NameDisplay;
class TextDisplay;
class AnimationDisplay;

class Display {
public:
    TurnedOffDisplay* turnedOffDisplay;
    NameDisplay* nameDisplay;
    TextDisplay* textDisplay;
    AnimationDisplay* animationDisplay;
    Display();
    void update();
    void shortButtonPress();
    void longButtonPress();
    void rotationRight();
    void rotationLeft();
    Color getDefaultColor();
    void setDefaultBrightness(int);
private:
    friend class DisplayState;
    void changeState(DisplayState*);
private:
    DisplayState* _state;
    Color defaultColor;
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
private:
    int x_pos;
};

class AnimationDisplay : public DisplayState {
public:
    AnimationDisplay();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
private:
    double start_faktor;
    double soft_cut_off;
    double hard_cut_off;
};

class Menu : public DisplayState {
public:
    Menu();
    virtual void update(Display*);
    virtual void shortButtonPress(Display*);
    virtual void longButtonPress(Display*);
    virtual void rotationLeft(Display*);
    virtual void rotationRight(Display*);
};

Display::Display() {
    matrix.begin();
    defaultColor = colors[5];
    defaultBrightness = 50;
    matrix.setBrightness(defaultBrightness);
    matrix.setTextColor(rgbToEncodedColor(defaultColor.rgb));
    matrix.setTextSize(1);

    turnedOffDisplay = new TurnedOffDisplay();
    nameDisplay = new NameDisplay();
    textDisplay = new TextDisplay();
    animationDisplay = new AnimationDisplay();
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

void Display::changeState(DisplayState* s) {
    _state = s;
}

Color Display::getDefaultColor() {
    return defaultColor;
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

void TurnedOffDisplay::shortButtonPress(Display*) {}
void TurnedOffDisplay::longButtonPress(Display*) {}

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

void NameDisplay::shortButtonPress(Display*) {}
void NameDisplay::longButtonPress(Display*) {}

void NameDisplay::rotationLeft(Display* d) {
    changeState(d, d->turnedOffDisplay);
}
void NameDisplay::rotationRight(Display* d) {
    changeState(d, d->textDisplay);
}

TextDisplay::TextDisplay() {
    x_pos = 0;
}

void TextDisplay::update(Display*) {
    matrix.setTextWrap(true);
    matrix.fillScreen(0);
    matrix.setCursor(x_pos, 0);
    String text = " Danke fuer Ihren Kauf bei Andi Start-Ups. Beehren Sie uns bald wieder <3.";
    matrix.print(text);
    matrix.show();
    x_pos -= CHAR_WIDTH;
    if (x_pos < (text.length() + 2) * -CHAR_WIDTH) {
        x_pos = 0;
    }
    delay(500);
}

void TextDisplay::shortButtonPress(Display*) {}
void TextDisplay::longButtonPress(Display*) {}

void TextDisplay::rotationLeft(Display* d) {
    changeState(d, d->nameDisplay);
}

void TextDisplay::rotationRight(Display* d) {
    changeState(d, d->animationDisplay);
}

AnimationDisplay::AnimationDisplay() {
    start_faktor = 1.0;
    soft_cut_off = 0.4;
    hard_cut_off = 0.2;
}

void AnimationDisplay::update(Display* display) {
    int faktored_rgb[3];
    matrix.fillScreen(0);
    double faktor = start_faktor;
    for (int i = 0; i < SIZE; i++) {
        if (faktor < hard_cut_off)
            faktor = 1.0;
        if (faktor > soft_cut_off) {
            faktored_rgb[0] = display->getDefaultColor().rgb[0] * faktor;
            faktored_rgb[1] = display->getDefaultColor().rgb[1] * faktor;
            faktored_rgb[2] = display->getDefaultColor().rgb[2] * faktor;
            matrix.drawLine(i, 0, i, SIZE, rgbToEncodedColor(faktored_rgb));
        }
        else
            matrix.drawLine(i, 0, i, SIZE, 0);
        faktor -= 1.0 / (double) SIZE;
    }
    start_faktor -= 1.0 / (double) SIZE;
    if (start_faktor < hard_cut_off)
        start_faktor = 1.0;
    matrix.show();
    delay(100);
}

void AnimationDisplay::shortButtonPress(Display*) {}
void AnimationDisplay::longButtonPress(Display*) {}

void AnimationDisplay::rotationLeft(Display* d) {
    changeState(d, d->textDisplay);
}

void AnimationDisplay::rotationRight(Display* d) {
    changeState(d, d->turnedOffDisplay);
}

void setup() {
    Serial.begin(9600);
    colors[0] = {"red", {255, 0, 0}};
    colors[1] = {"orange", {255, 100, 0}};
    colors[2] = {"yellow", {255, 160, 0}};
    colors[3] = {"green", {0, 255, 0}};
    colors[4] = {"blue", {0, 0, 255}};
    colors[5] = {"purple", {190, 115, 150}};
    colors[6] = {"pink", {255, 80, 120}};
    colors[7] = {"white", {255, 255, 255}};
    display = new Display();
}

void loop() {
    if (Serial.available() > 0) {
        String receivedString = Serial.readStringUntil('\n');
        char receivedChar = receivedString.charAt(0);
        if (receivedString.length() >= 2) {
            int receivedNumber = receivedString.toInt();
            display->setDefaultBrightness(receivedNumber);
        } else if (receivedChar == 's') {
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
