#ifndef epd-funcs-h
#define epd-funcs-h
#include <String.h>
#include <FS.h>
#include "Transition.h"
#include "TouchKey.h"
#include "RefreshType.h"



class MoonBadge {

    static const uint16_t input_buffer_pixels = 640; // may affect performance

    static const uint16_t max_row_width = 640; // for up to 7.5" display
    static const uint16_t max_palette_pixels = 256; // for depth <= 8

    uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
    uint8_t output_row_mono_buffer[max_row_width / 8]; // buffer for at least one row of b/w bits
    uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one row of color bits
    uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
    uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w



  public:
    RefreshType refresh;
    void init(void);
    void drawBitmapFromSpiffs(String filename, int16_t x, int16_t y, bool with_color);
    void print_text(String error,int x,int y);
    TouchKey getTouch();
    File openFile(String path,String modes);
    bool waitForTouchRelease(void);
};


void pathtest();

// Interrupt handlers for touch events.
void gotTouchUp();
void gotTouchDown();
void gotTouchLeft();
void gotTouchRight();
void resetTouch();
String key2str(TouchKey k);
TouchKey str2key(String s);

String getFilename(String absolute);

String getPath(String absolute);
bool isAbsolute(String path);
#endif
