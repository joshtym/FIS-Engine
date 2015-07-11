/***************************************************************************
* Class Name: /
* Date Created: June 22, 2014
* Inheritance: None
* Description: 
*
* Notes
* -----
*
* [1]:
*
* See .h file for TODOs
******************************************************************************/
#include "Game/Battle/RenderElement.h"

/*=============================================================================
 * CONSTANTS
 *============================================================================*/

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description:
 *
 * Inputs:
 */
RenderElement::RenderElement()
    : action_frame{nullptr}
    , render_sprite{nullptr}
    , flasher{nullptr}
    , status{RenderStatus::CONSTRUCTING}
    , type{RenderType::DAMAGE_VALUE}
    , color{0, 0, 0, 0}
    , shadow_color{0, 0, 0, 0}
    , font{nullptr}
    , text{""}
    , shadow{false}
    , remaining_time{0}
    , elapsed_time{0}
    , acceleration_x{0}
    , acceleration_y{0}
    , delta_x{0}
    , delta_y{0}
    , x{0}
    , y{0}
    , shadow_x{0}
    , shadow_y{0}
    , size_x{0}
    , size_y{0}
    , velocity_x{0}
    , velocity_y{0}
    , fade_in_time{0}
    , fade_out_time{0}
    , alpha{0}
    , shadow_alpha{0}
{}

/*
 * Description: Creates a render element with a type, remaining and fade in/out
 *              times. Note that for the element to be valid the fade in and
 *              out times must both not exceed or together not exceed the
 *              lifetime of the behaviour.
 *
 * Inputs: RenderType type - the type of render element to be created
 *         int32_t remaining_time - the lifetime of the object
 *         int32_t fade_in_time - time taken to fade in (default 0)
 *         int32_t fade_out_time - the time taken to fade out
 */
RenderElement::RenderElement(RenderType type, int32_t remaining_time,
    int32_t fade_in_time, int32_t fade_out_time) 
    : RenderElement()
{
  this->type = type;
  
  bool success = setTimes(remaining_time, fade_in_time, fade_out_time);

  if (success)
  {
    if (fade_in_time > 0)
      status = RenderStatus::FADING_IN;    

    else if (fade_in_time == 0 && fade_out_time == remaining_time)
      status = RenderStatus::FADING_OUT;
    else
      status = RenderStatus::DISPLAYING;
  }
  else
    status = RenderStatus::TIMED_OUT;
}

RenderElement::RenderElement(Sprite* plep, int32_t x, int32_t y)
  : RenderElement()
{
  type = RenderType::PLEP;
  status = RenderStatus::DISPLAYING;

  this->render_sprite = plep;
  this->x = x;
  this->y = y;
}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

uint8_t RenderElement::calcColorRed()
{
  auto pc_fade = 0;

  if (color.a != 0)
    pc_fade = (getAlpha() * 100) / color.a;
  
  float red_float = (pc_fade * color.r) / (float)100;

  return std::round(red_float);
}

uint8_t RenderElement::calcColorBlue()
{
  auto pc_fade = 0;

  if (color.a != 0)
    pc_fade = (getAlpha() * 100) / color.a;

  float blue_float = (pc_fade * color.b) / (float)100;
  return std::round(blue_float);
}

uint8_t RenderElement::calcColorGreen()
{
  auto pc_fade = 0;

  if (color.a != 0)
    pc_fade = (getAlpha() * 100) / color.a;

  float green_float = (pc_fade * color.g) / (float)100;
  return std::round(green_float);
}


/*
 * Description:
 *
 * Inputs:
 * Output:
 */
bool RenderElement::hasShadow()
{
  return shadow;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
bool RenderElement::isTimedOut()
{
  return (remaining_time < 0);
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
bool RenderElement::update(int cycle_time)
{
  if (type == RenderType::PLEP)
  {
    render_sprite->update(cycle_time);
    std::cout << "Updating plep: " << render_sprite->getLoops() << std::endl;
    if (render_sprite->getLoops() > 0)
      status = RenderStatus::TIMED_OUT;
  }
  else
  {
  /* Update the remaining time, put it into elapsed time */
  remaining_time -= cycle_time;
  elapsed_time   += cycle_time;

  if (elapsed_time >= fade_in_time)
    status = RenderStatus::DISPLAYING;
  if (remaining_time < fade_out_time)
    status = RenderStatus::FADING_OUT;

  /* Before updating coordinates, update the pixels/s from the pixels/s/s */
  // velocity_x += acceleration_x;
  // velocity_y += acceleration_y;

  /* Then, update the (X, Y) coordinates for the text and the shadow */
  auto temp_delta_x = 0.0;
  auto temp_delta_y = 0.0;

  temp_delta_x += static_cast<float>(velocity_x) * cycle_time / 100.0f;
  temp_delta_y += static_cast<float>(velocity_y) * cycle_time / 100.0f;

  if (delta_x >= 1.0 || delta_x <= -1.0)
  {
    x += std::floor(delta_x);

    if (shadow)
      shadow_x += std::floor(delta_x);

    delta_x = 0;
  }
  else
  {
    delta_x += temp_delta_x;
  }

  if (delta_y >= 1.0 || delta_y <= -1.0)
  {
    y += std::floor(delta_y);

    if (shadow)
      shadow_y += std::floor(delta_y);

    delta_y = 0;
  }
  else
  {
    delta_y += temp_delta_y;
  }
  }

  return true;
}

SDL_Color RenderElement::getColor()
{
  return color;
}

SDL_Color RenderElement::getShadowColor()
{
  return shadow_color;
}

int32_t RenderElement::getElapsedTime()
{
  return elapsed_time;
}

int32_t RenderElement::getFadeInTime()
{
  return fade_in_time;
}

int32_t RenderElement::getFadeOutTime()
{
  return fade_out_time;
}

Frame* RenderElement::getActionFrame()
{
  return action_frame;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
uint8_t RenderElement::getAlpha()
{
  return alpha;
}

TTF_Font* RenderElement::getFont()
{
  return font;
}

Person* RenderElement::getFlasher()
{
  return flasher;
}

int32_t RenderElement::getRemainingTime()
{
  return remaining_time;
}

Sprite* RenderElement::getSprite()
{
  return render_sprite;
}

RenderStatus RenderElement::getStatus()
{
  return status;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
uint8_t RenderElement::getShadowAlpha()
{
  return shadow_alpha;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
std::string RenderElement::getText()
{
  return text;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
RenderType RenderElement::getType()
{
  return type;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
int32_t RenderElement::getX()
{
  return x;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
int32_t RenderElement::getY()
{
  return y;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
int32_t RenderElement::getShadowX()
{
  return shadow_x;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
int32_t RenderElement::getShadowY()
{
  return shadow_y;
}

int32_t RenderElement::getSizeX()
{
  return size_x;
}

int32_t RenderElement::getSizeY()
{
  return size_y;
}

void RenderElement::setActionFrame(Frame* action_frame)
{
  this->action_frame = action_frame;
}

void RenderElement::setAlpha(uint8_t new_alpha)
{
  alpha = new_alpha;
  shadow_alpha = new_alpha;
}


/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setAcceleration(int32_t new_acceleration_x, 
    int32_t new_acceleration_y)
{
  acceleration_x = new_acceleration_x;
  acceleration_y = new_acceleration_y;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setCoordinates(int32_t new_x, int32_t new_y)
{
  setX(new_x);
  setY(new_y);
}

void RenderElement::setSprite(Sprite* new_render_sprite)
{
  render_sprite = new_render_sprite;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setShadowCoordinates(int32_t new_shadow_x, 
    int32_t new_shadow_y)
{
  setShadowX(new_shadow_x);
  setShadowY(new_shadow_y);
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setColor(SDL_Color new_color)
{
  color = new_color;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setShadowColor(SDL_Color new_shadow_color)
{
  shadow_color = new_shadow_color;
}

void RenderElement::setSizes(int32_t new_size_x, int32_t new_size_y)
{
  size_x = new_size_x;
  size_y = new_size_y;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setFont(TTF_Font* new_font)
{
  font = new_font;
}

void RenderElement::setFlasher(Person* new_flasher)
{
  flasher = new_flasher;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setShadow(bool to_show)
{
  shadow = to_show;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
bool RenderElement::setTimes(int32_t new_remaining_time, int32_t fade_in, 
    int32_t fade_out)
{
  bool valid = new_remaining_time >= 0;
  valid &= fade_in >= 0;
  valid &= fade_out >= 0;
  valid &= fade_in + fade_out <= new_remaining_time;
  valid &= fade_in <= new_remaining_time;
  valid &= fade_out <= new_remaining_time;

  if (valid)
  {
    remaining_time = new_remaining_time;
    fade_in_time   = fade_in;
    fade_out_time  = fade_out;
  }

  return valid;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setText(std::string new_text)
{
  text = new_text;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setVelocity(int32_t new_velocity_x, int32_t new_velocity_y)
{
  velocity_x = new_velocity_x;
  velocity_y = new_velocity_y;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setX(int32_t new_x)
{
  x = new_x;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setY(int32_t new_y)
{
  y = new_y;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setShadowX(int32_t new_shadow_x)
{
  shadow_x = x + new_shadow_x;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void RenderElement::setShadowY(int32_t new_shadow_y)
{
  shadow_y = y + new_shadow_y;
}

void RenderElement::setSizeX(int32_t new_size_x)
{
  size_x = new_size_x;
}

void RenderElement::setSizeY(int32_t new_size_y)
{
  size_y = new_size_y;
}