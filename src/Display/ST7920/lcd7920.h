#ifndef LCD7920_H
#define LCD7920_H

#include "RepRapFirmware.h"
#include "Print.h"
#include "SharedSpi.h"

// Enumeration for specifying drawing modes
enum PixelMode
{
	PixelClear = 0,    // clear the pixel(s)
	PixelSet = 1,      // set the pixel(s)
	PixelFlip = 2      // invert the pixel(s)
};

// Struct for describing a font table, always held in PROGMEM
struct LcdFont
{
	const uint8_t *ptr;			// pointer to font table
	uint8_t startCharacter;		// character code (e.g. ASCII) of the first character in the font
	uint8_t endCharacter;		// character code of the last character in the font
	uint8_t height;				// row height in pixels - only this number of pixels will be fetched and drawn - maximum 16 in this version of the software
	uint8_t width;				// max character width in pixels (the font table contains this number of bytes or words per character, plus 1 for the active width)
};

// Class for driving 128x64 graphical LCD fitted with ST7920 controller
// This drives the GLCD in serial mode so that it needs just 2 pins.
// Preferably, we use SPI to do the comms.

// Derive the LCD class from the Print class so that we can print stuff to it in alpha mode
class Lcd7920 : public Print
{
public:
	// Construct a GLCD driver.
	Lcd7920(Pin csPin);    		// constructor

	// Write a single character in the current font. Called by the 'print' functions.
	// A call to setFont must have been made before calling this.
	//  c = character to write
	// Returns the number of characters written (1 if we wrote it, 0 otherwise)
	virtual size_t write(uint8_t c);                 // write a character

	// Initialize the display. Call this in setup(). Also call setFont to select initial text font.
	void begin();

	// Select the font to use for subsequent calls to write() in graphics mode. Must be called before calling write().
	//  newFont = pointer to font descriptor in PROGMEM
	void setFont(const LcdFont *newFont);

	// Select normal or inverted text (only works in graphics mode)
	void textInvert(bool b);

	// Clear the display. Works in both graphic and alphanumeric mode. Also selects non-inverted text.
	void clear();

	// Set the cursor position
	//  r = row. In alphanumeric mode this is text row number.
	//           In graphics mode it is the number of pixels from the top of the display to the top of the character.
	//  c = column. In alphanumeric mode this must be even (ST7920 restriction because the characters are double width).
	//              In graphics mode this is the number of pixels from the left hand edge of the display and the left hand edge of the character.
	void setCursor(uint8_t r, uint8_t c);        // 'c' in alpha mode, should be an even column number

	// Get the cursor column. Useful in graphics mode after we have written some text.
	uint8_t getColumn() const { return column; }

	// Set the right margin. In graphics mode, anything written will be truncated at the right margin. Defaults to the right hand edge of the display.
	void setRightMargin(uint8_t r);

	// Clear a rectangle from the current position to the right margin (graphics mode only). The height of the rectangle is the height of the current font.
	void clearToMargin();

	// Flush the display buffer to the display. In graphics mode, calls to write, setPixel, line and circle will not be committed to the display until this is called.
	void flush();

	// Set, clear or invert a pixel
	//  x = x-coordinate of the pixel, measured from left hand edge of the display
	//  y = y-coordinate of the pixel, measured down from the top of the display
	//  mode = whether we want to set, clear or invert the pixel
	void setPixel(uint8_t x, uint8_t y, PixelMode mode);

	// Read a pixel. Returns true if the pixel is set, false if it is clear.
	//  x = x-coordinate of the pixel, measured from left hand edge of the display
	//  y = y-coordinate of the pixel, measured down from the top of the display
	bool readPixel(uint8_t x, uint8_t y) const;

	// Draw a line.
	//  x0 = x-coordinate of one end of the line, measured from left hand edge of the display
	//  y0 = y-coordinate of one end of the line, measured down from the top of the display
	//  x1, y1 = coordinates of the other end od the line
	//  mode = whether we want to set, clear or invert each pixel
	void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, PixelMode mode);

	// Draw a circle
	//  x0 = x-coordinate of the centre, measured from left hand edge of the display
	//  y0 = y-coordinate of the centre, measured down from the top of the display
	//  radius = radius of the circle in pixels
	//  mode = whether we want to set, clear or invert each pixel
	void circle(uint8_t x0, uint8_t y0, uint8_t radius, PixelMode mode);

	// Draw a bitmap
	//  x0 = x-coordinate of the top left, measured from left hand edge of the display. Currently, must be a multiple of 8.
	//  y0 = y-coordinate of the top left, measured down from the top of the display
	// width = width of bitmap in pixels. Currently, must be a multiple of 8.
	// rows = height of bitmap in pixels
	// data = bitmap image in PROGMEM, must be ((width/8) * rows) bytes long
	void bitmap(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height, const uint8_t data[]);

	// Set up translation for characters. Useful for translating fullstop into decimal point, or changing the width of spaces.
	// Either the first string passed must be NULL, or the two strings must have equal lengths as returned by strlen().
	void setTranslation(const char *tFrom, const char *tTo);

	// Keep the display up to date by sending some data if necessary. Call this regularly.
	void Update();

private:
	sspi_device device;
	uint16_t lastCharColData;                   // data for the last non-space column, used for kerning
	uint8_t row, column;
	uint8_t startRow, startCol, endRow, endCol; // coordinates of the dirty rectangle
	uint8_t rightMargin;
	uint8_t image[(128 * 64)/8];                // image buffer, 1K in size
	const struct LcdFont *currentFont;			// pointer to descriptor for current font
	const char* translateFrom;
	const char* translateTo;
	bool extendedMode;
	bool textInverted;

	void sendLcdCommand(uint8_t command);
	void sendLcdData(uint8_t data);
	void sendLcd(uint8_t data1, uint8_t data2);
	void commandDelay();
	void setGraphicsAddress(unsigned int r, unsigned int c);
	void ensureBasicMode();
	void ensureExtendedMode();
};

#endif
