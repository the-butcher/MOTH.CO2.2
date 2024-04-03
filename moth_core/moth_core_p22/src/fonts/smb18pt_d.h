#include <Adafruit_GFX.h>
#include <Arduino.h>

const uint8_t smb18pt_dBitmaps[] PROGMEM = {0x00, 0xFF, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x03, 0xF8, 0x01, 0xFF, 0xC0, 0x7F, 0xFC, 0x1F, 0xFF, 0xC3, 0xF0, 0xF8, 0xFC, 0x0F, 0x9F, 0x01, 0xF7, 0xC0, 0x1F, 0xF8, 0x03, 0xFF, 0x00, 0x7F, 0xE0, 0x07, 0xF8, 0x00, 0xFF, 0x00, 0x1F, 0xE0, 0x03, 0xFC, 0x00, 0x7F, 0xC0, 0x0F, 0xF8, 0x03, 0xFF, 0x00, 0x7F, 0xE0, 0x0F, 0xBE, 0x03, 0xE7, 0xC0, 0x7C, 0x7C, 0x1F, 0x0F, 0xFF, 0xE0, 0xFF, 0xF8, 0x0F, 0xFE, 0x00, 0xFF, 0x80, 0x03, 0x80,
                                            0x00, 0x07, 0xE0, 0x0F, 0xE0, 0x0F, 0xE0, 0x1F, 0xE0, 0x1F, 0xE0, 0x3F, 0xE0, 0x7F, 0xE0, 0x7B, 0xE0, 0xFB, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x00, 0x0F, 0xF0, 0x1F, 0xFE, 0x1F, 0xFF, 0x8F, 0xFF, 0xEF, 0xC3, 0xF7, 0xC0, 0xFC, 0xC0, 0x3E, 0x00, 0x1F, 0x00,
                                            0x0F, 0x80, 0x07, 0xC0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xF0, 0x03, 0xF0, 0x07, 0xF0, 0x07, 0xF0, 0x0F, 0xE0, 0x0F, 0xE0, 0x0F, 0xC0, 0x0F, 0xC0, 0x07, 0xC0, 0x07, 0xC0, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0x3F, 0xFF, 0x9F, 0xFF, 0xCF, 0xFF, 0xE0, 0x03, 0xE0, 0x03, 0xE0, 0x03, 0xF0, 0x01, 0xF0, 0x01, 0xF0, 0x01, 0xF0, 0x01, 0xFC, 0x01, 0xFF, 0x81, 0xFF, 0xE0, 0x03, 0xF8, 0x00, 0x7E, 0x00, 0x1F,
                                            0x00, 0x0F, 0x80, 0x03, 0xC0, 0x01, 0xE0, 0x01, 0xF7, 0x00, 0xFF, 0xC0, 0xFB, 0xFF, 0xFC, 0xFF, 0xFC, 0x3F, 0xFC, 0x0F, 0xFC, 0x00, 0x60, 0x00, 0x00, 0xF0, 0x00, 0xF8, 0x00, 0x78, 0x00, 0x7C, 0x00, 0x3C, 0x00, 0x3E, 0x00, 0x1F, 0x00, 0x0F, 0x0F, 0x0F, 0x87, 0x87, 0xC3, 0xC7, 0xC1, 0xE3, 0xE0, 0xF3, 0xE0, 0x79, 0xF0, 0x3C, 0xF8, 0x1E, 0xF8, 0x0F, 0x7C, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x00, 0x7C, 0x00,
                                            0x1E, 0x00, 0x0F, 0x00, 0x07, 0x80, 0x03, 0xC0, 0x1F, 0xFE, 0x0F, 0xFF, 0x07, 0xFF, 0x87, 0xFF, 0xC3, 0xE0, 0x01, 0xF0, 0x00, 0xF8, 0x00, 0x7C, 0x00, 0x3E, 0x00, 0x1F, 0xF0, 0x0F, 0xFE, 0x07, 0xFF, 0xC3, 0xFF, 0xF0, 0x03, 0xF8, 0x00, 0x7E, 0x00, 0x1F, 0x00, 0x07, 0x80, 0x03, 0xC0, 0x01, 0xE0, 0x00, 0xF7, 0x00, 0xFF, 0xC0, 0xFF, 0xFF, 0xFC, 0xFF, 0xFC, 0x3F, 0xFC, 0x07, 0xFC, 0x00, 0x60, 0x00, 0x00, 0x1F, 0x00, 0x07, 0xC0, 0x01,
                                            0xF8, 0x00, 0x7E, 0x00, 0x0F, 0x80, 0x03, 0xE0, 0x00, 0xF8, 0x00, 0x1F, 0x00, 0x07, 0xC0, 0x01, 0xFF, 0x00, 0x7F, 0xF8, 0x0F, 0xFF, 0x83, 0xFF, 0xF8, 0x7C, 0x1F, 0x9F, 0x01, 0xF3, 0xC0, 0x1E, 0x78, 0x03, 0xFF, 0x00, 0x7D, 0xE0, 0x0F, 0xBC, 0x01, 0xE7, 0xC0, 0x7C, 0xFC, 0x1F, 0x8F, 0xFF, 0xE0, 0xFF, 0xFC, 0x0F, 0xFF, 0x00, 0xFF, 0x80, 0x03, 0x80, 0x00, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x01, 0xF0, 0x00, 0xF8,
                                            0x00, 0x78, 0x00, 0x7C, 0x00, 0x3E, 0x00, 0x1E, 0x00, 0x1F, 0x00, 0x0F, 0x00, 0x0F, 0x80, 0x07, 0xC0, 0x03, 0xC0, 0x03, 0xE0, 0x01, 0xE0, 0x01, 0xF0, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x7C, 0x00, 0x3C, 0x00, 0x3E, 0x00, 0x1E, 0x00, 0x1F, 0x00, 0x0F, 0x80, 0x00, 0x07, 0xF0, 0x07, 0xFF, 0x03, 0xFF, 0xE0, 0xFF, 0xF8, 0x7E, 0x3F, 0x1F, 0x07, 0xC7, 0x80, 0xF1, 0xE0, 0x3C, 0x7C, 0x1F, 0x0F, 0x8F, 0x83, 0xFF, 0xE0, 0x7F, 0xF0, 0x3F, 0xFE,
                                            0x1F, 0xFF, 0xCF, 0xC1, 0xFB, 0xE0, 0x3E, 0xF0, 0x07, 0xBC, 0x01, 0xFF, 0x00, 0x7F, 0xC0, 0x1F, 0xF8, 0x0F, 0xBF, 0x07, 0xE7, 0xFF, 0xF1, 0xFF, 0xFC, 0x3F, 0xFE, 0x03, 0xFE, 0x00, 0x1C, 0x00, 0x03, 0xFC, 0x01, 0xFF, 0xC0, 0x7F, 0xFC, 0x1F, 0xFF, 0xC7, 0xF0, 0xFC, 0xF8, 0x0F, 0x9F, 0x00, 0xF3, 0xC0, 0x1F, 0xF8, 0x03, 0xEF, 0x00, 0x7D, 0xE0, 0x0F, 0x3E, 0x03, 0xE7, 0xE0, 0xFC, 0x7F, 0xFF, 0x07, 0xFF, 0xE0, 0x7F, 0xF8, 0x07, 0xFE,
                                            0x00, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x7C, 0x00, 0x1F, 0x00, 0x07, 0xE0, 0x00, 0xF8, 0x00, 0x3E, 0x00, 0x0F, 0x80, 0x03, 0xF0, 0x00, 0xF0, 0x01, 0xE0, 0x03, 0xC0, 0x07, 0x80, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x7F, 0xF8, 0xFF, 0xF9, 0xFF, 0xFB, 0xFF, 0xFF, 0xC1, 0xFF, 0x01, 0xFE, 0x03, 0xFC, 0x07, 0xF8, 0x0F, 0xF0, 0x1F, 0xE0, 0x3F, 0xC0, 0x7F, 0x80, 0xFF, 0x01, 0xFE, 0x03, 0xFC, 0x07, 0xF8, 0x0F, 0xF0, 0x1F, 0xE0, 0x3C, 0xF7,
                                            0x8F, 0x9F, 0xFB, 0xFB, 0xFF, 0xFF, 0xFD, 0xFD, 0xFF, 0x1F, 0x1F, 0xE3, 0xE3, 0xFC, 0x3C, 0x7F, 0x87, 0x8F, 0xF0, 0xF1, 0xFE, 0x1E, 0x3F, 0xC3, 0xC7, 0xF8, 0x78, 0xFF, 0x0F, 0x1F, 0xE1, 0xE3, 0xFC, 0x3C, 0x7F, 0x87, 0x8F, 0xF0, 0xF1, 0xFE, 0x1E, 0x3F, 0xC3, 0xC7, 0x80, 0x07, 0xF0, 0x0F, 0xFE, 0x0F, 0xFF, 0x8F, 0xFF, 0xE7, 0xC1, 0xF7, 0xC0, 0x7F, 0xE0, 0x3F, 0xF0, 0x0F, 0xF0, 0x07, 0xF8, 0x03, 0xFC, 0x01, 0xFE, 0x00, 0xFF, 0x80,
                                            0xFF, 0xC0, 0x7D, 0xF0, 0x7C, 0xFF, 0xFE, 0x3F, 0xFE, 0x0F, 0xFE, 0x03, 0xFE, 0x00, 0x38, 0x00, 0x03, 0xC0, 0x03, 0xE0, 0x01, 0xF0, 0x00, 0xF8, 0x00, 0x7C, 0x00, 0x3E, 0x00, 0x1F, 0x01, 0xFF, 0xFC, 0xFF, 0xFE, 0x7F, 0xFF, 0x3F, 0xFF, 0x80, 0xF8, 0x00, 0x7C, 0x00, 0x3E, 0x00, 0x1F, 0x00, 0x0F, 0x80, 0x07, 0xC0, 0x03, 0xE0, 0x01, 0xF0, 0x00, 0xF8, 0x00, 0x3E, 0x00, 0x1F, 0x80, 0x0F, 0xFC, 0x03, 0xFF, 0x00, 0xFF, 0x80, 0x1F, 0xC0};

const GFXglyph smb18pt_dGlyphs[] PROGMEM = {

    {0, 2, 2, 22, 0, 0},         // 0x20 ' '
    {0, 0, 0, 0, 0, 0},          // 0x21 '!'
    {0, 0, 0, 0, 0, 0},          // 0x22 '"'
    {0, 0, 0, 0, 0, 0},          // 0x23 '#'
    {0, 0, 0, 0, 0, 0},          // 0x24 '$'
    {0, 0, 0, 0, 0, 0},          // 0x25 '%'
    {0, 0, 0, 0, 0, 0},          // 0x26 '&'
    {0, 0, 0, 0, 0, 0},          // 0x27 '''
    {0, 0, 0, 0, 0, 0},          // 0x28 '('
    {0, 0, 0, 0, 0, 0},          // 0x29 ')'
    {0, 0, 0, 0, 0, 0},          // 0x2A '*'
    {0, 0, 0, 0, 0, 0},          // 0x2B '+'
    {0, 0, 0, 0, 0, 0},          // 0x2C ','
    {1, 13, 4, 22, 6, -14},      // 0x2D '-'
    {0, 0, 0, 0, 0, 0},          // 0x2E '.'
    {0, 0, 0, 0, 0, 0},          // 0x2F '/'
    {8, 19, 27, 22, 1, -25},     // 0x30 '0'
    {73, 16, 26, 22, 2, -25},    // 0x31 '1'
    {126, 17, 26, 22, 2, -25},   // 0x32 '2'
    {182, 17, 27, 22, 2, -25},   // 0x33 '3'
    {240, 17, 26, 22, 1, -25},   // 0x34 '4'
    {296, 17, 27, 22, 2, -25},   // 0x35 '5'
    {354, 19, 27, 22, 1, -25},   // 0x36 '6'
    {419, 17, 26, 22, 2, -25},   // 0x37 '7'
    {475, 18, 27, 22, 2, -25},   // 0x38 '8'
    {536, 19, 26, 22, 1, -25},   // 0x39 '9'
    {0, 0, 0, 0, 0, 0},          // 0x3A ':'
    {0, 0, 0, 0, 0, 0},          // 0x3B ';'
    {0, 0, 0, 0, 0, 0},          // 0x3C '<'
    {0, 0, 0, 0, 0, 0},          // 0x3D '='
    {0, 0, 0, 0, 0, 0},          // 0x3E '>'
    {0, 0, 0, 0, 0, 0},          // 0x3F '?'
    {0, 0, 0, 0, 0, 0},          // 0x40 '@'
    {0, 0, 0, 0, 0, 0},          // 0x41 'A'
    {0, 0, 0, 0, 0, 0},          // 0x42 'B'
    {0, 0, 0, 0, 0, 0},          // 0x43 'C'
    {0, 0, 0, 0, 0, 0},          // 0x44 'D'
    {0, 0, 0, 0, 0, 0},          // 0x45 'E'
    {0, 0, 0, 0, 0, 0},          // 0x46 'F'
    {0, 0, 0, 0, 0, 0},          // 0x47 'G'
    {0, 0, 0, 0, 0, 0},          // 0x48 'H'
    {0, 0, 0, 0, 0, 0},          // 0x49 'I'
    {0, 0, 0, 0, 0, 0},          // 0x4A 'J'
    {0, 0, 0, 0, 0, 0},          // 0x4B 'K'
    {0, 0, 0, 0, 0, 0},          // 0x4C 'L'
    {0, 0, 0, 0, 0, 0},          // 0x4D 'M'
    {0, 0, 0, 0, 0, 0},          // 0x4E 'N'
    {0, 0, 0, 0, 0, 0},          // 0x4F 'O'
    {0, 0, 0, 0, 0, 0},          // 0x50 'P'
    {0, 0, 0, 0, 0, 0},          // 0x51 'Q'
    {0, 0, 0, 0, 0, 0},          // 0x52 'R'
    {0, 0, 0, 0, 0, 0},          // 0x53 'S'
    {0, 0, 0, 0, 0, 0},          // 0x54 'T'
    {0, 0, 0, 0, 0, 0},          // 0x55 'U'
    {0, 0, 0, 0, 0, 0},          // 0x56 'V'
    {0, 0, 0, 0, 0, 0},          // 0x57 'W'
    {0, 0, 0, 0, 0, 0},          // 0x58 'X'
    {0, 0, 0, 0, 0, 0},          // 0x59 'Y'
    {0, 0, 0, 0, 0, 0},          // 0x5A 'Z'
    {0, 0, 0, 0, 0, 0},          // 0x5B '['
    {0, 0, 0, 0, 0, 0},          // 0x5C '\'
    {0, 0, 0, 0, 0, 0},          // 0x5D ']'
    {0, 0, 0, 0, 0, 0},          // 0x5E '^'
    {0, 0, 0, 0, 0, 0},          // 0x5F '_'
    {0, 0, 0, 0, 0, 0},          // 0x60 '`'
    {0, 0, 0, 0, 0, 0},          // 0x61 'a'
    {0, 0, 0, 0, 0, 0},          // 0x62 'b'
    {0, 0, 0, 0, 0, 0},          // 0x63 'c'
    {0, 0, 0, 0, 0, 0},          // 0x64 'd'
    {0, 0, 0, 0, 0, 0},          // 0x65 'e'
    {0, 0, 0, 0, 0, 0},          // 0x66 'f'
    {0, 0, 0, 0, 0, 0},          // 0x67 'g'
    {598, 15, 26, 22, 3, -25},   // 0x68 'h'
    {0, 0, 0, 0, 0, 0},          // 0x69 'i'
    {0, 0, 0, 0, 0, 0},          // 0x6A 'j'
    {0, 0, 0, 0, 0, 0},          // 0x6B 'k'
    {0, 0, 0, 0, 0, 0},          // 0x6C 'l'
    {647, 19, 19, 22, 1, -18},   // 0x6D 'm'
    {0, 0, 0, 0, 0, 0},          // 0x6E 'n'
    {693, 17, 20, 22, 2, -18},   // 0x6F 'o'
    {0, 0, 0, 0, 0, 0},          // 0x70 'p'
    {0, 0, 0, 0, 0, 0},          // 0x71 'q'
    {0, 0, 0, 0, 0, 0},          // 0x72 'r'
    {0, 0, 0, 0, 0, 0},          // 0x73 's'
    {736, 17, 26, 22, 3, -25}};  // 0x74 't'

const GFXfont smb18pt_d PROGMEM = {(uint8_t *)smb18pt_dBitmaps, (GFXglyph *)smb18pt_dGlyphs, 0x20, 0x74, 21};
