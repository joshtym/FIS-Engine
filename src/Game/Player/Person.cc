/*******************************************************************************
* Class Name: Person [Implementation]
* Date Created: 
* Inheritance: 
* Description:
*
* Notes
* -----
*
* [1]: AttributeSets outside of and during Battle:
*
* In & Out of Battle
* -------------
* base_stats - the level 1 stats of the Person calculated by battle_class + race
* max_base_stats - the level max stats of the Person calc by battle_class + race
* 
* Out of Battle
* -------------
* curr_sats      - unused
* curr_max_stats - normal stats between base and base max at the person's level
* temp           - curr_max_stats + equipmemt
*
* In Battle
* ---------
* curr_stats     - displayed value of stats after Battle modifiers
* curr_max_stats - possible reachable stat values after Battle modifiers
* temp           - the normal value of stats before Batle modifiers
*
* See .h file for TODOs
*******************************************************************************/

#include "Game/Player/Person.h"

/*=============================================================================
 * CONSTANTS
 *============================================================================*/
const size_t   Person::kNUM_LEVELS{127};     
const size_t   Person::kNUM_EQUIP_SLOTS{5}; 
const uint32_t Person::kMAX_CREDIT_DROP{1000000}; /* 1 million */
const uint32_t Person::kMAX_EXP{1000000000}; /* 1 billion */       
const uint32_t Person::kMAX_EXP_DROP{1000000}; /* 1 million */
const size_t   Person::kMAX_ITEM_DROPS{5};
const uint32_t Person::kMAX_LVL_EXP{100000000}; /* 100 million */   
const uint32_t Person::kMIN_EXP{0};   
const uint32_t Person::kMIN_LVL_EXP{10};   
const float Person::kMIN_DMG_MODI{ 0.01};    
const float Person::kMAX_DMG_MODI{10.00};
const float Person::kMIN_EXP_MODI{ 0.10};
const float Person::kMAX_EXP_MODI{10.00}; 

/* Static ID counter */
int Person::id{0};

std::vector<uint32_t> Person::exp_table{};

/* XS, S, A, B, C, D */
const std::vector<float> Person::kPRIM_MODS =
{ 1.425, 1.400, 1.375, 1.350, 1.325, 1.300};

const std::vector<float> Person::kSECD_MODS =
{ 1.375, 1.300, 1.250, 1.225, 1.200, 1.175};

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description:
 *
 * Inputs:
 */
Person::Person(const int32_t &game_id, const std::string &name, 
               Category* const battle_class, Category* const race_class)
  : game_id{game_id}
  , my_id{++id}
  , base_person{nullptr}
  , battle_class{battle_class}
  , race_class{race_class}
  , name{name}
{
  setupClass();
}

/*
 * Description:
 *
 * Inputs:
 */
Person::Person(Person* const source)
  : game_id{source->game_id}
  , my_id{++id}
  , base_person{source}
{
  setupClass();
}

/*
 * Description:
 */
Person::~Person()
{
  if (base_person != nullptr)
  {
    /* Delete the equipments that were created from the base person */
    for (auto equipment : equipments)
    {
      if (equipment != nullptr)
      {
        if (!(equipment->getEquipFlag(EquipState::TWO_HANDED) &&
            equipment == getEquip(EquipSlots::RARM)))
        {
          delete equipment;
        }

        equipment = nullptr;
      }
    }
  }
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

/* Sets up the class, based on whether base_person is assigned or not */
void Person::setupClass()
{
  if (exp_table.empty())
    buildExpTable();

  /* Setup the class for a Base Person */
  if (base_person == nullptr)
  {
    battle_flags = static_cast<BState>(0);
    person_flags = static_cast<PState>(0);
    //person_record{};

    updateRank();

    primary = Element::NONE;
    secondary = Element::NONE;
    primary_curve = ElementCurve::D;
    secondary_curve = ElementCurve::D;

    total_exp = kMIN_LVL_EXP;
    updateLevel();
    updateBaseStats();

    curr_stats     = base_stats;
    curr_max_stats = base_stats;
    temp_max_stats = base_stats;

    learned_skills = SkillSet();
    updateBaseSkills();
    curr_skills = base_skills;

    dmg_mod = 1.000;
    exp_mod = 1.000;

    for (size_t a = 0; a < kNUM_EQUIP_SLOTS; a++)
      equipments.push_back(nullptr);

    credit_drop = 0;
    exp_drop = 0;

    first_person = nullptr;
    third_person = nullptr;
    fp_bubbified_sprite = nullptr;
    tp_bubbified_sprite = nullptr;
  }

  /* Setup the class as a copy of the Base Person */
  else
  {
    battle_flags = base_person->battle_flags;
    person_flags = base_person->person_flags;
    //person_record = base-person->person_record;
    battle_class = base_person->battle_class;
    race_class = base_person->race_class;
    name = base_person->name;
    rank = base_person->rank;
    primary = base_person->primary;
    secondary = base_person->secondary;
    primary_curve = base_person->primary_curve;
    secondary_curve = base_person->secondary_curve;
    base_stats = base_person->base_stats;
    base_max_stats = base_person->base_max_stats;
    curr_stats = base_person->curr_stats;
    curr_max_stats = base_person->curr_max_stats;
    temp_max_stats = base_person->temp_max_stats;
    base_skills = base_person->base_skills;
    curr_skills = base_person->curr_skills;
    learned_skills = base_person->learned_skills;
    dmg_mod = base_person->dmg_mod;
    exp_mod = base_person->exp_mod;

    /* Equipments are deep-copied */
    for (auto equipment : base_person->equipments)
    {
      if (equipment != nullptr)
      {
        auto new_equip = new Equipment(equipment);
        equipments.push_back(new Equipment(new_equip));
      }
      else
        equipments.push_back(nullptr);
    }

    item_drops = base_person->item_drops;
    credit_drop = base_person->credit_drop;
    exp_drop = base_person->exp_drop;
    level = base_person->level;
    total_exp = base_person->total_exp;

    first_person = base_person->first_person;
    third_person = base_person->third_person;
    fp_bubbified_sprite = base_person->fp_bubbified_sprite;
    tp_bubbified_sprite = base_person->tp_bubbified_sprite;
  }
}

/* Recalculates the Person's base and base_max stats based on categories */
void Person::updateBaseStats()
{
  AttributeSet temp = battle_class->getBaseSet() + race_class->getBaseSet();
  AttributeSet temp_max = battle_class->getTopSet() + race_class->getTopSet();

  std::vector<int32_t> prim_indexes;
  std::vector<int32_t> secd_indexes;
  
  prim_indexes.push_back(AttributeSet::getOffensiveIndex(primary));
  prim_indexes.push_back(AttributeSet::getDefensiveIndex(primary));
  secd_indexes.push_back(AttributeSet::getOffensiveIndex(secondary));
  secd_indexes.push_back(AttributeSet::getDefensiveIndex(secondary));

  auto p_mod = getCurveModifier(primary_curve, true);
  auto s_mod = getCurveModifier(secondary_curve, false);

  for (auto index : prim_indexes)
  {
    if (index != -1)
    {
      auto a = std::floor(static_cast<float>(temp.getStat(index)) * p_mod);
      auto b = std::floor(static_cast<float>(temp_max.getStat(index)) * p_mod);

      temp.setStat(index, a);
      temp_max.setStat(index, b);
    }
  }
  for (auto index : secd_indexes)
  {
    if (index != -1)
    {
      auto a = std::floor(static_cast<float>(temp.getStat(index)) * s_mod);
      auto b = std::floor(static_cast<float>(temp_max.getStat(index)) * s_mod);

      temp.setStat(index, a);
      temp_max.setStat(index, b);
    }
  }

  temp.cleanUp();
  temp_max.cleanUp();

  base_stats = temp;
  base_max_stats = temp_max;

  updateStats();
}

void Person::updateBaseSkills()
{
  base_skills.clear();
  base_skills = SkillSet();

  if (battle_class->getSkills() != nullptr)
    base_skills += *(battle_class->getSkills());
  if (race_class->getSkills() != nullptr)
    base_skills += *(race_class->getSkills());

  updateSkills();
}

/* Updates the level of the person based on their current total experience */
void Person::updateLevel()
{
  if (getPFlag(PState::CAN_LEVEL_UP))
  {
    auto before = level;

    level = getLevelAt(total_exp);

    if (level == kNUM_LEVELS)
      setPFlag(PState::MAX_LVL);

    if (level != before)
      updateStats();
  }
}

/* Updates the current stats of the Person based on their current level */
void Person::updateStats()
{
  if (level == 1)
  {
    curr_stats = base_stats;
    curr_max_stats = base_stats;
  }
  else if (level == kNUM_LEVELS)
  {
    curr_stats = base_max_stats;
    curr_max_stats = base_max_stats;
  }
  else
  {
    curr_stats = AttributeSet(0, true);
    curr_max_stats = AttributeSet(0, true);

    for (size_t i = 0; i < AttributeSet::getSize(); i++)
    {
      std::vector<uint32_t> stat_values{};
      stat_values = Helpers::buildExpTable(base_stats.getStat(i), 
                                           base_max_stats.getStat(i), 
                                           kNUM_LEVELS);
      curr_stats.setStat(i, stat_values.at(level));
    }

    curr_max_stats = curr_stats;
  }

  temp_max_stats = curr_max_stats;

  for (auto equipment : equipments)
    if (equipment != nullptr)
      temp_max_stats += equipment->getStats();

  curr_stats.cleanUp();
  curr_max_stats.cleanUp();
  temp_max_stats.cleanUp();
}

void Person::updateSkills()
{
  curr_skills.clear();
  curr_skills  = learned_skills;
  curr_skills += base_skills;

  for (auto equipment : equipments)
    if (equipment != nullptr)
      curr_skills += equipment->getSkills();
}

/* Updates the rank of the Person based on their Person record */
void Person::updateRank()
{
  //rank = person_record.getRank();
}

/* Constructs the table of experience values given the current constants */
void Person::buildExpTable()
{
  exp_table = Helpers::buildExpTable(kMIN_LVL_EXP, kMAX_LVL_EXP, kNUM_LEVELS);
}

/* Returns the curve modifier given a curve value and whether to check prim */
float Person::getCurveModifier(const ElementCurve &curve, 
                               const bool primary)
{
  auto index = static_cast<uint8_t>(curve);

  if (primary && index < kPRIM_MODS.size())
    return kPRIM_MODS.at(index);

  if (index < kSECD_MODS.size())
    return kSECD_MODS.at(index);

  return 0;
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/* Adds an amount of experience and may update the level */
bool Person::addExp(const uint32_t &amount, const bool &update)
{
  auto can_add = false;

  if (getPFlag(PState::CAN_GAIN_EXP))
  {
    if (total_exp + amount < kMAX_EXP)
    { 
      total_exp += amount;
      can_add = true;
    }
    else if (total_exp != kMAX_EXP)
    {
      total_exp = kMAX_EXP;
      can_add = true;
    }

    for (auto equipment : equipments)
      if (equipment != nullptr)
        equipment->getSignature()->addExp(amount);
  }
  
  if (can_add && update)
    updateLevel();

  return can_add;
}

/* Prepares the person for entering Battle (flags, Attributes etc.) */
void Person::battlePrep()
{
  curr_stats = curr_max_stats;
  temp_max_stats = curr_max_stats;
  
  setBFlag(BState::IN_BATTLE, true);
  setBFlag(BState::ALIVE, true);
  setBFlag(BState::ATK_ENABLED, true);
  setBFlag(BState::SKL_ENABLED, true);
  setBFlag(BState::ITM_ENABLED, true);

  // TODO - BState flags - Run enabled false for mini-bosses? [12-23-13]
  //      - Def, Grd, Imp enabled by class?
  setBFlag(BState::DEF_ENABLED, true);
  setBFlag(BState::GRD_ENABLED, true);
  setBFlag(BState::IMP_ENABLED, true);
  setBFlag(BState::RUN_ENABLED, true);
  setBFlag(BState::PAS_ENABLED, true);

  setBFlag(BState::SKIP_NEXT_TURN, false);
  setBFlag(BState::MISS_NEXT_TARGET, false);
  setBFlag(BState::NEXT_ATK_NO_EFFECT, false);
  setBFlag(BState::IS_BUBBY, false);
  setBFlag(BState::TWO_SKILLS, false);
  setBFlag(BState::THREE_SKILLS, false);
  setBFlag(BState::HALF_COST, false);
  setBFlag(BState::REFLECT, false);
  setBFlag(BState::BOND, false);
  setBFlag(BState::BONDED, false);
  setBFlag(BState::REVIVABLE, false);
}

/* Clears the skills the player has learned */
void Person::clearLearnedSkills()
{
  learned_skills.clear();
  updateSkills();
}

/* Shorthand function for dealing damage, returns true if the Person KO's */
bool Person::doDmg(const uint32_t &amount)
{
  curr_stats.alterStat(Attribute::VITA, -amount);

  if (curr_stats.getStat(Attribute::VITA) == 0)
  {
    setBFlag(BState::ALIVE, false);

    return true;
  }

  return false;
}

/* Method for printing out the data of the class */
void Person::print(const bool &simple, const bool &equips,
                   const bool &flags, const bool &skills)
{
  if (simple)
  {
    std::cout << "GID: " << game_id << " MID: " << my_id << " Name: " << name
              << " Level: " << level << " Exp: " << total_exp << "\n";
  }
  else
  {
    std::cout << "Game ID: " << game_id << "\n";
    std::cout << "My ID: " << my_id << "\n";
    std::cout << "Base Person?" << (base_person != nullptr) << "\n";
    std::cout << "Battle Class: " << battle_class->getName() << "\n";
    std::cout << "Race? " << race_class->getName() << "\n";
    std::cout << "Name: " << name << "\n";
    std::cout << "[void]Rank" << "\n";
    std::cout << "Primary: " << Helpers::elementToString(primary) << "\n";
    std::cout << "Secondary: " << Helpers::elementToString(secondary) << "\n";
    std::cout << "[void]Prim Curve: " << "\n";
    std::cout << "[void]Secd Curve: " << "\n";
    
    //std::cout << "Base Stats: " << base_stats.print(true) << "\n";
    //std::cout << "Base Max: " << base_stats.print(true) << "\n";
    //std::cout << "Curr Stats: " << curr_stats.print(true) << "\n";
    //std::cout << "Curr Max Stats: " << curr_max_stats.print(true) << "\n";
    //std::cout << "Temp Max Stats: ";
    //temp_max_stats.print(true);
    //std::cout << "\n";

    std::cout << "Dmg Modifier: " << dmg_mod << "\n";
    std::cout << "Exp Modifier: " << exp_mod << "\n";
    std::cout << "Item Drops: " << item_drops.size() << "\n";
    std::cout << "Credit Drop: " << credit_drop << "\n";
    std::cout << "Exp Drop: " << exp_drop << "\n";
    std::cout << "Level: " << level << "\n";
    std::cout << "Total Exp: " << total_exp << "\n";
    std::cout << "First Person? " << (first_person != nullptr) << "\n";
    std::cout << "Third Person? " << (third_person != nullptr) << "\n";
    std::cout << "Fp Bubbified? " << (fp_bubbified_sprite != nullptr) << "\n";
    std::cout << "Tp Bubbified? " << (tp_bubbified_sprite != nullptr) << "\n";

    if (skills)
    {
      base_skills.print();
      curr_skills.print();
      learned_skills.print();
    }

    if (equips)
      for (auto equipment : equipments)
        if (equipment != nullptr)
          equipment->print();

    if (flags)
    {
      std::cout << "IN_BATTLE: " << getBFlag(BState::IN_BATTLE) << "\n";
      std::cout << "ALIVE: " << getBFlag(BState::ALIVE) << "\n";
      std::cout << "ATK_ENABLED: " << getBFlag(BState::ATK_ENABLED) << "\n";
      std::cout << "SKL_ENABLED: " << getBFlag(BState::SKL_ENABLED) << "\n";
      std::cout << "ITM_ENABLED: " << getBFlag(BState::ITM_ENABLED) << "\n";
      std::cout << "DEF_ENABLED: " << getBFlag(BState::DEF_ENABLED) << "\n";
      std::cout << "GRD_ENABLED: " << getBFlag(BState::GRD_ENABLED) << "\n";
      std::cout << "IMP_ENABLED: " << getBFlag(BState::IMP_ENABLED) << "\n";
      std::cout << "RUN_ENABLED: " << getBFlag(BState::RUN_ENABLED) << "\n";
      std::cout << "PAS_ENABLED: " << getBFlag(BState::PAS_ENABLED) << "\n";
      std::cout << "SKIP_NEXT_TURN: " << getBFlag(BState::SKIP_NEXT_TURN) 
                << "\n";
      std::cout << "MISS_NEXT_TARGET: " << getBFlag(BState::MISS_NEXT_TARGET) 
                << "\n";
      std::cout << "NEXT_ATK_NO_EFFECT: " << getBFlag(BState::NEXT_ATK_NO_EFFECT) 
                << "\n";
      std::cout << "IS_BUBBY: " << getBFlag(BState::IS_BUBBY) << "\n";
      std::cout << "TWO_SKILLS: " << getBFlag(BState::TWO_SKILLS) << "\n";
      std::cout << "THREE_SKILLS: " << getBFlag(BState::THREE_SKILLS) << "\n";
      std::cout << "HALF_COST: " << getBFlag(BState::HALF_COST) << "\n";
      std::cout << "REFLECT: " << getBFlag(BState::REFLECT) << "\n";
      std::cout << "BOND: " << getBFlag(BState::BOND) << "\n";
      std::cout << "BONDED: " << getBFlag(BState::BONDED) << "\n";
      std::cout << "REVIVABLE: " << getBFlag(BState::REVIVABLE) << "\n";
      

      std::cout << "SLEUTH: " << getPFlag(PState::SLEUTH) << "\n";
      std::cout << "BEARACKS: " << getPFlag(PState::BEARACKS) << "\n";
      std::cout << "MAIN: " << getPFlag(PState::MAIN) << "\n";
      std::cout << "FINAL: " << getPFlag(PState::FINAL) << "\n";
      std::cout << "BOSS: " << getPFlag(PState::BOSS) << "\n";
      std::cout << "MINI_BOSS: " << getPFlag(PState::MINI_BOSS) << "\n";
      std::cout << "CAN_GAIN_EXP: " << getPFlag(PState::CAN_GAIN_EXP) << "\n";
      std::cout << "CAN_LEVEL_UP: " << getPFlag(PState::CAN_LEVEL_UP) << "\n";
      std::cout << "CAN_LEARN_SKILLS: " << getPFlag(PState::CAN_LEARN_SKILLS) 
                << "\n";
      std::cout << "CAN_CHANGE_EQUIP: " << getPFlag(PState::CAN_CHANGE_EQUIP) 
                << "\n";
      std::cout << "MAX_LVL: " << getPFlag(PState::MAX_LVL) << "\n";
    }
  }
}

/* Removes the equipment from a given slot */
bool Person::removeEquip(const EquipSlots &equip_slot)
{
  auto removed = false;

  if (getEquip(equip_slot) == nullptr)
    return false;

  auto equip = getEquip(equip_slot);

  switch (equip_slot)
  {
    case (EquipSlots::LARM):
    case (EquipSlots::RARM):
      if (equip->getEquipFlag(EquipState::TWO_HANDED))
      {
        equipments.at(1) = nullptr;
        equipments.at(2) = nullptr;
      }
      else if (equip_slot == EquipSlots::LARM)
        equipments.at(1) = nullptr;
      else if (equip_slot == EquipSlots::RARM)
        equipments.at(2) = nullptr;

      removed = true;
      break;
    case (EquipSlots::HEAD):
      equipments.at(0) = nullptr;
    case (EquipSlots::BODY):
      equipments.at(3) = nullptr;
    case (EquipSlots::LEGS):
      equipments.at(4) = nullptr;
    default:
      std::cerr << "Warning: Attempting removal of invalid equip slot.\n";
  }

  return removed;
}

/* Returns the game_id (not unique) of the Person */
uint32_t Person::getGameID()
{
  return game_id;
}

/* Returns the my_id (unique) of the Person */
uint32_t Person::getMyID()
{
  return my_id;
}

/* Evaluates and returns the state of a given BState flag */
bool Person::getBFlag(const BState &test_flag)
{
  return static_cast<bool>((battle_flags & test_flag) == battle_flags); 
}

/* Evaluates and returns the state of a given PState flag */
bool Person::getPFlag(const PState &test_flag)
{
  return static_cast<bool>((person_flags & test_flag) == person_flags);
}

/* Returns a pointer to the assigned base person */
Person* Person::getBasePerson()
{
  return base_person;
}

/* Returns a pointer to the battle class category */
Category* Person::getClass()
{
  return battle_class;
}

/* Returns a pointer to the race category */
Category* Person::getRace()
{
  return race_class;
}

/* Returns the string name */
std::string Person::getName()
{
  return name;
}

/* Returns the enumerated rank of the Person */
Ranks Person::getRank()
{
  return rank;
}

/* Returns the primary elemental affiliation */
Element Person::getPrimary()
{
  return primary;
}

/* Return the secondary elemental affiliation */
Element Person::getSecondary()
{
  return secondary;
}

/* Return the enumerated curve of prim. elemental progression */
Person::ElementCurve Person::getPrimaryCurve()
{
  return primary_curve;
}

/* Return the enumerated curve of secd. elemental progression */
Person::ElementCurve Person::getSecondaryCurve()
{
  return secondary_curve;
}

/* Returns a reference to the base stats */
AttributeSet& Person::getBase()
{
  return base_stats;
}

/* Returns a ref to the base max stats */
AttributeSet& Person::getBaseMax()
{
  return base_max_stats;
}

/* Returns a ref to the curr stats */
AttributeSet& Person::getCurr()
{
  return curr_stats;
}

/* Returns a ref to the curr max stats */
AttributeSet& Person::getCurrMax()
{
  return curr_max_stats;
}

/* Returns the temp stats */
AttributeSet& Person::getTemp()
{
  return temp_max_stats;
}

/* Returns the base skills of the Person */
SkillSet& Person::getBaseSkills()
{
  return base_skills;
}

/* Returns the assigned current skills of the Person */
SkillSet& Person::getCurrSkills()
{
  return curr_skills;
}

/* Returns the damage modifier value */
float Person::getDmgMod()
{
  return dmg_mod;
}

/* Returns the experience modifier value */
float Person::getExpMod()
{
  return exp_mod; 
}

/* Returns a pointer to the equipment a given slot */
Equipment* Person::getEquip(const EquipSlots &equip_slot)
{
  switch (equip_slot)
  {
    case (EquipSlots::HEAD):
      return equipments.at(0);
    case (EquipSlots::LARM):
      return equipments.at(1);
    case (EquipSlots::RARM):
      return equipments.at(2);
    case (EquipSlots::BODY):
      return equipments.at(3);
    case (EquipSlots::LEGS):
      return equipments.at(4);
    default:
      std::cerr << "Error: Checking invalid equipment slot";
      break;
  }

  return nullptr;
}

/* Returns the credit drop reward */
uint32_t Person::getCreditDrop()
{
  return credit_drop;
}

/* Returns the exp drop reward */
uint32_t Person::getExpDrop()
{
  return exp_drop;
}

/* Returns the current level */
uint8_t Person::getLevel()
{
  return level;
}

/* Returns the total experience earned */
uint32_t Person::getTotalExp()
{
  return total_exp;
}

/* Grabs the curr first person frame (based on BUBBIFIED flags) */
Frame* Person::getFirstPerson()
{
  if (getBFlag(BState::IS_BUBBY))
    return fp_bubbified_sprite;
  return first_person;
}

Frame* Person::getThirdPerson()
{
  if (getBFlag(BState::IS_BUBBY))
    return tp_bubbified_sprite;
  return third_person;
}

/* Grabs the vector of item IDs the person can drop */
std::vector<uint32_t> Person::getItemDrops()
{
  return item_drops;
}

/* Evaluates and returns a given battle state flag */
void Person::setBFlag(const BState &flag, const bool &set_value)
{
  (set_value) ? (battle_flags |= flag) : (battle_flags &= ~flag);
}

/* Evaluates and returns a given person state flag */
void Person::setPFlag(const PState &flag, const bool &set_value)
{
  (set_value) ? (person_flags |= flag) : (person_flags &= ~flag);  
}

/* Assigns a new curr Attr set */
bool Person::setCurr(const AttributeSet& new_curr_set)
{
  if (new_curr_set.getFlag(AttributeState::PERSONAL))
  {
    curr_stats = new_curr_set;

    return true;
  }

  return false;
}

/* Assigns a new curr max Attr set */
bool Person::setCurrMax(const AttributeSet& new_curr_max_set)
{
  if (curr_max_stats.getFlag(AttributeState::PERSONAL))
  {
    curr_max_stats = new_curr_max_set;

    return true;
  }

  return false;  
}

/* Assigns a new temp attr set */
bool Person::setTemp(const AttributeSet& new_temp_set)
{
  if (new_temp_set.getFlag(AttributeState::PERSONAL))
  {
    temp_max_stats = new_temp_set;

    return true;
  }

  return false;
}

/* Assigns a new damage modifier value */
bool Person::setDmgMod(const float &new_dmg_mod)
{
  if (Helpers::isInRange(new_dmg_mod, kMIN_DMG_MODI, kMAX_DMG_MODI))
  {
    dmg_mod = new_dmg_mod;

    return true;
  }

  return false;
}

/* Assigns a new experience modifier value */
bool Person::setExpMod(const float &new_exp_mod)
{
  if (Helpers::isInRange(new_exp_mod, kMIN_EXP_MODI, kMAX_EXP_MODI))
  {
    exp_mod = new_exp_mod;

    return true;
  }

  return false;
}

/* Attempts to assign a given equipment slot a given equipment pointer */
bool Person::setEquip(const EquipSlots &slot, Equipment* new_equip)
{
  if (new_equip == nullptr || slot != new_equip->getEquipSlot())
    return false;

  if (new_equip->getEquipFlag(EquipState::EQUIPPED))
    return false;

  if (new_equip->getEquipFlag(EquipState::TWO_HANDED))
  {
    if (getEquip(EquipSlots::LARM) == nullptr && 
        getEquip(EquipSlots::RARM) == nullptr)
    {
      //TODO: Grab equip slot function [01-14-14] 
      //getEquip(EquipSlots::LARM) = new_equip;
      //getEquip(EquipSlots::RARM) = new_equip;
    }
  }
  else
  {
    //TODO: Grab equip slot function [01-14-14] 
    //getEquip(slot) = new_equip;
  }

  return true;
}


/* Attempts to assign a new loop set for the person */
bool Person::setLoot(const uint32_t &new_credit_drop, 
                     const uint32_t &new_exp_drop, 
                     const std::vector<uint32_t> &new_item_drops)
{
  auto all_set = true;

  if (new_credit_drop < kMAX_CREDIT_DROP)
    credit_drop = new_credit_drop;
  else
  {
    credit_drop = kMAX_CREDIT_DROP;
    all_set = false;
  }

  if (new_exp_drop < kMAX_EXP_DROP)
    credit_drop = new_exp_drop;
  else
  {
    exp_drop = kMAX_EXP_DROP;
    all_set = false;
  }

  if (new_item_drops.size() < kMAX_ITEM_DROPS)
    item_drops = new_item_drops;
  else
  {
    item_drops.clear();
    all_set = false;
  }

  return all_set;
}

/* Assigns the sprite pointers for the person */
void Person::setSprites(Frame* const new_fp, Frame* const new_tp,
                        Frame* const new_fp_bubby, Frame* const new_tp_bubby)
{
  first_person = new_fp;
  third_person = new_tp;
  fp_bubbified_sprite = new_fp_bubby;
  tp_bubbified_sprite = new_tp_bubby;
}

/*=============================================================================
 * PUBLIC STATIC FUNCTIONS
 *============================================================================*/

/* Grabs the experience required to reach a given level */
uint32_t Person::getExpAt(const uint8_t &level)
{
  if (level != 0 && level <= kNUM_LEVELS)
    return exp_table.at(level - 1);

  return kMAX_EXP + 1;
}

uint8_t Person::getLevelAt(const uint32_t &experience)
{
  //TODO: Get level at function [11-14-14]
  return experience;//warning
}

/* Grabs the number the total number of levels for Person progression */
size_t Person::getNumLevels()
{
  return kNUM_LEVELS; 
}

/* Grabs the number of equipment slots a Person has */
size_t Person::getNumEquipSlots()
{
  return kNUM_EQUIP_SLOTS;
}

/* Returns the minimum experience possible */
uint32_t Person::getMinExp()
{
  return kMIN_EXP; 
}

/* Returns the maximum experience possible */
uint32_t Person::getMaxExp()
{
  return kMAX_EXP;
}

/* Returns the minimum level experience */
uint32_t Person::getMinLvlExp()
{
  return kMIN_LVL_EXP;
}

/* Returns theexperience at the final level */
uint32_t Person::getMaxLvlExp()
{
  return kMAX_LVL_EXP;
}

/* Returns the maximum possible credit drop */
uint32_t Person::getMaxCreditDrop()
{
  return kMAX_CREDIT_DROP;
}

/* Returns the maximum experience a Person can reward */
uint32_t Person::getMaxExpDrop()
{
  return kMAX_EXP_DROP;
}

/* Returns the maximmum # of item drops a Person can reward */
uint32_t Person::getMaxItemDrops()
{
  return kMAX_ITEM_DROPS;
}

/* Returns the minimum damage modifier possible */
float Person::getMinDmgModi()
{
  return kMIN_DMG_MODI;
}

/* Returns the maximum damage modifier possible */
float Person::getMaxDmgModi()
{
  return kMAX_DMG_MODI;
}

/* Returns the minimum experience modifier possible */
float Person::getMinExpModi()
{
  return kMIN_EXP_MODI;
}

/* Returns the maximum experience modifier possible */
float Person::getMaxExpModi()
{
  return kMAX_EXP_MODI;
}