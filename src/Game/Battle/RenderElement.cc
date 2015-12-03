/*******************************************************************************
* Class Name: RenderElement
* Date Created: June 22, 2015
* Date Redesigned: November 8, 2015
* Inheritance: None
*
* See .h for description and TODOs
******************************************************************************/
#include "Game/Battle/RenderElement.h"
#include "Game/Battle/Battle.h"

/*=============================================================================
 * CONSTANTS
 *============================================================================*/

/* ---- Color Constants ---- */
const SDL_Color RenderElement::kSTRD_DMG_COLOR = {225, 225, 225, 255};
const SDL_Color RenderElement::kCRIT_DMG_COLOR = {255, 255, 0, 255};
const SDL_Color RenderElement::kPOIS_DMG_COLOR = {138, 43, 226, 255};
const SDL_Color RenderElement::kBURN_DMG_COLOR = {172, 0, 0, 255};
const SDL_Color RenderElement::kHEAL_DMG_COLOR = {50, 215, 49, 255};
const SDL_Color RenderElement::kVITA_REGEN_COLOR = {50, 205, 50, 255};
const SDL_Color RenderElement::kQTDR_REGEN_COLOR = {0, 128, 255, 255};
const SDL_Color RenderElement::kHIBERNATION_REGEN_COLOR = {75, 205, 50, 255};
const uint16_t RenderElement::kACTION_COLOR_R = 175;
const uint16_t RenderElement::kACTION_SHADOW = 3;
const uint16_t RenderElement::kDAMAGE_SHADOW = 2;
const uint16_t RenderElement::kACTION_TEXT_X = 800;
const uint16_t RenderElement::kACTION_CENTER = 381;

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTOR
 *============================================================================*/

RenderElement::RenderElement()
    : has_shadow{false},
      text_string{""},
      time_fade_in{0},
      time_fade_out{0},
      time_begin{0},
      time_left{0},
      alpha{0},
      fade_rate{0},
      element_sprite{nullptr},
      loops_to_do{0},
      element_text{Text()},
      element_font{nullptr},
      color{0, 0, 0, 0},
      shadow_color{0, 0, 0, 0},
      renderer{nullptr},
      status{RenderStatus::DISPLAYING},
      target{nullptr},
      render_type{RenderType::NONE}
{
}

RenderElement::RenderElement(SDL_Renderer* renderer, TTF_Font* element_font)
    : RenderElement()
{
  this->element_font = element_font;
  this->renderer = renderer;
}

RenderElement::RenderElement(SDL_Renderer* renderer, Sprite* plep_sprite,
                             int32_t num_loops, int32_t x, int32_t y)
    : RenderElement()
{
  this->renderer = renderer;
  buildSprite(plep_sprite);

  if(element_sprite)
    element_sprite->resetLoops();

  loops_to_do = num_loops;

  status = initialStatusFade();
  render_type = RenderType::PLEP;

  location.point.x = x;
  location.point.y = y;
}

RenderElement::~RenderElement()
{
  if(element_sprite)
    delete element_sprite;

  element_sprite = nullptr;
}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

bool RenderElement::buildSprite(Sprite* build_sprite)
{
  if(renderer && build_sprite && !element_sprite)
  {
    element_sprite = new Sprite(*(build_sprite));

    if(element_sprite)
    {
      element_sprite->setNonUnique(true, build_sprite->getSize());
      element_sprite->createTexture(renderer);

      return true;
    }
  }

  return false;
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

void RenderElement::createAsActionText(std::string action_name)
{
  text_string = action_name;
  color = {0, 0, 0, 255};
  setShadow({kACTION_COLOR_R, 0, 0, 255}, kACTION_SHADOW, kACTION_SHADOW);
  setTimes(1000, 150, 150);
  render_type = RenderType::ACTION_TEXT;
  status = initialStatusFade();

  if(element_font && renderer)
  {
    element_text = Text(element_font);
    element_text.setText(renderer, action_name, color);
    location.point.x = kACTION_TEXT_X - element_text.getWidth();
    location.point.y = kACTION_CENTER - element_text.getHeight() / 2 - 8;
  }
}

void RenderElement::createAsDamageText(std::string text, DamageType type,
                                       int32_t sc_height, int32_t x, int32_t y)
{
  text_string = text;
  color = {0, 0, 0, 255};
  setShadow(colorFromDamageType(type), kDAMAGE_SHADOW, kDAMAGE_SHADOW - 1);
  setTimes(650, 300, 300);
  status = initialStatusFade();
  render_type = RenderType::DAMAGE_TEXT;

  setAcceleration(0.000, 0.000);
  setVelocity(0.005, -0.020);

  if(element_font && renderer)
  {
    element_text = Text(element_font);
    element_text.setText(renderer, text, color);

    auto half_w = element_text.getWidth() / 2;
    location.point.x = x + (Battle::kPERSON_WIDTH / 2) - half_w;
    location.point.y = y + half_w + (sc_height / 13);
  }
}

void RenderElement::createAsDamageValue(int32_t amount, DamageType type,
                                        int32_t sc_height, int32_t x, int32_t y)
{
  text_string = std::to_string(amount);
  createAsDamageText(text_string, type, sc_height, x, y);

  render_type = RenderType::DAMAGE_VALUE;
}

void RenderElement::createAsRegenValue(int32_t amount, DamageType type,
                                       int32_t sc_height, int32_t x, int32_t y)
{
  createAsDamageText(std::to_string(amount), type, sc_height, x, y);
  setTimes(650, 300, 300);
  status = initialStatusFade();
  setAcceleration(0.000, -0.0001);
  setVelocity(0.000, 0.001);
  render_type = RenderType::DAMAGE_VALUE;
}

void RenderElement::createAsEnterText(std::string text, int32_t sc_height,
                                      int32_t sc_width)
{
  text_string = text;
  color = {14, 14, 15, 255};
  setShadow({194, 59, 34, 255}, kACTION_SHADOW + 1, kACTION_SHADOW + 1);
  setTimes(2000, 450, 450);
  status = initialStatusFade();
  render_type = RenderType::ENTER_TEXT;

  if(element_font && renderer)
  {
    element_text = Text(element_font);
    element_text.setText(renderer, text, color);
    location.point.x = (sc_width - element_text.getWidth()) / 2;
    location.point.y = sc_height / 2 - (element_text.getHeight() / 2);

  }
}

void RenderElement::createAsRGBOverlay(SDL_Color color, int32_t overlay_time,
                                       int32_t fade_in_time,
                                       int32_t fade_out_time, int32_t sc_height,
                                       int32_t sc_width)
{
  this->color = color;
  setTimes(overlay_time, fade_in_time, fade_out_time);
  location.point.x = 0;
  location.point.y = 0;
  location.width = sc_width;
  location.height = sc_height;
  status = initialStatusFade();
  render_type = RenderType::RGB_OVERLAY;
}

void RenderElement::createAsSpriteDeath(BattleActor* target, SDL_Color color,
                                        int32_t death_time,
                                        int32_t fade_in_time,
                                        int32_t fade_out_time)
{
  setTimes(death_time, fade_in_time, fade_out_time);
  status = initialStatusFade();
  this->color = color;
  this->target = target;
  render_type = RenderType::RGB_SPRITE_DEATH;
}

void RenderElement::createAsSpriteFlash(BattleActor* target, SDL_Color color,
                                        int32_t flash_time)
{
  auto fade_time = std::floor(flash_time * 3.00 / 7.00);
  setTimes(flash_time, fade_time, fade_time);
  status = initialStatusFade();
  this->color = color;
  this->target = target;
  render_type = RenderType::RGB_SPRITE_FLASH;
}

void RenderElement::setShadow(SDL_Color shadow_color, int32_t offset_x,
                              int32_t offset_y)
{
  this->shadow_color = shadow_color;
  this->has_shadow = true;
  shadow_offset.x = offset_x;
  shadow_offset.y = offset_y;
}

bool RenderElement::setTimes(int32_t time_begin, int32_t time_fade_in,
                             int32_t time_fade_out)
{

  auto valid = false;

  if(time_fade_in > 0 && time_fade_out == 0)
    valid &= time_fade_in <= time_begin;
  else if(time_fade_out > 0 && time_fade_in == 0)
    valid &= time_fade_out <= time_begin;
  else if(time_fade_in > 0 && time_fade_out > 0)
    valid &= time_fade_in + time_fade_out <= time_begin;

  this->time_begin = time_begin;
  this->time_left = time_begin;
  this->time_fade_in = time_fade_in;
  this->time_fade_out = time_fade_out;

  status = initialStatusFade();

  return valid;
}

void RenderElement::setVelocity(float velocity_x, float velocity_y)
{
  velocity.x = velocity_x;
  velocity.y = velocity_y;
}

void RenderElement::setAcceleration(float acceleration_x, float acceleration_y)
{
  acceleration.x = acceleration_x;
  acceleration.y = acceleration_y;
}

bool RenderElement::update(int32_t cycle_time)
{
  time_left -= cycle_time;

  if(render_type == RenderType::PLEP)
    updateStatusPlep(cycle_time);
  else
    updateStatusFade(cycle_time);

  return false;
}

void RenderElement::updateStatusPlep(int32_t cycle_time)
{
  if(element_sprite)
  {
    element_sprite->update(cycle_time);

    if(element_sprite->getLoops() >= loops_to_do)
      status = RenderStatus::TIMED_OUT;
  }
}

void RenderElement::updateStatusFade(int32_t cycle_time)
{
  if(time_left <= 0)
  {
    status = RenderStatus::TIMED_OUT;
  }
  else
  {
    if((time_begin - time_left) >= time_fade_in)
      status = RenderStatus::DISPLAYING;
    if(time_left < time_fade_out)
      status = RenderStatus::FADING_OUT;

    velocity.x += (acceleration.x * cycle_time);
    velocity.y += (acceleration.y * cycle_time);

    delta.x += velocity.x * cycle_time;
    delta.y += velocity.y * cycle_time;

    if(std::abs(delta.x) >= 1.00)
    {
      auto neg_delta_x = std::floor(delta.x);

      location.point.x += neg_delta_x;
      delta.x -= neg_delta_x;
    }

    if(std::abs(delta.y) >= 1.00)
    {
      auto neg_delta_y = std::floor(delta.y);

      location.point.y += neg_delta_y;
      delta.y -= neg_delta_y;
    }

    if(status == RenderStatus::FADING_IN)
    {
      float delta_a = color.a * 1.0 / time_fade_in * cycle_time;
      delta_a = std::max(1.0, (double)delta_a);

      if(alpha + delta_a > color.a)
        alpha = color.a;
      else
        alpha += delta_a;
    }
    else if(status == RenderStatus::DISPLAYING)
    {
      alpha = color.a;
    }
    else if(status == RenderStatus::FADING_OUT && time_fade_out != 0)
    {
      float delta_a = color.a * 1.0 / time_fade_out * cycle_time;
      delta_a = std::max(1.0, (double)delta_a);

      if(delta_a > alpha)
        alpha = 0;
      else if(alpha - delta_a >= 0)
        alpha -= delta_a;
    }
  }
}

RenderStatus RenderElement::initialStatusFade()
{
  if(time_fade_in > 0)
    return RenderStatus::FADING_IN;
  else if(time_fade_in <= 0 && time_fade_out == time_left)
    return RenderStatus::FADING_OUT;
  else if(time_begin > 0)
    return RenderStatus::DISPLAYING;

  return RenderStatus::TIMED_OUT;
}

/*=============================================================================
 * PRIVATE STATIC FUNCTIONS
 *============================================================================*/

SDL_Color RenderElement::colorFromDamageType(DamageType type)
{
  if(type == DamageType::CRITICAL)
    return kCRIT_DMG_COLOR;
  else if(type == DamageType::POISON)
    return kPOIS_DMG_COLOR;
  else if(type == DamageType::HEALING)
    return kHEAL_DMG_COLOR;
  else if(type == DamageType::BURN)
    return kBURN_DMG_COLOR;
  else if(type == DamageType::VITA_REGEN)
    return kVITA_REGEN_COLOR;
  else if(type == DamageType::QTDR_REGEN)
    return kQTDR_REGEN_COLOR;
  else if(type == DamageType::HIBERNATION_REGEN)
    return kHIBERNATION_REGEN_COLOR;

  return kSTRD_DMG_COLOR;
}
