/*******************************************************************************
 * Class Name: Frame
 * Date Created: December 2, 2012
 * Inheritance: none
 * Description: The Frame class, this represents an animation frame within the
 *              Sprite class. It acts as a linked list node, in that it contains
 *              a pointer to the next Frame in the sequence. The image frame is
 *              stored as a SDL_Texture which is used for rendering.
 ******************************************************************************/
#ifndef FRAME_H
#define FRAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "Helpers.h"

#include "SDL2_gfxPrimitives.h"

/* PI */
#define PI 3.14159265359

/* Class for frame handling */
class Frame
{
public:
  /* Constructor function - empty initialization */
  Frame();

  /* Constructor function - path and next pointer initialization */
  Frame(std::string path, SDL_Renderer* renderer, uint16_t angle = 0,
        Frame* previous = nullptr, Frame* next = nullptr);

  /* Constructor function - path, adjustments, and previous/next inits */
  Frame(std::string path, std::vector<std::string> adjustments,
        SDL_Renderer* renderer, uint16_t angle = 0, Frame* previous = nullptr,
        Frame* next = nullptr);

  /* Destructor function */
  ~Frame();

private:
  /* The stored alpha value for rendering */
  uint8_t alpha;

  /* The frame control color mode */
  uint8_t color_alpha;
  ColorMode color_mode;

  /* How the SDL texture should be flipped while rendering */
  SDL_RendererFlip flip;

  /* The height of the stored texture */
  int height;

  /* The next element in the linked list */
  Frame* next;

  /* The frame path sprite data */
  std::string path;

  /* The previous element in the linked list */
  Frame* previous;

  /* Source rect settings */
  SDL_Rect rect_src;
  bool rect_src_valid;

  /* The texture for this frame */
  SDL_Texture* texture;
  SDL_Texture* texture_grey;

  /* The width of the stored texture */
  int width;

  /*------------------- Private Constants -----------------------*/
  const static uint8_t kDEFAULT_ALPHA; /* The default alpha rating */
  const static float kGREY_FOR_BLUE;   /* Grey scale convert for blue factor */
  const static float kGREY_FOR_GREEN;  /* Grey scale convert for green factor */
  const static float kGREY_FOR_RED;    /* Grey scale convert for red factor */

  /*========================= PUBLIC FUNCTIONS ===============================*/
public:
  /* Executes the necessary image adjustments, as per the file data handlers */
  bool execImageAdjustment(std::string adjustment);
  bool execImageAdjustments(std::vector<std::string> adjustments);

  /* Flips the sprite SDL image - either horizontal or vertical */
  void flipHorizontal(bool flip = true);
  void flipVertical(bool flip = true);

  /* Returns the alpha setting of the stored texture */
  uint8_t getAlpha();

  /* Returns the color information */
  uint8_t getColorAlpha();
  ColorMode getColorMode();

  /* Returns the flip indication, for rendering */
  SDL_RendererFlip getFlip();

  /* Returns the height of the texture */
  int getHeight();

  /* Get next frame */
  Frame* getNext();

  /* Returns the frame path that was used in the last setTexture() call */
  std::string getPath();

  /* Get previous frame */
  Frame* getPrevious();

  /* Returns the 2D SDL texture to be painted */
  SDL_Texture* getTexture(bool grey_scale = false);
  SDL_Texture* getTextureActive();

  /* Returns the width of the texture */
  int getWidth();

  /* Returns if an image is set */
  bool isTextureSet(bool grey_scale = false);

  /* Render the texture to the given renderer with the given parameters */
  bool render(SDL_Renderer* renderer, int x = 0, int y = 0, int w = 0,
              int h = 0, SDL_Rect* src_rect = nullptr, bool for_sprite = false);

  /* Sets the alpha rating of the texture rendering */
  void setAlpha(uint8_t alpha = 255);

  /* Sets the color information handling */
  bool setColorAlpha(uint8_t alpha = 0);
  bool setColorMode(ColorMode mode);

  /* Set next frame */
  bool setNext(Frame* next);

  /* Set previous frame */
  bool setPrevious(Frame* previous);

  /* Sets the frame texture */
  bool setTexture(std::string path, SDL_Renderer* renderer, uint16_t angle = 0,
                  bool no_warnings = false, bool enable_greyscale = true);
  bool setTexture(std::string path, std::vector<std::string> adjustments,
                  SDL_Renderer* renderer, uint16_t angle = 0,
                  bool no_warnings = false, bool enable_greyscale = true);
  bool setTexture(SDL_Texture* texture);

  /* Unsets the texture, if one is set */
  void unsetTexture();

  /*===================== PRIVATE STATIC  FUNCTIONS ==========================*/
private:
  /* Draws a line. This is needed because of SDL draw line glitch */
  static void drawLine(int32_t x1, int32_t x2, int32_t y,
                       SDL_Renderer* renderer);
  static void drawLineY(int32_t y1, int32_t y2, int32_t x,
                        SDL_Renderer* renderer);

  /* Render bottom flat triangle - handled by renderTriangle() */
  static void renderBottomFlatTriangle(uint16_t x1, uint16_t x2, uint16_t x3,
                                       uint16_t y1, uint16_t y23,
                                       SDL_Renderer* renderer, bool aliasing,
                                       bool flat_side = false);

  /* Render top flat triangle - handled by renderTriangle() */
  static void renderTopFlatTriangle(uint16_t x1, uint16_t x2, uint16_t x3,
                                    uint16_t y12, uint16_t y3,
                                    SDL_Renderer* renderer, bool aliasing,
                                    bool flat_side = false);

  /*===================== PUBLIC STATIC  FUNCTIONS ===========================*/
public:
  /* Draws a line given a vector of coordinates */
  static void drawLine(std::vector<Coordinate> line_points,
                       SDL_Renderer* renderer);

  /* Converts RGB to grey scale value */
  static uint8_t getGreyValue(uint8_t red, uint8_t green, uint8_t blue);

  /* Creates a bar, given the parameters and a renderer */
  static bool renderBar(uint16_t x, uint16_t y, uint16_t length,
                        uint16_t height, float slope, SDL_Renderer* renderer);

  /* Creates a circle border, given the parameters and a renderer */
  static bool renderCircle(int center_x, int center_y, uint16_t radius,
                           SDL_Renderer* renderer);

  /* Creates a filled circle, given the parameters and a renderer */
  static bool renderCircleFilled(int center_x, int center_y, uint16_t radius,
                                 SDL_Renderer* renderer);

  /* Creates a rectangle with multiple pixel border */
  static bool renderRect(SDL_Rect rect, uint16_t border_width,
                         SDL_Renderer* renderer, bool reverse = false);

  /* Creates a Rectangle with SDL_Gfx given a specfic rectangle and color */
  static bool renderRectSelect(SDL_Rect rect, SDL_Renderer* renderer,
                               SDL_Color color);

  /* */
  static bool renderRectBorderSelect(SDL_Rect rect, SDL_Renderer* renderer,
                                     SDL_Color color);

  /* Creates a right hand triangle, given the parameters and a renderer */
  static bool renderRHTriangle(uint32_t x, uint32_t y, uint16_t height,
                               SDL_Renderer* renderer, bool reverse = false);

  /* Creates a triangle, given the parameters and a renderer */
  static bool renderTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                             uint16_t x3, uint16_t y3, SDL_Renderer* renderer,
                             bool aliasing = false);

  /* Fills hoziontal lines between two vectors of lines */
  static bool renderFillLineToLine(std::vector<Coordinate> line_start,
                                   std::vector<Coordinate> line_end,
                                   SDL_Renderer* renderer,
                                   bool vertical = false);

  /* Renders a Hexagon with origin point and a size h x eh */
  static bool renderHexagon(Coordinate start, int32_t l,
                            SDL_Renderer* renderer);

  /* Renders a Hexagonal border */
  static bool renderHexagonBorder(Coordinate start, int32_t l,
                                  SDL_Renderer* renderer);

  /* Render the top part of a normalized trapezoid (half a hexagon) */
  static bool renderTrapezoidNormalTop(Coordinate start, int32_t l,
                                       SDL_Renderer* renderer);

  /* Renders a trapezoidal normalized top border */
  static bool renderTrapezoidNormalTopBorder(Coordinate start, int32_t l,
                                             SDL_Renderer* renderer,
                                             bool hexagon);

  /* Render the bottom part of a normalzied trapezoid */
  static bool renderTrapezoidNormalBottom(Coordinate start, int32_t l,
                                          SDL_Renderer* renderer);

  /* Renders a trapezoidal normalized bottom border */
  static bool renderTrapezoidNormalBottomBorder(Coordinate start, int32_t l,
                                                SDL_Renderer* renderer,
                                                bool hexagon);

  /*  Renders any trapezoid */
  static bool renderTrapezoid(Coordinate start, int32_t h, int32_t b1,
                              int32_t b2, SDL_Renderer* renderer);

  static bool renderFoursided(Coordinate a, Coordinate b, Coordinate c,
                              Coordinate d);

  // static bool renderExpCircle(Coordinate start, uint32_t w, float curr_exp_pc,
  //                             float orig_exp_pc, uint32_t level,
  //                             uint32_t orig_level, SDL_Renderer* renderer);

  /* Render a hexagonal experience bar given needed values */
  static bool renderExpHex(Coordinate start, uint32_t w, float curr_exp_pc,
                           float orig_gain_pc, uint32_t level,
                           uint32_t orig_level, SDL_Renderer* renderer);

  /* Renders a blank experience hex */
  static bool renderExpHexBlank(Coordinate start, uint32_t w,
                                SDL_Renderer* renderer);

  /* Method to set the rendering draw color */
  static bool setRenderDrawColor(SDL_Renderer* renderer, SDL_Color color);
};

#endif // FRAME_H
