/******************************************************************************
* Class Name: Person [Header Declaration]
* Date Created: December 2nd, 2012 (Rewritten March 6th, 2013)
* Inheritance: None
* Description: Head for Person class. Container for all the info describing
*              a character in the game, including their statistics for battle,
*              flags, etc.
******************************************************************************/
#ifndef PERSON_H
#define PERSON_H

#include "MathHelper.h"
#include "EnumDb.h"
#include "Game/Player/Category.h"
#include "Game/Player/Race.h"
#include "Game/Player/Equipment.h"
#include "Game/Player/PersonRecord.h"
#include "Game/Sprite.h"

class Person : QWidget
{
  Q_OBJECT;

public:
  /* Default Constructor */
  Person();

  /* Copy constructor */
  Person(const Person &source);

  /* Constructor for a person object */
  Person(QString pname, Race* prace, Category* pcat, QString p, QString s);

  /* Annihilates a person object */
  ~Person();

  /* Enumerated flags for states of a person */
  enum PersonState
  {
    ALIVE            = 1 << 0, /* Is the person alive? */
    IN_BATTLE        = 1 << 1, /* Is the person in a battle? */
    SLEUTH           = 1 << 2, /* Person is currently in your sleuth */
    BEARACKS         = 1 << 3, /* Person is currently in your RESERVE */
    RENDERING        = 1 << 4, /* Person is rendering on the map */
    MAIN_CHARACTER   = 1 << 5, /* Is this person the main character? */
    FINAL_BOSS       = 1 << 6, /* Is this the final boss? */
    BOSS             = 1 << 7, /* Is this person a Boss character? */
    MINI_BOSS        = 1 << 8,  /* Is this person a miniboss? */
    MAX_LVL          = 1 << 9,  /* Is this person the max level? */
    ATTACK_ENABLED   = 1 << 10, /* Can the person use physical attacks> */
    SKILL_ENABLED    = 1 << 11, /* Can the person currently use non-phys skills? */
    ITEM_USE_ENABLED = 1 << 12, /* Can the person currently use items? */
    RUN_ENABLED      = 1 << 13, /* Can the person run? */
    SKIP_NEXT_TURN   = 1 << 14, /* Will the person skip their next turn? */
    MISS_NEXT_TARGET = 1 << 15, /* Will the person miss their next target? */
    NO_EFFECT        = 1 << 16, /* Does the person's next attack have no effect? */
    IS_BUBBY         = 1 << 17, /* Is the person currently a bubby? */
    TWO_SKILLS       = 1 << 18, /* Can the person use two skills per turn? */
    THREE_SKILLS     = 1 << 19, /* Can the person use three skills per turn? */
    HALF_COST        = 1 << 20, /* Does the person only use half the QD? */
    REFLECT          = 1 << 20, /* Is the person currently reflecting skills? */
    BOND             = 1 << 21, /* Is the person in the BOND state? */
    REVIVABLE        = 1 << 22, /* Can the person be revived? */
    CAN_LEVEL_UP     = 1 << 23, /* Can the person level up? */
    CAN_LEARN_SKILLS = 1 << 24  /* Can the person learn skills? */
  };
  Q_DECLARE_FLAGS(PersonFlags, PersonState)

private:
  /* Equipment names and slots */
  QVector<Equipment*> equipment;

  /* Modifier by which to change a Person's actual damage */
  double damage_modifier;

  /* Person's level [1-127] */
  uint level;

  /* Person's total experience */
  uint total_exp;

  /* Table of experience */
  static QVector<int> exp_table;

  /* Person's loot when killed */
  QVector<Item*> item_drops;
  uint experience_drop;
  uint credit_drop;

  /* The person's class */
  Category* cat;

  /* The parent of the person */
  Person* parent;
  PersonRecord* person_record;

  /* The person's race */
  Race* race;

  /* The person's name & rank */
  QString name;
  EnumDb::PersonRanks rank;

  /* Set of flags for the person */
  PersonFlags state_set;

  /* List of skills and attributes that the person has */
  AttributeSet base_stats;
  AttributeSet current_stats;
  AttributeSet current_max_stats;
  AttributeSet temp_stats;
  AttributeSet temp_max_stats;

  /* List of currently available skills */
  SkillSet* base_skill_list;
  SkillSet* temp_skill_list;

  /* Person's primary and secondary curves */
  QString primary;
  QString secondary;
  QChar primary_curve;
  QChar secondary_curve;

  /* Sprites for the person, first person and third person versions */
  Sprite* first_person;
  Sprite* third_person;
  Sprite* bubbified_sprite;

  /*------------------- Constants -----------------------*/
  static const uint kMAX_LEVEL;
  static const uint kMIN_LVL_EXP;
  static const uint kMAX_LVL_EXP;
  static const uint kMAX_EXPERIENCE;  /* Billion */
  static const uint kMAX_EXP_DROP;    /* Million */
  static const uint kMAX_CREDIT_DROP; /* Ten Million */
  static const uint kMAX_EQUIP_SLOTS;
  static const uint kMAX_ITEM_DROPS;
  static const double kMAX_DAMAGE_MODIFIER;
  static const double kPRIM_X_MODI;
  static const double kPRIM_S_MODI;
  static const double kPRIM_A_MODI;
  static const double kPRIM_B_MODI;
  static const double kPRIM_C_MODI;
  static const double kSECD_X_MODI;
  static const double kSECD_S_MODI;
  static const double kSECD_A_MODI;
  static const double kSECD_B_MODI;
  static const double kSECD_C_MODI;


/*============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
private:
  /* Calculates an AttributeSet representing the person's basic statistics */
  AttributeSet calcBaseLevelStats();

  /* Calculate an AttributeSet representing the person's top statistics */
  AttributeSet calcMaxLevelStats();

  /* Copy all parameters from the other to the current */
  void copySelf(const Person &source);

/*============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/
public:
  /* Adds experience of a given amount --> level up */
  void addExperience(uint n);

  /* Prepare temp_stats to be used for a battle */
  void battlePrep();

  /* Calculates the usable skill set */
  void calcSkills();

  /* Quick function for dealing Vitality damage */
  bool damage(ushort amount);

  /* Methods for printing info pertaining to the person */
  void printAll();
  void printBasics();
  void printEquipment();
  void printFlags();
  void printSkills();

  /* PersonState flag functions */
  bool getPersonFlag(PersonState flags);
  void setPersonFlag(PersonState flags, bool set_value = true);

  /* Obtains the bubbified sprite of the person */
  Sprite* getBubbySprite();

  /* Gets the person's category */
  Category* getCategory();

  /* Obtain the curent Damage Modifier */
  double getDmgMod();

  /* Methods for returning equipments */
  QVector<Equipment*> getEquipment();
  Equipment* getEquipSlot(int index);
  Equipment* getEquipSlot(QString name);

  /* Gets the person's race */
  Race* getRace();

  /* Returns the available skills of the person */
  SkillSet* getSkills();
  SkillSet* getUseableSkills();

  /* Gets the person's experience */
  uint getExp();

  /* Get loot values */
  uint getExpLoot();
  uint getCreditLoot();
  QVector<Item*> getItemLoot();

  /* Gets the person's level */
  uint getLevel();

  /* Gets the persons name (set in construction and never again) */
  QString getName();

  /* Returns the personal record of the person */
  PersonRecord* getPersonRecord();

  /* Gets the person's primary and secondary elemental strengths */
  QString getPrimary();
  QChar   getPrimaryCurve();
  QString getSecondary();
  QChar   getSecondaryCurve();

  /* Gets the person's word rank */
  EnumDb::PersonRanks getRank();

  /* Gets the first and third person sprites */
  Sprite* getFirstPerson();
  Sprite* getThirdPerson();

  /* Methods for obtaining stat sets */
  AttributeSet* getStats();
  AttributeSet* getMax();
  AttributeSet* getTemp();
  AttributeSet* getMaxTemp();

  /* Sets an equipment to a slot */
  bool setEquipment(QString slot, Equipment* new_equipment);

  /* Sets the sprite of the Bubby */
  void setBubbySprite(Sprite* new_sprite);

  /* Sets the person's category */
  void setCategory(Category* c);

  /* Set's the person's damage modifier */
  void setDmgMod(double value);

  /* Set's the person's experience */
  void setExp(uint value);

  /* Set's the person's experience drop */
  void setExpLoot(uint value);

  /* Set's the person's credit drop */
  void setCreditLoot(uint value);

  /* Set's the person's item drops. */
  void setItemLoot(QVector<Item*> items);

  /* Sets the person's level */
  bool setLevel(const uint &new_level);

  /* Sets the person's level */
  void setName(QString new_name);

  /* Sets the person's primary and elemental strength curves */
  void setPrimary(QString value);
  void setPrimaryCurve(QChar value);
  void setSecondary(QString value);
  void setSecondaryCurve(QChar value);

  /* Sets the person's race */
  void setRace(Race* new_race);

  /* Sets the persons word rank */
  void setRank(EnumDb::PersonRanks new_rank);

  /* Sets the persons skill set */
  void setSkills(SkillSet* new_skill_set);

  /* Sets the first person sprite */
  void setFirstPerson(Sprite* s = 0);

  /* Sets the third person sprite */
  void setThirdPerson(Sprite* s = 0);

  /* Methods for setting stat values */
  void setStats(AttributeSet new_stat_set);
  void setMax(AttributeSet new_stat_set);
  void setTemp(AttributeSet new_stat_set);
  void setMaxTemp(AttributeSet new_stat_set);

  EnumDb::PersonRanks getRankEnum(QString rank_string);
  QString getRankString(EnumDb::PersonRanks person_rank);

 /*============================================================================
  * SIGNALS
  *============================================================================*/
signals:
  /* Emitted when the Person levels up */
  void levelUp();

/*============================================================================
 * PUBLIC STATIC FUNCTIONS
 *============================================================================*/
public:
  /* Gets the experience at a given level */
  static uint getExpAt(ushort level);

  /* Gets the max level constant */
  static uint getMaxLevel();

/*============================================================================
 * OPERATOR FUNCTIONS
 *===========================================================================*/
public:
  Person& operator= (const Person &source);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Person::PersonFlags)

#endif // PERSON_H
