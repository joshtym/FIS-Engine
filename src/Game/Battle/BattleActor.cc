/*******************************************************************************
* Class Name: BattleActor [Implementation]
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
#include "Game/Battle/BattleActor.h"

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
BattleActor::BattleActor(Person* person_base, int32_t battle_index,
                         bool is_ally, bool can_run, SDL_Renderer* renderer)
    : active_sprite{ActiveSprite::NONE},
      battle_index{battle_index},
      flags{static_cast<ActorState>(0)},
      person_base{person_base},
      sprite_first_person{nullptr},
      sprite_third_person{nullptr},
      sprite_action{nullptr},
      sprite_dialog{nullptr},
      state_action_frame{FadeState::NOT_SHOWN},
      state_active_sprite{SpriteState::HIDDEN},
      state_death_fade{FadeState::NOT_SHOWN},
      state_elapsed_time{0},
      state_guarding{GuardingState::NONE},
      state_selection{SelectionState::NOT_SELECTED},
      state_upkeep{UpkeepState::COMPLETE},
      stats_actual{AttributeSet()},
      stats_rendered{AttributeSet()},
      temp_alpha{255},
      dialog_x{0},
      dialog_y{0}
{
  /* The base person must be a non-nullptr person */
  assert(person_base);

  /* Prepare the person for a Battle */
  battleSetup(is_ally, can_run);
  createSprites(renderer);
}

/*
 * Description: Annihilates a BattleActor object
 */
BattleActor::~BattleActor()
{
  clearAilments();
  clearBattleItems();
  clearBattleSkills();
  clearInfoFrame();
}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
void BattleActor::battleSetup(bool is_ally, bool can_run)
{
  /* Recalculate the current stats of the base person, create BattleStats */
  auto equip_stats = person_base->calcEquipStats();
  stats_actual = BattleStats(equip_stats);
  stats_rendered = BattleStats(equip_stats);

  /* Recalculate the skills the person has available at the moment */
  // person_base->updateSkills();

  /* Reset the 'silenced' flag on skills */
  //person_base->resetSkills();

  /* If the person's current VITA is >0, they are not KO'd */
  setFlag(ActorState::KO,
          (person_base->getCurr().getStat(Attribute::VITA) > 0));
  setFlag(ActorState::ALIVE, true);
  setFlag(ActorState::REVIVABLE, true);
  setFlag(ActorState::RUN_ENABLED, true);

  /* Set up for allies */
  if(is_ally)
  {
    setFlag(ActorState::ALLY, true);
    active_sprite = ActiveSprite::THIRD_PERSON;
  }
  /* Set up for enemies */
  else
  {
    active_sprite = ActiveSprite::FIRST_PERSON;
  }

  /* Reset the action types useable by the battle actor */
  resetActionTypes();

  // TODO
  (void)can_run;
}

void BattleActor::clearAilments()
{
  for(auto& clear_ailment : ailments)
  {
    if(clear_ailment)
      delete clear_ailment;
    clear_ailment = nullptr;
  }

  ailments.clear();
}

void BattleActor::clearBattleItems()
{
  for(auto& battle_item : battle_items)
  {
    if(battle_item)
      delete battle_item;
    battle_item = nullptr;
  }

  battle_items.clear();
}

void BattleActor::clearBattleSkills()
{
  for(auto& battle_skill : battle_skills)
  {
    if(battle_skill)
      delete battle_skill;
    battle_skill = nullptr;
  }

  battle_skills.clear();
}

void BattleActor::clearInfoFrame()
{
  if(frame_info)
    delete frame_info;
  frame_info = nullptr;
}

void BattleActor::clearSprites()
{
  if(sprite_action)
    delete sprite_action;
  if(sprite_first_person)
    delete sprite_first_person;
  if(sprite_third_person)
    delete sprite_third_person;

  sprite_action = nullptr;
  sprite_first_person = nullptr;
  sprite_third_person = nullptr;
}

void BattleActor::createSprites(SDL_Renderer* renderer)
{
  if(person_base)
  {
    /* Build the action sprite */
    if(person_base->getActionSprite())
    {
      sprite_action = new Sprite(*(person_base->getActionSprite()));

      if(sprite_action)
      {
        sprite_action->setNonUnique(true,
                                    person_base->getActionSprite()->getSize());
        sprite_action->createTexture(renderer);
      }
    }

    /* Build the first person sprite, if one exists */
    if(person_base->getFirstPerson())
    {
      sprite_first_person = new Sprite(*(person_base->getFirstPerson()));

      if(sprite_first_person)
      {
        sprite_first_person->setNonUnique(
            true, person_base->getFirstPerson()->getSize());
        sprite_first_person->createTexture(renderer);
      }
    }

    /* Build the third person sprite, if one exists */
    if(person_base->getThirdPerson())
    {
      sprite_third_person = new Sprite(*(person_base->getThirdPerson()));

      if(sprite_third_person)
      {
        sprite_third_person->setNonUnique(
            true, person_base->getThirdPerson()->getSize());
        sprite_third_person->createTexture(renderer);
      }
    }
  }
}

bool BattleActor::resetActionTypes()
{
  if(person_base && person_base->getClass())
  {
    // TODO [09-06-15] Action types temporarily offline.
    setFlag(ActorState::ITM_ENABLED, false); // Build item functionality
    setFlag(ActorState::INS_ENABLED, false); // Build inspect functionality
    setFlag(ActorState::RUN_ENABLED, false); // Get from enemy party
    setFlag(ActorState::GRD_ENABLED, false); // Get from category

    /* Build defend and implode based on category */
    setFlag(ActorState::DEF_ENABLED,
            person_base->getClass()->getFlag(CategoryState::DEF_ENABLED));
    setFlag(ActorState::IMP_ENABLED,
            person_base->getClass()->getFlag(CategoryState::IMP_ENABLED));

    /* Everyone can pass */
    setFlag(ActorState::PAS_ENABLED, true);

    return true;
  }

  return false;
}

void BattleActor::updateBrightness(int32_t cycle_time)
{
  (void)cycle_time;
  // auto layer_index = battle->getBattleMenu()->getLayerIndex();
  // auto person_index = battle->getIndexOfPerson(test_person);
  // auto person_state = getState(test_person);
  // auto brightness = 1.0;

  // if(rendering_state == TurnState::SELECT_ACTION_ALLY)
  // {
  //   if(layer_index == 1 || layer_index == 2)
  //   {
  //     brightness = 0.25;

  //     if(test_person->getBFlag(BState::IS_SELECTING))
  //     {
  //       brightness = 0.3 * sin((float)person_state->elapsed_time *
  //       kCYCLE_RATE);
  //       brightness = abs(brightness) + 0.7;
  //     }
  //   }
  //   else if(layer_index == 3 || layer_index == 4)
  //   {
  //     auto hover_targets = battle->getBattleMenu()->getHoverTargets();
  //     auto h_it =
  //         std::find(begin(hover_targets), end(hover_targets), person_index);
  //     bool is_hovered = (h_it != end(hover_targets));

  //     auto selected = battle->getBattleMenu()->getActionTargets();
  //     auto s_it = std::find(begin(selected), end(selected), person_index);
  //     bool is_selected = (s_it != end(selected));

  //     if(is_hovered || is_selected)
  //       brightness = 1.0;
  //     else
  //       brightness = 0.25;

  //     if(layer_index == 4)
  //     {
  //       if(!is_selected)
  //         brightness = 0.25;
  //       else
  //         brightness = 1.0;
  //     }
  //   }
  // }
  // else
  // {
  //   brightness = 1.0;
  // }

  // return brightness;
}

void BattleActor::updateOpacity(int32_t cycle_time)
{
  (void)cycle_time;
  // uint8_t opacity = 0;
  // auto state = getState(test_person);

  // if(test_person && state)
  // {
  //   if(!test_person->getBFlag(BState::ALIVE))
  //   {
  //     if(state)
  //       state->dying = false;

  //     opacity = kPERSON_KO_ALPHA;
  //   }
  //   else if(state && state->dying)
  //   {
  //     auto alpha = getPersonSprite(test_person)->getOpacity();
  //     float alpha_diff = alpha * 1.0 / 1000 * cycle_time;

  //     alpha_diff = std::max(1.0, (double)alpha_diff);

  //     if(alpha_diff > alpha)
  //       opacity = 50;
  //     else if(alpha - alpha_diff >= 50)
  //       opacity = alpha - alpha_diff;
  //   }
  //   else
  //   {
  //     opacity = 255;
  //   }

  //   if(test_person->getBFlag(BState::IS_SELECTING))
  //   {
  //     auto sprite = getPersonSprite(test_person);

  //     if(sprite)
  //     {
  //        If the person is selecting and not set cycling, set cycling
  //       if(!state->cycling)
  //       {
  //         state->temp_alpha = sprite->getOpacity();
  //         state->cycling = true;
  //       }
  //       /* Else, if they are already cycling -> update opacity of sprite */
  //       else
  //       {
  //         state->elapsed_time += cycle_time;

  //         uint8_t new_alpha =
  //             abs(100 * sin((float)state->elapsed_time * kCYCLE_RATE));
  //         opacity = new_alpha + 155;
  //       }
  //     }
  //   }
  //   else
  //   {
  //     /* If the person is not selecting and their state is set to cycling,
  //      * reload the original alpha into their sprite and unset cycling */
  //     if(state->cycling)
  //     {
  //       opacity = state->temp_alpha;
  //       state->cycling = false;
  //       state->elapsed_time = 0;
  //     }
  //   }
  // }

  // return opacity;
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

// TODO: Build battle Items [09-07-15]
bool BattleActor::buildBattleItems(std::vector<BattleActor*> all_targets)
{
  (void)all_targets;
  bool success = person_base;

  return success;
}

bool BattleActor::buildBattleSkills(std::vector<BattleActor*> all_targets)
{
  (void)all_targets;
  // /* Clear BattleSkills if they were previously created */
  // clearBattleSkills();

  // bool success = person_base;

  // if(success)
  // {
  //   auto useable_skills = person_base->getUseableSkills()->getElements();

  //   for(auto use_skill : useable_skills)
  //   {
  //     auto battle_skill = new BattleSkill();

  //     if(use_skill && battle_skill)
  //     {
  //       battle_skill->targets =
  //           getTargetsFromScope(this, use_skill->getScope(), all_targets);

  //       battle_skill->true_cost = getSkillCost(use_skill);

  //       /* At first, the valid skill is valid if it has a target */
  //       if(battle_skill->targets.size() > 0)
  //       {
  //         battle_skill->valid_status = ValidStatus::VALID;

  //         /* Silenced Skill */
  //         if(isInflicted(Infliction::SILENCE))
  //           battle_skill->valid_status = ValidStatus::SILENCED;

  //         /* Check if the person can afford to use the skill -- current stats */
  //         if(stats_actual.getValue(Attribute::QTDR) > battle_skill->true_cost)
  //           battle_skill->valid_status = ValidStatus::NOT_AFFORDABLE;
  //       }
  //       else
  //         battle_skill->valid_status = ValidStatus::NO_TARGETS;
  //     }
  //     else
  //     {
  //       delete battle_skill;
  //     }
  //   }
  // }

  return true;
}

bool BattleActor::isInflicted(Infliction test_infliction)
{
  for(const auto& ailment : ailments)
    if(ailment && ailment->getType() == test_infliction)
      return true;

  return false;
}

bool BattleActor::removeAilment(Ailment* remove_ailment)
{
  (void)remove_ailment;

  return false;
  // TODO: [09-06-15] Removing ailments
}

void BattleActor::turnSetup()
{
  state_selection = SelectionState::NOT_SELECTED;

  if(person_base && person_base->getAI())
  {
    // person_base->getAI()->incrementTurns();
    // person_base->getAI()->resetForNewTurn(person_base);
  }
}

bool BattleActor::update(int32_t cycle_time)
{
  (void)cycle_time;
  // TODO: [09-06-15] Update code
  return true;
}

Sprite* BattleActor::getActiveSprite()
{
  if(active_sprite == ActiveSprite::FIRST_PERSON)
    return sprite_first_person;
  if(active_sprite == ActiveSprite::THIRD_PERSON)
    return sprite_third_person;
  if(active_sprite == ActiveSprite::ACTION)
    return sprite_action;

  return nullptr;
}

std::vector<Ailment*> BattleActor::getAilments()
{
  return ailments;
}

Person* BattleActor::getBasePerson()
{
  return person_base;
}

bool BattleActor::getFlag(const ActorState& test_flag)
{
  return static_cast<bool>((flags & test_flag) == test_flag);
}

int32_t BattleActor::getIndex()
{
  return battle_index;
}

Frame* BattleActor::getInfoFrame()
{
  return frame_info;
}

int32_t BattleActor::getDialogX()
{
  return dialog_x;
}

int32_t BattleActor::getDialogY()
{
  return dialog_y;
}

// TODO: Bubbification cost
// TODO: Halfcost infliction cost
uint32_t BattleActor::getSkillCost(Skill* test_skill)
{
  if(test_skill)
    return test_skill->getCost();

  return AttributeSet::getMaxValue();
}

void BattleActor::setActiveSprite(ActiveSprite new_active_sprite)
{
  active_sprite = new_active_sprite;
}

void BattleActor::setFlag(ActorState set_flags, const bool& set_value)
{
  (set_value) ? (flags |= set_flags) : (flags &= ~set_flags);
}

void BattleActor::setInfoFrame(Frame* frame_info)
{
  clearInfoFrame();
  this->frame_info = frame_info;
}

/*=============================================================================
 * PUBLIC STATIC FUNCTIONS
 *============================================================================*/
std::vector<BattleActor*>
BattleActor::getTargetsFromScope(BattleActor* user, ActionScope scope,
                                 std::vector<BattleActor*> targets)
{
  std::vector<BattleActor*> valid_targets;
  (void)user;
  (void)scope;
  (void)targets;

  return valid_targets;
}