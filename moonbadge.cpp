#include "moonbadge.h"
#include "StringSplitter.h"
GxEPD2_BW<GxEPD2_213, GxEPD2_213::HEIGHT> display(GxEPD2_213(/*CS=*/ 5, /*DC=*/ 25, /*RST=*/ 16, /*BUSY=*/ 4)); // GDE0213B1, phased out

#define FileClass fs::File

// function declaration with default parameter
void drawBitmapFromSpiffs(const char *filename, int16_t x, int16_t y, bool with_color = true);

uint16_t read16(fs::File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
/*
   t5 up
   t6 down
   t4 r
   t7 l
*/
int threshold = 30;
void MoonBadge::init() {
  display.init();

  SPIFFS.begin();
  Serial.println("SPIFFS started");
  if (!SD.begin(15)) {
    Serial.println("Card Mount Failed");
  } else {
    Serial.println("SDMMC started");
  }
  touchAttachInterrupt(T5, gotTouchUp, threshold);
  touchAttachInterrupt(T6, gotTouchDown, threshold);
  touchAttachInterrupt(T7, gotTouchLeft, threshold);
  touchAttachInterrupt(T4, gotTouchRight, threshold);

}
int pad_num_touched;
void gotTouchUp() {
  //Serial.println("Touch Up");
  pad_num_touched = 1;
}
void gotTouchDown() {
  //Serial.println("Touch Down");
  pad_num_touched = 2;
}
void gotTouchLeft() {
  //Serial.println("Touch Left");
  pad_num_touched = 3;
}
void gotTouchRight() {
  //Serial.println("Touch Right");
  pad_num_touched = 4;
}
void resetTouch() {
  pad_num_touched = 0;
}

void MoonBadge::print_text(String error, int x, int y) {
  display.setRotation(0);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(error);

  }
  while (display.nextPage());
  delay(5000);
}

int MoonBadge::getTouch() {
  int n =  pad_num_touched;
  resetTouch();
  return n;
}

bool MoonBadge::waitForTouchRelease() {
  int up, down, left, right;
  int timeout = 5000;
  while (true) {
    up = touchRead(T5);
    down = touchRead(T6);
    left = touchRead(T4);
    right = touchRead(T7);
    if ((up > threshold && down > threshold && left > threshold && right > threshold) | timeout <= 0) break;
    delay(50);
    timeout -= 1;
  }
}

File MoonBadge::openFile(String path, String modes) {
  fs::File file;
  // SPIFFS or SD?
  StringSplitter *splitter = new StringSplitter(path, ':', 5);
  int itemCount = splitter->getItemCount();
  for (int i = 0; i < itemCount; i++) {
    Serial.print("\t");
    Serial.print(i);
    Serial.print(":");
    Serial.println(splitter->getItemAtIndex(i));
  }
  if (itemCount != 1) {
    String drive = splitter->getItemAtIndex(0);
    String drive_path = splitter->getItemAtIndex(1);
    Serial.print("Drive: \t'");Serial.print(drive);Serial.println("'");
    Serial.print("Path: \t'");Serial.print(drive_path);Serial.println("'");
    if (drive == "SD") {
      file = SD.open(drive_path, "r");
    } else if (drive == "SPI") {
      file = SPIFFS.open(drive_path, "r");
    }
  } else {
    file = SPIFFS.open(path, "r");
  }

  String fpath = path;

  return file;
}

void MoonBadge::drawBitmapFromSpiffs(String filename, int16_t x, int16_t y, bool with_color)
{
  fs::File file;
  bool valid = false; // valid format to be handled
  bool flip = true; // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display.width()) || (y >= display.height())) return;
  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  file = openFile(filename, "r");

  if (!file)
  {
    Serial.print("File not found");
    return;
  }
  // Parse BMP header
  if (read16(file) == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width  = read32(file);
    uint32_t height = read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) && ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      Serial.print("File size: "); Serial.println(fileSize);
      Serial.print("Image Offset: "); Serial.println(imageOffset);
      Serial.print("Header size: "); Serial.println(headerSize);
      Serial.print("Bit Depth: "); Serial.println(depth);
      Serial.print("Image size: ");
      Serial.print(width);
      Serial.print('x');
      Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8) rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0)
      {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display.width())  w = display.width()  - x;
      if ((y + h - 1) >= display.height()) h = display.height() - y;
      if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish, colored;
        if (depth == 1) with_color = false;
        if (depth <= 8)
        {
          if (depth < 8) bitmask >>= depth;
          file.seek(54); //palette is always @ 54
          for (uint16_t pn = 0; pn < (1 << depth); pn++)
          {
            blue  = file.read();
            green = file.read();
            red   = file.read();
            file.read();
            whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8) mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8) color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
          }
        }
        display.clearScreen();
        uint32_t rowPosition = flip ? imageOffset + (height - h) * rowSize : imageOffset;
        for (uint16_t row = 0; row < h; row++, rowPosition += rowSize) // for each line
        {
          uint32_t in_remain = rowSize;
          uint32_t in_idx = 0;
          uint32_t in_bytes = 0;
          uint8_t in_byte = 0; // for depth <= 8
          uint8_t in_bits = 0; // for depth <= 8
          uint8_t out_byte = 0xFF; // white (for w%8!=0 boarder)
          uint8_t out_color_byte = 0xFF; // white (for w%8!=0 boarder)
          uint32_t out_idx = 0;
          file.seek(rowPosition);
          for (uint16_t col = 0; col < w; col++) // for each pixel
          {
            // Time to read more pixel data?
            if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS multiple of 3)
            {
              in_bytes = file.read(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
              in_remain -= in_bytes;
              in_idx = 0;
            }
            switch (depth)
            {
              case 24:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 16:
                {
                  uint8_t lsb = input_buffer[in_idx++];
                  uint8_t msb = input_buffer[in_idx++];
                  if (format == 0) // 555
                  {
                    blue  = (lsb & 0x1F) << 3;
                    green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                    red   = (msb & 0x7C) << 1;
                  }
                  else // 565
                  {
                    blue  = (lsb & 0x1F) << 3;
                    green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                    red   = (msb & 0xF8);
                  }
                  whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                  colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                }
                break;
              case 1:
              case 4:
              case 8:
                {
                  if (0 == in_bits)
                  {
                    in_byte = input_buffer[in_idx++];
                    in_bits = 8;
                  }
                  uint16_t pn = (in_byte >> bitshift) & bitmask;
                  whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                  colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                  in_byte <<= depth;
                  in_bits -= depth;
                }
                break;
            }
            if (whitish)
            {
              // keep white
            }
            else if (colored && with_color)
            {
              out_color_byte &= ~(0x80 >> col % 8); // colored
            }
            else
            {
              out_byte &= ~(0x80 >> col % 8); // black
            }
            if ((7 == col % 8) || (col == w - 1)) // write that last byte! (for w%8!=0 boarder)
            {
              output_row_color_buffer[out_idx] = out_color_byte;
              output_row_mono_buffer[out_idx++] = out_byte;
              out_byte = 0xFF; // white (for w%8!=0 boarder)
              out_color_byte = 0xFF; // white (for w%8!=0 boarder)
            }
          } // end pixel
          uint16_t yrow = y + (flip ? h - row - 1 : row);
          display.writeImage(output_row_mono_buffer, output_row_color_buffer, x, yrow, w, 1);
        } // end line
        Serial.print("loaded in "); Serial.print(millis() - startTime); Serial.println(" ms");
        display.refresh();
      }
    }
  }
  file.close();
  if (!valid)
  {
    Serial.println("bitmap format not handled.");
  }
}

void pathtest() {

  getFilename("foo.bmp");
  getFilename("/foo.bmp");
  getFilename("a/foo.bmp");
  getFilename("/a/foo.bmp");
  getFilename("a/b/c/foo.bmp");
  getFilename("/a/b/c/foo.bmp");
  getPath("foo.bmp");
  getPath("/foo.bmp");
  getPath("a/foo.bmp");
  getPath("/a/foo.bmp");
  getPath("a/b/c/foo.bmp");
  getPath("/a/b/c/foo.bmp");
}
String getFilename(String absolute) {
  //Serial.println("\ngetFilename:\t");

  String resp;
  StringSplitter *splitter = new StringSplitter(absolute, '/', 30);
  int itemCount = splitter->getItemCount();
  for (int i = 0; i < itemCount; i++) {
    //Serial.print("\t");
    //Serial.print(i);
    //Serial.print(":");
    //Serial.println(splitter->getItemAtIndex(i));
  }
  if (absolute.substring(0, 1) == "/") {
    resp = splitter->getItemAtIndex(itemCount - 2);
  } else {
    resp = splitter->getItemAtIndex(itemCount - 1);
  }
  //Serial.print("items: "); Serial.print(itemCount); Serial.print("\tInput: '"); Serial.print(absolute); Serial.print("'\tOutput: '"); Serial.print(resp); Serial.println("'");
  return resp;
}

String getPath(String absolute) {
  //Serial.println("\ngetPath:\t");
  String resp;
  StringSplitter *splitter = new StringSplitter(absolute, '/', 30);
  int itemCount = splitter->getItemCount();
  for (int i = 0; i < itemCount; i++) {
    //Serial.print("\t");
    //Serial.print(i);
    //Serial.print(":");
    //Serial.println(splitter->getItemAtIndex(i));
  }
  int dec = 1;
  if (absolute.substring(0, 1) == "/") {
    dec = 2;
    resp = "/";
  }
  for (int i = 0; i < itemCount - dec; i++) {
    resp += splitter->getItemAtIndex(i) + "/";
  }


  //Serial.print("items: "); Serial.print(itemCount); Serial.print("\tInput: '"); Serial.print(absolute); Serial.print("'\tOutput: '"); Serial.print(resp); Serial.println("'");

  return resp;
}

bool isAbsolute(String path) {
  if (path.substring(0, 1) == "/") return true;
  return false;
}
