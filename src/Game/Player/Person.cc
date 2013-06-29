/******************************************************************************
* Class Name: Person Implementation
* Date Created: December 2nd, 2012 (Rewritten March 6th, 2013)
* Inheritance: None
* Description: Head for Person class. Container for all the info describing
*              a character in the game, including their statistics for battle,
*              flags, etc.
* // TODO: Race and category equip item types [03-06-13]
* // TODO: Finish use item
* // TODO: Setup Functions
* // TODO: Skill functions?? [06-23-13]
******************************************************************************/
#include "Game/Player/Person.h"

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/
const uint Person::kMAX_LEVEL             =        127;
const uint Person::kMIN_LVL_EXP           =        156;
const uint Person::kMAX_LVL_EXP           =    5327426;
const uint Person::kMAX_EXPERIENCE        = 1000000000; /* Billion */
const uint Person::kMAX_EXP_DROP          =    1000000; /* Million */
const uint Person::kMAX_CREDIT_DROP       =   10000000; /* Ten Million */
const uint Person::kMAX_EQUIP_SLOTS       =          5;
const uint Person::kMAX_ITEM_DROPS        =          5;
const double Person::kMAX_DAMAGE_MODIFIER =    10.0000;
QVector<uint> Person::exp_table;

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description: Constructs a Person object gievn a name, race and category
 *              pointers, plus two strings describing which stats they are
 *              focused on and how much (their curve)
 *
 * Inputs: QString name - name of the person
 *         Race*   - pointer to the race of the person
 *         Category* - pointer to the category of the person
 *         QString p - primary element and curve of the person
 *         QString s - secondary element and curve of the person
 * Output: none
 */
Person::Person(QString name, Race *race, Category* cat, QString p, QString s)
{
  /* Pointer setup */
  setRace(race);
  setCategory(cat);
  skills = 0;
  setFirstPerson();
  setThirdPerson();

  setDmgMod(0);
  setExp(0);
  setLevel(1);
  setName(name);
  setPrimary(p);
  setSecondary(s);

  /* Set up all three attribute sets */
  setUpBaseStats();
  setStats(base_stats);
  setTempStats(base_stats);

  if (exp_table.isEmpty())
    calcExpTable();

  /* Initially gather all usable skills */
  calcSkills();
}

/*
 * Description: Annihilates a Person object
 */
Person::~Person() {}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

/*
 * Description: Calculates and builds the experience table for a person
 *
 * Inputs: none
 * Output: none
 *
 */
void Person::calcExpTable()
{
  double b = log((double)kMAX_LVL_EXP / kMIN_LVL_EXP) / (kMAX_LEVEL - 1);
  double a = (double)kMIN_LVL_EXP / (exp(b) - 1.0);
  for (uint i = 1; i <= kMAX_LEVEL + 1; i++)
  {
    int old_exp = round(a * exp(b * (i - 1)));
    int new_exp = round(a * exp(b * i));
    exp_table.push_back(new_exp - old_exp);
  }
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/*
 * Description: Returns the value of the experience table at a given point
 *
 * Inputs: ushort - level to look up on the table
 * Output: uint   - total experience required for given level
 */
uint Person::getExpAt(ushort level)
{
  if (level < exp_table.size())
    return exp_table.at(level);
  return 0;
}

/*
 * Description: Adds an amount to the experience of the person and calls
 *              setLevel() if necessary to deal with level ups accordingly
 *              Will deal with multiple level ups. 
 *
 * Inputs: value - amount of experience to be added
 * Output: none
 */
void Person::addExperience(uint value)
{
  if (total_exp + value > kMAX_EXPERIENCE)
    total_exp = kMAX_EXPERIENCE;
  else
    total_exp += value;

  /* Level the character to the proper value (if necessary) */
  while (!getPersonFlag(Person::MAXLVL) && total_exp >= getExpAt(getLevel()+1))
    setLevel(getLevel() + 1);
}

/*
 * Description: Prepares the Person for a battle, by assigning his temp min and
 *              max stats as the current stats value.
 *
 * Inputs: none
 * Output: none
 */
void Person::battlePrep()
{
  for (int i = 0 ; i < temp_stats.getSize(); i++)
  {
    temp_stats.setStat(i, stats.getStat(i));
    temp_stats.setMax(i, stats.getStat(i));
  }
}

/*
 * Description: This function calculates the currently usable skills of the
 *              person, based on the skills allowed by their Race and Category.
 *
 * Inputs: none
 * Output: none
 */
void Person::calcSkills()
{
  if (getSkills() != 0)
  {
    QVector<Skill*> race_skills = getRace()->getSkillSet()->getSkills();
    QVector<Skill*> cat_skills  = getCategory()->getSkillSet()->getSkills();

    QVector<ushort> race_lvls = getRace()->getSkillSet()->getSkillLevels();
    QVector<ushort> cat_lvls  = getCategory()->getSkillSet()->getSkillLevels();

    for (int i = 0; i < race_skills.size(); i++)
      if (getLevel() > race_lvls.at(i))
        skills->addSkill(race_skills.at(i));

    for (int i = 0; i < cat_skills.size(); i++)
     if (getLevel() > cat_lvls.at(i))
       skills->addSkill(cat_skills.at(i));


    for (int i = 0; i < equipment.size(); i++)
    {
      SkillSet* equipment_skills = equipment[i]->getSkills(getLevel());
      QVector<Skill*> equip_skills = equipment_skills->getSkills();

      for (int j = 0; j < equip_skills.size(); j++)
        skills->addSkill(equip_skills.at(i));

      equip_skills.clear();
    }

    skills->cleanUp();
  }
}

/*
 * Description: Quick function for dealing damage to a Person with much nicer
 *              looking code lines.
 *
 * Inputs: ushort - amount of damage to do to the Person
 * Output: bool - returns true If the Person is now dead
 */
bool Person::damage(ushort amount)
{
  tempStats()->changeStat(0, -amount);
  if (tempStats()->getStat(0) == 0)
    return true;
  return false;
}

/*
 * Description: Method for printing all the info describing a person, by
 *              calling sub-print functions
 *
 * Inputs: none
 * Output: none
 */
void Person::printAll()
{
  qDebug() << " --- Person --- ";
  printBasics();
  printEquipment();
  printFlags();
  printBaseStats();
  printStats();
  printTempStats();
  printSkills();
  qDebug() << " --- / Person --- ";
}

/*
 * Description: Method for printing the basic info of a person
 *
 * Inputs: none
 * Output: none
 */
void Person::printBasics()
{
  qDebug() << "Name: " << getName();
  qDebug() << "Rank: " << getRank();
  qDebug() << "Level: " << getLevel();
  qDebug() << "Total Exp: " << getExp();
  qDebug() <<  "Category: " << cat->getName();
  qDebug() << "Race: " << race->getName();
  qDebug() << "Dmg Modifier: " << damage_modifier;
  qDebug() << "Primary Element: " << primary;
  qDebug() << "Secondary Element: " << secondary;
  qDebug() << "Primary Curve: " << primary_curve;
  qDebug() << "Secondary Curve: " << secondary_curve;

  /* Print out the drops */
  for (int i = 0; i < item_drops.size(); i++)
  // qDebug() << "Item Drop #" << i << ": " << item_drops[i].getName();
 qDebug() << "Exp Drop: " << experience_drop;
 qDebug() << "Credit Drop: " << credit_drop;
}

/*
 * Description: Method for printing the basic info of a person
 *
 * Inputs: none
 * Output: none
 */
void Person::printEquipment()
{
  for (int i = 0; i < equipment.size(); i++)
    qDebug() << "Equipment #" << i << ": " << equipment[i]->getName();
}

/*
 * Description: Method for printing out the flags of the Person
 *
 * Inputs: none
 * Output: none
 */
void Person::printFlags()
{
  qDebug() << "ALIVE: " << getPersonFlag(Person::ALIVE);
  qDebug() << "INBATTLE: " << getPersonFlag(Person::INBATTLE);
  qDebug() << "SLEUTH: " << getPersonFlag(Person::SLEUTH);
  qDebug() << "BEARACKS: " << getPersonFlag(Person::BEARACKS);
  qDebug() << "RENDERING: " << getPersonFlag(Person::RENDERING);
  qDebug() << "MAINCHARACTER: " << getPersonFlag(Person::MAINCHARACTER);
  qDebug() << "FINALBOSS: " << getPersonFlag(Person::FINALBOSS);
  qDebug() << "BOSS: " << getPersonFlag(Person::BOSS);
  qDebug() << "MINIBOSS: " << getPersonFlag(Person::MINIBOSS);
  qDebug() << "MAXLVLL " << getPersonFlag(Person::MAXLVL);
  qDebug() << "CANATTACK: " << getPersonFlag(Person::CANATTACK);
  qDebug() << "CANUSEITEM: " << getPersonFlag(Person::CANUSESKILLS);
  qDebug() << "CANRUN: " << getPersonFlag(Person::CANRUN);
  qDebug() << "SKIPNEXTTURN: " << getPersonFlag(Person::SKIPNEXTTURN);
  qDebug() << "MISSNEXTTARGET: " << getPersonFlag(Person::MISSNEXTTARGET);
  qDebug() << "NOEFFECT: " << getPersonFlag(Person::NOEFFECT);
  qDebug() << "ISBUBBY: " << getPersonFlag(Person::ISBUBBY);
  qDebug() << "TWOSKILLS: " << getPersonFlag(Person::TWOSKILLS);
  qDebug() << "THREESKILLS: " << getPersonFlag(Person::THREESKILLS);
  qDebug() << "REFLECT: " << getPersonFlag(Person::REFLECT);
}

/*
 * Description: Method for printing out the base stats of a person
 *
 * Inputs: none
 * Output: none
 */
void Person::printBaseStats()
{
    base_stats.printInfo();
}

/*
 * Description: Method for printing out the normal stats of a person
 *
 * Inputs: none
 * Output: none
 */
void Person::printStats()
{
    stats.printInfo();
}

/*
 * Description: Method for printing out the temp stats of a person
 *
 * Inputs: none
 * Output: none
 */
void Person::printTempStats()
{
    temp_stats.printInfo();
}

/*
 * Description: Method for printing out the skills of a person
 *
 * Inputs: none
 * Output: none
 */
void Person::printSkills()
{
  skills->printInfo();
}

/*
 * Description: Sets up the base stats (grabs Category min and max values
 *              and adds them to the Race bonuses)
 *
 * Inputs: none
 * Output: none
 */
void Person::setUpBaseStats()
{
  for (int i = 0; i < base_stats.getSize(); i++)
  {
    base_stats.setStat(i, cat->getAttrSet().getStat(i) +
                       race->getAttrSet().getStat(i));
    base_stats.setMax(i, cat->getAttrSet().getMax(i) +
                      race->getAttrSet().getMax(i));
  }
}

/*
 * Description: Uses an item
 *
 * Inputs: used_item - pointer of an item to be used
 * Output: bool - tue if item used successfully
 */
bool Person::useItem(Item* used_item)
{
  /* Assert the person can use items and the item is menu-usable */
  if (!getPersonFlag(Person::CANUSEITEM))
    return false;

  if (!used_item->getItemFlag(Item::MENUREADY))
    return false;

  QVector<Skill*> to_do = used_item->getActionSet()->getSkills();

  for (int i = 0; i < to_do.size(); i++)
  {
      QVector<Action*> to_be_done = to_do.at(i)->getEffects();

    for (int j = 0; j < to_be_done.size(); j++)
    {
      if (to_be_done.at(j)->getActionFlag(Action::THERMAL))
      {

      }
      if (to_be_done.at(j)->getActionFlag(Action::POLAR))
      {
      }
      if (to_be_done.at(j)->getActionFlag(Action::CHARGED))
      {
      }

        if (to_be_done.at(j)->getActionFlag(Action::OFFENSIVE))
        {

        }

        if (to_be_done.at(j)->getActionFlag(Action::LOWER))
        {

        }
        if (to_be_done.at(j)->getActionFlag(Action::RAISE))
        {

        }
    }
  }
}

/*
 * Description: Evaluates a given person state flag
 *
 * Inputs: Person state flag
 * Output: Boolean evaluation of the flag
 */
bool Person::getPersonFlag(PersonState flags)
{
  return state_set.testFlag(flags);
}

/*
 * Description: Sets a person state flag to a given boolean value
 *
 * Inputs: PersonState flag
 *         Boolean value to set the flag to
 * Output: none
 */
void Person::setPersonFlag(PersonState flags, bool set_value)
{
  (set_value) ? (state_set |= flags) : (state_set &= flags);
}

/*
 * Description: Returns the bubby sprite of the Person
 *
 * Inputs: none
 * Output: Sprite* - pointer to the bubby sprite of the person
 */
Sprite* Person::getBubbySprite()
{
  return bubbified_sprite;
}

/*
 * Description: Returns a pointer to the category of the Person.
 *
 * Inputs: none
 * Output: Category* - pointer to the category
 */
Category* Person::getCategory()
{
  return cat;
}

/*
 * Description: Returns the damage modifier of the Person
 *
 * Inputs: none
 * Output: double - the damage modifier of the Person
 */
double Person::getDmgMod()
{
  return damage_modifier;
}

/*
 * Description: Returns the vector of equipment for the person.
 *
 * Inputs: None
 * Output: QVector<Equipment*> - the equipment of the person
 */
QVector<Equipment*> Person::getEquipment()
{
  return equipment;
}

/*
 * Description: Returns a pointer to an Equipment at a given index
 *
 * Inputs: int - index of the equipment to be checked
 * Output: Equipment* - pointer at the given index
 */
Equipment* Person::getEquipSlot(int index)
{
  return equipment[index];
}

/*
 * Description: Returns the pointer to an Equipment at a given slot
 *
 * Inputs: QString - name of the slot to check
 * Output: Equipment* - pointer at the given slot
 */
Equipment* Person::getEquipSlot(QString slot_name)
{
  if (slot_name == "HEAD")     return getEquipSlot(0);
  if (slot_name == "LEFTARM")  return getEquipSlot(1);
  if (slot_name == "RIGHTARM") return getEquipSlot(2);
  if (slot_name == "TORSO")    return getEquipSlot(3);
  if (slot_name == "LEGS")     return getEquipSlot(4);
  return 0;
}

/*
 * Description: Returns the pointer to the Person's Race
 *
 * Inputs: none
 * Output: Race* - pointer to the person's Race
 */
Race* Person::getRace()
{
  return race;
}

/*
 * Description: Returns the set of skills describing a person.
 *
 * Inputs: none
 * Output: SkillSet* - pointer to the Skills of the person
 */
SkillSet* Person::getSkills()
{
  return skills;
}

/*
 * Description: Returns the set of skills the Person can currently use, based
 *              on the amount of QD available and their enabled or disabled
 *
 *
 * Inputs: none
 * Output: SkillSet* - pointer to the useable Skills of the person
 */
SkillSet* Person::getUseableSkills()
{
  calcSkills();
  QVector<Skill*> temp_skills = skills->getSkills();

  /* Remove skills which have too high of QD cost */
  for (int i = 0; i < temp_skills.size(); i++)
    if (temp_skills.at(i)->getQdValue() > (uint)temp_stats.getStat("QTDR"))
      temp_skills.remove(i);

  /* Remove physical skills if physical skill flag disabled */
  if (getPersonFlag(Person::CANATTACK) == FALSE)
    for (int i = 0; i < temp_skills.size(); i++)
      if (temp_skills.at(i)->getFlag(Skill::PHYSICAL))
        temp_skills.remove(i);

  /* Remove non-physical skills if non-physical skill flag disabled */
  if (getPersonFlag(Person::CANUSESKILLS) == FALSE)
    for (int i = 0; i < temp_skills.size(); i++)
      if (!temp_skills.at(i)->getFlag(Skill::PHYSICAL))
        temp_skills.remove(i);

  SkillSet* temp_skill_set = 0;

  for (int i = 0; i < temp_skills.size(); i++)
    temp_skill_set->addSkill(temp_skills.at(i));

  return temp_skill_set;
}

/*
 * Description: Returns the experience of the person
 *
 * Inputs: none
 * Output: uint - experience of the person
 */
uint Person::getExp()
{
  return total_exp;
}

/*
 * Description: Returns the experience rewarded on battle victory
 *
 * Inputs: none
 * Output: uint - experience reward
 */
uint Person::getExpLoot()
{
  return experience_drop;
}

/*
 * Description: Returns the credits dropped on battle victory
 *
 * Inputs: none
 * Output: uint - credits dropped on battle victory
 */
uint Person::getCreditLoot()
{
  return credit_drop;
}

/*
 * Description: Returns the item loot of the person
 *
 * Inputs: none
 * Output: QVector<Item> - vector of items containing loot
 */
QVector<Item*> Person::getItemLoot()
{
  return item_drops;
}

/*
 * Description: Returns the level of the Person
 *
 * Inputs: AttributeSet - new attribute set to be assigned
 * Output: none
 */
uint Person::getLevel()
{
  return level;
}

/*
 * Description: Returns the name of the Person
 *
 * Inputs: none
 * Output: QString - name of the Person
 */
QString Person::getName()
{
  return name;
}

/*
 * Description: Returns the primary elemental bonus
 *
 * Inputs: none
 * Output: QString - the primary elemental bonus
 */
QString Person::getPrimary()
{
  return primary;
}

/*
 * Description: Returns the secondary elemental bonus
 *
 * Inputs: none
 * Output: QString - the secondary elemental bonus
 */
QString Person::getSecondary()
{
  return secondary;
}

/*
 * Description: Returns a QChar of the Primary Curve
 *
 * Inputs: none
 * Output: QChar - curve of the primary element
 */
QChar Person::getPrimaryCurve()
{
  return primary_curve;
}

/*
 * Description: Returns a QChar of the secondary_curve
 *
 * Inputs: none
 * Output: QChar - curve of the secondary element
 */
QChar Person::getSecondaryCurve()
{
  return secondary_curve;
}

/*
 * Description: Returns the rank string
 *
 * Inputs: none
 * Output: QString - rank of the Person
 */
QString Person::getRank()
{
  return rank;
}

/*
 * Description: Returns a pointer to the third person sprite
 *
 * Inputs: none
 * Output: Sprite* - pointer to the third peron sprite
 */
Sprite* Person::getThirdPerson()
{
  return third_person;
}

/*
 * Description: Returns a pointer to the first person sprite
 *
 * Inputs: none
 * Output: Sprite* - pointer to the first person sprite
 */
Sprite* Person::getFirstPerson()
{
  return first_person;
}

/*
 * Description: Returns the base AttributeSet
 *
 * Inputs: none
 * Output: AttributeSet - the base stat AttributeSet
 */
AttributeSet* Person::baseStats()
{
  return &base_stats;
}

/*
 * Description: Returns the standard AttributeSet
 *
 * Inputs: none
 * Output: AttributeSet - the standard stat AttributeSet
 */
AttributeSet* Person::getStats()
{
  return &stats;
}

/*
 * Description: Returns the temporary AttributeSet
 *
 * Inputs: none
 * Output: AttributeSet - the temporary AttributeSet
 */
AttributeSet* Person::tempStats()
{
  return &temp_stats;
}

/*
 * Description: Equips a given item to a given slot.
 *
 * Inputs: QString - slot to equip the item to.
 *         Equipment* - pointer to the equipment to attach
 * Output: bool - true if the equip took place
 */
bool Person::setEquipment(QString slot, Equipment* new_equipment)
{
  if (slot == "TWOHAND" && new_equipment->getEquipmentFlag(Equipment::TWOHAND))
  {
    if (getEquipSlot("LEFTARM") == 0 && getEquipSlot("RIGHTARM") == 0)
    {
      equipment[1] = new_equipment;
      equipment[2] = new_equipment;
      return true;
    }
  }
  if (slot == "HEAD" && new_equipment->getEquipmentFlag(Equipment::HEAD))
  {
    if (getEquipSlot("HEAD") == 0 && getEquipSlot("HEAD") == 0)
    {
      equipment[0] = new_equipment;
      return true;
    }
  }
  if (slot == "LEFTARM" && new_equipment->getEquipmentFlag(Equipment::LEFTARM))
  {
    if (getEquipSlot("LEFTARM") == 0)
    {
      equipment[1] = new_equipment;
      return true;
    }
  }
  if (slot == "RIGHTARM" && new_equipment->getEquipmentFlag(Equipment::RIGHTARM))
  {
    if (getEquipSlot("RIGHTARM") == 0)
    {
      equipment[2] = new_equipment;
      return true;
    }
  }
  if (slot == "TORSO" && new_equipment->getEquipmentFlag(Equipment::TORSO))
  {
    if (getEquipSlot("TORSO") == 0)
    {
      equipment[3] = new_equipment;
      return true;
    }
  }
  if (slot == "LEGS" && new_equipment->getEquipmentFlag(Equipment::LEGS))
  {
    if (getEquipSlot("LEGS") == 0)
    {
      equipment[4] = new_equipment;
      return true;
    }
  }
  return false;
}

/*
 * Description: Sets the bubby version sprite of the person
 *
 * Inputs: Sprite* - the sprite to be set for the bubby version of person
 * Output: none
 */
void Person::setBubbySprite(Sprite* new_sprite)
{
  bubbified_sprite = new_sprite;
}

/*
 * Description: Assigns a new Category to the person
 *
 * Inputs: Category* - new category for the Person
 * Output: none
 */
void Person::setCategory(Category* c)
{
  cat = c;
}

/*
 * Description: Assigns a new Category to the person
 *
 * Inputs: Category* - new category for the Person
 * Output: none
 */
void Person::setDmgMod(double value)
{
    (value < kMAX_DAMAGE_MODIFIER) ? (damage_modifier = value) :
                                     (damage_modifier = kMAX_DAMAGE_MODIFIER);
}

/*
 * Description: Sets the experience of the person
 * Note: Does NOT level the character, use addExp(int) for that.
 *       Use (setLevel()) to adjust the level accordingly!
 *
 * Inputs: uint - value of experience to be set
 * Output: none
 */
void Person::setExp(uint value)
{
  (value < kMAX_EXPERIENCE) ? (total_exp = value) :
                              (total_exp = kMAX_EXPERIENCE);
}

/*
 * Description: Sets the experience loot of the person
 *
 * Inputs: uint - value of experience loot to be set
 * Output: none
 */
void Person::setExpLoot(uint value)
{
  (value < kMAX_EXP_DROP) ? (experience_drop = value) :
                            (experience_drop = kMAX_EXP_DROP);
}

/*
 * Description: Sets the credit loot of a person
 *
 * Inputs: uint - value of credit loot to be set
 * Output: none
 */
void Person::setCreditLoot(uint value)
{
    (value < kMAX_CREDIT_DROP) ? (credit_drop = value) :
                                 (credit_drop = kMAX_CREDIT_DROP);
}

/*
 * Description: Sets the item drops of the person by creating new pointers.
 *
 *
 * Inputs: QVector<Item> - vector of items the loot is to be set to.
 * Output: none
 */
void Person::setItemLoot(QVector<Item*> items)
{
  for (int i = 0; i < item_drops.size(); i++)
  {
    delete item_drops.value(i);
      item_drops[i] = NULL;
  }
  item_drops.clear();

  for (int i = 0; (int)i < items.size() && (int)i < kMAX_ITEM_DROPS; i++)
      item_drops.push_back(new Item(items.value(i)));
}

/*
 * Description: Sets the level of the Person. Will account for higher levels,
 *              and sets the MAXLVL flag if the Person has reached kMAX_LEVEL.
 *              This function will calculate their new stat attributes.
 *
 * Notes [1]: The difference between old_stat(i) and new_stat(i) is
 *            curr_stat(i) and temp_stat(i)
 *
 * Inputs: new_level - the level to change the person to.
 * Output: bool      - true if the level was changed
 */
bool Person::setLevel(const uint &new_level)
{
  /* Assign the proper level value */
  if (new_level == getLevel())
    return false;
  if (new_level < kMAX_LEVEL)
    level = new_level;
  else
  {
    level = kMAX_LEVEL;
    setPersonFlag(Person::MAXLVL, true);
  }

  /* Calculate the stats for the person based on their maximum values */
  setUpBaseStats();
  setTempStats(*getStats());

  for (int i = 0; i < stats.getSize(); i++)
  {
    int base = base_stats.getStat(i);
    int max  = base_stats.getMax(i);
    QString test = stats.getName(i);
    test.chop(2);

    if (primary == test)
    {
      if (primary_curve == 'S') base = floor(base * 1.200);
      if (primary_curve == 'A') base = floor(base * 1.150);
      if (primary_curve == 'B') base = floor(base * 1.100);
      if (primary_curve == 'C') base = floor(base * 1.050);
    }
    else if (secondary == test)
    {
      if (secondary_curve == 'S') base = floor(base * 1.120);
      if (secondary_curve == 'A') base = floor(base * 1.090);
      if (secondary_curve == 'B') base = floor(base * 1.060);
      if (secondary_curve == 'C') base = floor(base * 1.030);
    }

    double b = log((double)max / base) / (kMAX_LEVEL - 1);
    double a = (double)base / (exp(b) - 1.0);

    int old_values = round(a * exp(b * (new_level - 1)));
    int new_values = round(a * exp(b * new_level));

    stats.setStat(i, new_values - old_values);
  }
  return true;
}

/*
 * Description: Assigns a new name
 *
 * Inputs: QString - new name
 * Output: none
 */
void Person::setName(QString new_name)
{
  name = new_name;
}

/*
 * Description: Sets the rank of the person
 *
 * Inputs: QString - string representing a new rank
 * Output: none
 */
void Person::setRank(QString new_rank)
{
  rank = new_rank;
}

/*
 * Description: Sets the race pointer of the Person
 *
 * Inputs: Race* - pointer to a new race value for the Person
 * Output: none
 */
void Person::setRace(Race* new_race)
{
  race = new_race;
}

/*
 * Description: Sets the person's primary elemental strength and curve
 *              Template "<2 Letter Element Code><Curve(S,A-D)>"
 *
 * Inputs: QString - string representing the strength and curve
 * Output: none
 */
void Person::setPrimary(QString value)
{
  setPrimaryCurve(value.at(value.length() - 1));
  value.chop(1);
  primary = value;

}

/*
 * Description: Sets the person's primary curve (also set from setPrimary())
 *
 * Inputs: QChar - character of the curve
 * Output: none
 */
void Person::setPrimaryCurve(QChar value)
{
  primary_curve = value;
}

/*
 * Description: Sets the person's secondary elemental strength and curve
 *              Template "<Element><Curve(A-F)>"
 *
 * Inputs: QString - string representing the strength and curve
 * Output: none
 */
void Person::setSecondary(QString value)
{
  setSecondaryCurve(value.at(value.length() - 1));
  value.chop(1);
  secondary = value;
}

/*
 * Description: Sets the person's secd curve (also set from setSecondary())
 *
 * Inputs: QString - string representing the strength and curve
 * Output: none
 */
void Person::setSecondaryCurve(QChar value)
{
  secondary_curve = value;
}

/*
 * Description: Sets the first person view sprite for the person
 *
 * Inputs: Sprite* - pointer to the sprite to be set
 * Output: none
 */
void Person::setFirstPerson(Sprite* s)
{
  first_person = s;
}

/*
 * Description: Sets the third person view sprite for the person
 *
 * Inputs: Sprite* - pointer to the sprite to be set
 * Output: none
 */
void Person::setThirdPerson(Sprite* s)
{
  third_person = s;
}

/*
 * Description: Assigns a new base stat set.
 *
 * Inputs: AttributeSet - new attribute set to be assigned
 * Output: none
 */
void Person::setBaseStats(AttributeSet new_stat_set)
{
  base_stats = new_stat_set;
}

/*
 * Description: Assigns a new standard stat set
 *
 * Inputs: AttributeSet - new attribute set to be assigned
 * Output: none
 */
void Person::setStats(AttributeSet new_stat_set)
{
  stats = new_stat_set;
}

/*
 * Description: Assigns a new temp stat set.
 *
 * Inputs: AttributeSet - new attribute set to be assigned
 * Output: none
 */
void Person::setTempStats(AttributeSet new_stat_set)
{
  temp_stats = new_stat_set;
}
