#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


int PIN = 7;
const int SIZE = 16;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
        NEO_MATRIX_TOP     + NEO_MATRIX_LEFT  +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
        NEO_GRB            + NEO_KHZ800);


int rgb[] = {192, 115, 107};
uint16_t color = matrix.Color(rgb[0], rgb[1], rgb[2]);
int mode = 2;

void setup() {
    Serial.begin(9600);
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(255);
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
    matrix.setPixelColor(0, color);
}

void mode1() {
    /*
       int x = matrix.width();
       int pass = 0;

       matrix.fillScreen(0);
       matrix.setCursor(x, 0);
       matrix.print(F("AZ-Delivery"));
       if(--x < -72) {
       x = matrix.width();
       if(++pass >= 3) pass = 0;
       matrix.setTextColor(colors[pass]);
       }
       matrix.show();
     */
}

double start_faktor = 1.0;

void mode2() {
    matrix.fillScreen(0);
    double faktor = start_faktor;
    for(int i = 0; i < SIZE; i++) {
        matrix.drawLine(i, 0, i, SIZE, matrix.Color(rgb[0] * faktor, rgb[1] * faktor, rgb[2] * faktor));
        faktor -= 1.0 / (double) SIZE;
        if (faktor < 0)
          faktor = 1.0;
    }
    start_faktor -= 1.0 / (double) SIZE;
    if (start_faktor < 0) {
      start_faktor = 1.0;
    }
    matrix.show();
    delay(100);
}
