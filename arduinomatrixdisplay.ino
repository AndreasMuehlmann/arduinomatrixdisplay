#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


int PIN = 7;
const int SIZE = 16;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
        NEO_MATRIX_TOP     + NEO_MATRIX_LEFT  +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
        NEO_GRB            + NEO_KHZ800);


// int rgb[] = {255, 0, 0}; // red
// int rgb[] = {255, 100, 0}; // orange
// int rgb[] = {255, 160, 0}; // yellow
// int rgb[] = {0, 255, 0}; // green
// int rgb[] = {0, 0, 255}; // blue
int rgb[] = {190, 115, 150}; // lite purple
// int rgb[] = {255, 80, 120}; // pink
// int rgb[] = {255, 255, 255}; // white
uint16_t color = matrix.Color(rgb[0], rgb[1], rgb[2]);
int mode = 1;

void setup() {
    Serial.begin(9600);
    matrix.begin();
    matrix.setBrightness(50);
    matrix.setTextColor(color);
    matrix.setTextSize(1);
}

void loop() {
    switch (mode) {
        case 0:
            mode0();
            break;
        case 1:
            mode1();
            break;
        case 2:
            mode2();
            break;
        default:
            break;
    }

}

void mode0() {
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
    delay(100);
}

int x_pos = 0;
int CHAR_WIDTH = 6;

void mode1() {
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
    delay(800);
}

double start_faktor = 1.0;
double soft_cut_off = 0.4;
double hard_cut_off = 0.2;

void mode2() {
    matrix.fillScreen(0);
    double faktor = start_faktor;
    for(int i = 0; i < SIZE; i++) {
        if (faktor < hard_cut_off)
            faktor = 1.0;
        if (faktor > soft_cut_off)
            matrix.drawLine(i, 0, i, SIZE, matrix.Color(rgb[0] * faktor, rgb[1] * faktor, rgb[2] * faktor));
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
