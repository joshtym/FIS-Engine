/*******************************************************************************
 * Class Name: TestBattle
 * Date Created: July 31, 2015
 * Inheritance: none
 * Description: This class is the testing interface for battle scenarios.
 * // Remove ailments
 * // Chlorophononaonaonaon - switch to curr canopy
 ******************************************************************************/
#include "TestBattle.h"

/* Constant Implementation - see header file for descriptions */
const uint8_t TestBattle::kLVL_MAX = 15;
const std::string TestBattle::kMENU_ITEMS[] = {
    "Arcadius", "Aurora Agent", "Aurora Agent x 2", "Aurora Agent x 3",
    "Aurora Agent, Heavy", "Aurora Heavy", "Aurora Engineer, Drone",
    "Aurora Engineer, Drone x 3", "Aurora Engineer, Agent, Heavy, Drone",
    "Reverdile, Reverdling", "Reverdile, Reverdling x 2",
    "Reverdile, Reverdliling x 3", "Reverdling", "Reverdling x 2",
    "The Impossible", "Exit"};
const std::string TestBattle::kMENU_FOE = "Enter Foe Level: ";
const std::string TestBattle::kMENU_FRIEND = "Enter Friend Level: ";
const uint8_t TestBattle::kNUM_MENU_ITEMS = 16;

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/* ------------------------------------------------------------------------- */
/* Constructor function */
/* ------------------------------------------------------------------------- */
TestBattle::TestBattle(Options *running_config, EventHandler *event_handler)
    : display_data{nullptr},
      // plep_sullen_sting{nullptr},
      // plep_befuddling_sting{nullptr},
      // plep_chlorophoria{nullptr},
      // plep_canopy{nullptr},
      // plep_numbing_sting{nullptr},
      // plep_toxic_sting{nullptr},
      // plep_updraft{nullptr},
      // plep_light_push{nullptr},
      // plep_light_shot{nullptr},
      // plep_prismatic_shot{nullptr},
      // plep_rail_shot{nullptr},
      // plep_shatter_shot{nullptr},
      // plep_static_shot{nullptr},
      // plep_locked_shot{nullptr},
      // plep_strike{nullptr},
      // plep_paw_strike{nullptr},
      // plep_maul{nullptr},
      // plep_multi_strike{nullptr},
      // plep_ensnare{nullptr},
      // plep_enrich{nullptr},
      // plep_upgrade{nullptr},
      // plep_rock{nullptr},
      // plep_medkit{nullptr},
      font_normal{nullptr},
      party_foes{nullptr},
      party_friends{nullptr}

{
  rock = nullptr;
  medkit = nullptr;
  non_base_rock = nullptr;
  non_base_medkit = nullptr;

  base_path = "";
  battle_logic = nullptr;
  battle_start = false;
  first_run = true;
  font_normal = nullptr;
  game_config = nullptr;
  lvl_foe = 1;
  lvl_friend = 1;
  menu_index = 0;
  mode = SCENARIO;
  party_foes = nullptr;
  party_friends = nullptr;

  display_data = new BattleDisplayData();
  battle_logic = new Battle();

  setConfiguration(running_config);
  setEventHandler(event_handler);

  battle_logic->setDisplayData(display_data);
  create();
}

/* ------------------------------------------------------------------------- */
/* Destructor function */
/* ------------------------------------------------------------------------- */
TestBattle::~TestBattle()
{
  destroy();

  if(display_data)
    delete display_data;

  display_data = nullptr;
}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

/* ------------------------------------------------------------------------- */
/* Builds the ailment frames */
/* ------------------------------------------------------------------------- */
void TestBattle::buildBattleDisplay(SDL_Renderer *renderer)
{
  /* Render fonts */
  for(uint8_t i = 0; i < menu_items.size(); i++)
  {
    menu_items[i]->setText(renderer, kMENU_ITEMS[i], {255, 255, 255, 255});
    menu_items_sel[i]->setText(renderer, kMENU_ITEMS[i], {255, 0, 0, 255});
  }

  getSkill(222)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/healplep_AA_A");
  getSkill(222)->setAnimationFrames(12);
  getSkill(222)->setAnimationTime(70);

  getSkill(240)->setAnimationPath(base_path +
                                  "sprites/Battle/Pleps/numbingstingplep_AA_A");
  getSkill(240)->setAnimationFrames(4);
  getSkill(240)->setAnimationTime(70);

  getSkill(241)->setAnimationPath(base_path +
                                  "sprites/Battle/Pleps/sullenstingplep_AA_A");
  getSkill(241)->setAnimationFrames(6);
  getSkill(241)->setAnimationTime(70);

  getSkill(242)->setAnimationPath(base_path +
                                  "sprites/Battle/Pleps/toxicstingplep_AA_A");
  getSkill(242)->setAnimationFrames(12);
  getSkill(242)->setAnimationTime(70);

  getSkill(243)->setAnimationPath(
      base_path + "sprites/Battle/Pleps/befuddlingstingplep_AA_A");
  getSkill(243)->setAnimationFrames(4);
  getSkill(243)->setAnimationTime(70);

  getSkill(220)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/canopyplep_AA_A");
  getSkill(220)->setAnimationFrames(6);
  getSkill(220)->setAnimationTime(90);

  getSkill(221)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/updraftplep_AA_A");
  getSkill(221)->setAnimationFrames(5);
  getSkill(221)->setAnimationTime(80);

  getSkill(100)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/lightpushplep_AA_A");
  getSkill(100)->setAnimationFrames(7);
  getSkill(100)->setAnimationTime(40);

  getSkill(120)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/lightshotplep_AA_A");
  getSkill(120)->setAnimationFrames(6);
  getSkill(120)->setAnimationTime(80);

  getSkill(121)->setAnimationPath(
      base_path + "sprites/Battle/Pleps/prismaticshotplep_AA_A");
  getSkill(121)->setAnimationFrames(7);
  getSkill(121)->setAnimationTime(85);

  getSkill(140)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/railshotplep_AA_A");
  getSkill(140)->setAnimationFrames(6);
  getSkill(140)->setAnimationTime(80);

  getSkill(141)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/basicplep_AA_A");
  getSkill(141)->setAnimationFrames(3);
  getSkill(141)->setAnimationTime(70);

  getSkill(160)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/upgradeplep_AA_A");
  getSkill(160)->setAnimationFrames(16);
  getSkill(160)->setAnimationTime(80);

  getSkill(180)->setAnimationPath(base_path +
                                  "sprites/Battle/Pleps/staticshotplep_AA_A");
  getSkill(180)->setAnimationFrames(4);
  getSkill(180)->setAnimationTime(80);

  getSkill(181)->setAnimationPath(base_path +
                                  "sprites/Battle/Pleps/lockedshotplep_AA_A");
  getSkill(181)->setAnimationFrames(14);
  getSkill(181)->setAnimationTime(80);

  getSkill(1001)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/basicplep_AA_A");
  getSkill(1001)->setAnimationFrames(3);
  getSkill(1001)->setAnimationTime(70);

  getSkill(1020)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/pawstrikeplep_AA_A");
  getSkill(1020)->setAnimationFrames(7);
  getSkill(1020)->setAnimationTime(45);

  getSkill(1021)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/maulplep_AA_A");
  getSkill(1021)->setAnimationFrames(11);
  getSkill(1021)->setAnimationTime(45);

  getSkill(1022)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/basicplep_AA_A");
  getSkill(1022)->setAnimationFrames(3);
  getSkill(1022)->setAnimationTime(70);

  getSkill(1040)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/ensnareplep_AA_A");
  getSkill(1040)->setAnimationFrames(8);
  getSkill(1040)->setAnimationTime(70);

  getSkill(1041)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/healplep_AA_A");
  getSkill(1041)->setAnimationFrames(12);
  getSkill(1041)->setAnimationTime(60);

  getSkill(1042)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/basicplep_AA_A");
  getSkill(1042)->setAnimationFrames(3);
  getSkill(1042)->setAnimationTime(70);

  getSkill(1043)
      ->setAnimationPath(base_path + "sprites/Battle/Pleps/healplep_AA_A");
  getSkill(1043)->setAnimationFrames(12);
  getSkill(1043)->setAnimationTime(60);

  /* Background */
  // Sprite* background = new Sprite(  , renderer);
  // battle_logic->setBackground(background);

  /* Skill Images (lol) */
  for(uint32_t i = 0; i < skills.size(); i++)
  {
    if(i % 2 == 0)
      skills[i]->setThumbnail(new Frame(
          base_path + "sprites/Battle/Skills/pawstrike_AA_A00.png", renderer));
    else
      skills[i]->setThumbnail(new Frame(
          base_path + "sprites/Battle/Skills/staticshot_AA_A00.png", renderer));
  }
}

/* ------------------------------------------------------------------------- */
/* Main create call - first construction */
/* ------------------------------------------------------------------------- */
void TestBattle::create()
{
  // clang-format off
  /* Attribute Sets
    "VITA", "QTDR", "PHAG", "PHFD",
    "THAG", "THFD", "POAG", "POFD",
    "PIAG", "PIFD", "CHAG", "CHFD",
    "CYAG", "CYFD", "NIAG", "NIFD",
    "LIMB", "MMTM", "UNBR", "MANN"
  */

  stats_human_min = AttributeSet(
  {
    50, 100, 8, 8,
    0, 0, 0, 0,
    5, 5, 5, 5,
    0, 0, 0, 0,
    9, 9, 10, 0
  }, true);

  stats_human_max = AttributeSet(
  {
    85, 200, 16, 16,
    0, 0, 0, 0,
    10, 10, 10, 10,
    0, 0, 0, 0,
    19, 19, 15, 0
  }, true);

  stats_bear_min = AttributeSet(
  {
    75, 100, 10, 10,
    0, 0, 0, 0,
    8, 8, 6, 6,
    0, 0, 0, 0,
    10, 12, 10, 0
  }, true);

  stats_bear_max = AttributeSet(
  {
    125, 200, 20, 20,
    0, 0, 0, 0,
    16, 16, 12, 12,
    0, 0, 0, 0,
    20, 25, 20, 0
  }, true);

  stats_treefolk_min = AttributeSet(
  {
    25, 100, 7, 7,
    0, 0, 0, 0,
    4, 4, 4, 4,
    0, 0, 0, 0,
    10, 10, 10, 0
  }, true);

  stats_treefolk_max = AttributeSet(
  {
    26, 200, 14, 14,
    0, 0, 0, 0,
    8, 8, 8, 8,
    0, 0, 0, 0,
    18, 18, 15, 0
  }, true);

  stats_robot_min = AttributeSet(
  {
    40, 100, 7, 7,
    0, 0, 0, 0,
    5, 5, 5, 5,
    0, 0, 0, 0,
    15, 10, 15, 0
  }, true);

  stats_robot_max = AttributeSet(
  {
    50, 200, 14, 14,
    0, 0, 0, 0,
    10, 10, 10, 10,
    0, 0, 0, 0,
    25, 25, 20, 0
  }, true);

  stats_player_min = AttributeSet(
  {
    150, 50, 8, 8,
    0, 0, 0, 0,
    6, 5, 3, 2,
    0, 0, 0, 0,
    15, 27, 10, 1
  }, true);

  stats_player_max = AttributeSet(
  {
    350, 75, 14, 14,
    0, 0, 0, 0,
    12, 11, 7, 6,
    0, 0, 0, 0,
    20, 32, 15, 2
  }, true);

  stats_aurora_heavy_min = AttributeSet(
  {
    100, 50, 3, 5,
    0, 0, 0, 0,
    6, 6, 4, 4,
    0, 0, 0, 0,
    5, 5, 10, 0
  }, true);

  stats_aurora_heavy_max = AttributeSet(
  {
    200, 75, 7, 10,
    0, 0, 0, 0,
    12, 12, 8, 8,
    0, 0, 0, 0,
    10, 10, 15, 0
  }, true);

  stats_aurora_eng_min = AttributeSet(
  {
    50, 100, 3, 2,
    0, 0, 0, 0,
    3, 2, 5, 4,
    0, 0, 0, 0,
    9, 9, 10, 0
  }, true);

  stats_aurora_eng_max = AttributeSet(
  {
    100, 125, 8, 6,
    0, 0, 0, 0,
    8, 6, 12, 12,
    0, 0, 0, 0,
    18, 18, 15, 0
  }, true);

  stats_aurora_agent_min = AttributeSet(
  {
    50, 100, 3, 2,
    0, 0, 0, 0,
    3, 2, 5, 4,
    0, 0, 0, 0,
    9, 9, 10, 0
  }, true);

  stats_aurora_agent_max = AttributeSet(
  {
    100, 125, 8, 6,
    0, 0, 0, 0,
    8, 6, 12, 12,
    0, 0, 0, 0,
    18, 18, 15, 0
  }, true);

  stats_aurora_drone_min = AttributeSet(
  {
    25, 150, 3, 3,
    0, 0, 0, 0,
    3, 3, 3, 3,
    0, 0, 0, 0,
    15, 15, 10, 0
  }, true);

  stats_aurora_drone_max = AttributeSet(
  {
    50, 175, 6, 6,
    0, 0, 0, 0,
    3, 3, 3, 3,
    0, 0, 0, 0,
    30, 30, 20, 0
  }, true);

  stats_reverdling_min = AttributeSet(
  {
    10, 100, 2, 2,
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0,
    10, 6, 10, 0
  }, true);

  stats_reverdling_max = AttributeSet(
  {
    35, 125, 4, 4,
    0, 0, 0, 0,
    2, 2, 2, 2,
    0, 0, 0, 0,
    20, 16, 15, 0
  }, true);

  stats_reverdile_min = AttributeSet(
  {
    100, 100, 3, 3,
    0, 0, 0, 0,
    5, 7, 3, 5,
    0, 0, 0, 0,
    15, 15, 10, 0
  }, true);

  stats_reverdile_max = AttributeSet(
  {
    125, 125, 6, 6,
    0, 0, 0, 0,
    18, 22, 10, 10,
    0, 0, 0, 0,
    30, 30, 15, 0
  }, true);

  // Yuki DO NOT edit Arcadius
  stats_arcadius_race_min = AttributeSet(
  {
    55000, 10000, 40000, 40000,
    1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000
  }, true);

  stats_arcadius_race_max = stats_arcadius_race_min;
  stats_arcadius_cat_min  = stats_arcadius_race_min;
  stats_arcadius_cat_max  = stats_arcadius_race_max;
  // clang-format on

  /* Actions */
  createActions();
  createSkills();
  createSkillSets();
  createClasses();
  createRaces();
}

/* ------------------------------------------------------------------------- */
/* Create actions */
/* ------------------------------------------------------------------------- */
void TestBattle::createActions()
{
  /* Alter Actions */

  /* Enrich - healing */
  act_alt.push_back(new Action("200,ALTER,,,,VITA,AMOUNT.50,AMOUNT.10,,100"));

  /* Chlorophona - healing */
  act_alt.push_back(new Action("201,ALTER,,,,VITA,PC.15,AMOUNT.10,,100"));

  /* Damage actions */

  /* Light Push (~5000 Damage) */
  act_dmg.push_back(new Action("0,DAMAGE,,,,VITA,AMOUNT.2000,AMOUNT.200,,100"));

  /* Prismatic Shot (~20 Damage / Hit) */
  act_dmg.push_back(new Action("1,DAMAGE,,,,VITA,AMOUNT.13,AMOUNT.2,,95"));

  /* Rail Shot (~35 Damage) */
  act_dmg.push_back(new Action("2,DAMAGE,,,,VITA,AMOUNT.38,AMOUNT.5,,100"));

  /* ShattersShot (~80 Damage) */
  act_dmg.push_back(new Action("3,DAMAGE,,,,VITA,AMOUNT.65,AMOUNT.10,,100"));

  /* Static Shot (drone nuisance) */
  act_dmg.push_back(new Action("4,DAMAGE,,,,VITA,AMOUNT.15,AMOUNT.5,,100"));

  /* Surge of Will */
  act_dmg.push_back(new Action("5,DAMAGE,,,,VITA,AMOUNT.130,AMOUNT.35,,100"));

  /* Strike (Humans) */
  act_dmg.push_back(new Action("6,DAMAGE,,,,VITA,AMOUNT.30,AMOUNT.10,,100"));

  /* Light Shot */
  act_dmg.push_back(new Action("7,DAMAGE,,,,VITA,AMOUNT.30,AMOUNT.2,,100"));

  /* Locked Shot */
  act_dmg.push_back(new Action("8,DAMAGE,,,,VITA,AMOUNT.13,AMOUNT.2,,100"));

  /* Paw Strike */
  act_dmg.push_back(new Action("9,DAMAGE,,,,VITA,AMOUNT.30,AMOUNT.5,,100"));

  /* Maul */
  act_dmg.push_back(new Action("10,DAMAGE,,,,VITA,AMOUNT.55,AMOUNT.10,,100"));

  /* Ensnare */
  act_dmg.push_back(new Action("11,DAMAGE,,,,VITA,AMOUNT.20,AMOUNT.5,,100"));

  /* Inflict Actions */

  /* Poison */
  act_inf.push_back(new Action("500,INFLICT,3.6,,,POISON,AMOUNT.30,,VITA,90"));

  /* Confusion */
  act_inf.push_back(new Action("501,INFLICT,2.5,,,CONFUSE,AMOUNT.30,,VITA,75"));

  /* Silence */
  act_inf.push_back(new Action("502,INFLICT,3.5,,,SILENCE,AMOUNT.30,,VITA,80"));

  /* All Attack Buff */
  act_inf.push_back(
      new Action("503,INFLICT,3.6,,,ALLATKBUFF,AMOUNT.5,,VITA,100"));

  /* All Def Buff */
  act_inf.push_back(
      new Action("504,INFLICT,3.6,,,ALLDEFBUFF,AMOUNT.5,,VITA,100"));

  /* Dodge Buff */
  act_inf.push_back(new Action("505,INFLICT,4.7,,,LIMBUFF,AMOUNT.5,,VITA,100"));

  /* Paralysis */
  act_inf.push_back(
      new Action("506,INFLICT,2.5,,,PARALYSIS,AMOUNT.30,,VITA,80"));

  /* Hibernation */
  act_inf.push_back(
      new Action("507,INFLICT,3.5,,,HIBERNATION,AMOUNT.10,,VITA,100"));

  /* Check to check all the actions are checked [checkingly] */
  // for(const auto& action : act_alt)
  // {
  //   std::cout << "Alt Valid: " << action->actionFlag(ActionFlags::VALID)
  //             << std::endl;
  // }
  // for(const auto& action : act_dmg)
  // {
  //   std::cout << "Dmg Valid: " << action->actionFlag(ActionFlags::VALID)
  //             << std::endl;
  // }
  // for(const auto& action : act_inf)
  // {
  //   std::cout << "Inf Valid: " << action->actionFlag(ActionFlags::VALID)
  //             << std::endl;
  // }
}

/* ------------------------------------------------------------------------- */
/* Create classes */
/* ------------------------------------------------------------------------- */
void TestBattle::createClasses()
{
  /* Arcadius LVL1 Class */
  class_arcadius1 =
      new Category(100, "Arcadius", "arcadius", stats_arcadius_cat_min,
                   stats_arcadius_cat_max, getSkillSet(100));
  class_arcadius1->setVitaRegenRate(RegenRate::WEAK);
  class_arcadius1->setQDRegenRate(RegenRate::WEAK);
  class_arcadius1->setDescription("The one. The only.");

  /* Aurora Agent Class */
  class_aurora_agent =
      new Category(120, "Agent", "agent", stats_aurora_agent_min,
                   stats_aurora_agent_max, getSkillSet(120));
  class_aurora_agent->setVitaRegenRate(RegenRate::ZERO);
  class_aurora_agent->setQDRegenRate(RegenRate::WEAK);
  class_aurora_agent->setDescription("A less than smart agent");

  /* Aurora Heavy Classs */
  class_aurora_heavy =
      new Category(140, "Heavy", "heavy", stats_aurora_heavy_min,
                   stats_aurora_heavy_max, getSkillSet(140));
  class_aurora_heavy->setVitaRegenRate(RegenRate::ZERO);
  class_aurora_heavy->setQDRegenRate(RegenRate::WEAK);
  class_aurora_heavy->setDescription("He weighs more than your house");

  /* Aurora Drone Class */
  class_aurora_drone =
      new Category(180, "Drone", "drone", stats_aurora_drone_min,
                   stats_aurora_drone_max, getSkillSet(180));
  class_aurora_drone->setVitaRegenRate(RegenRate::ZERO);
  class_aurora_drone->setQDRegenRate(RegenRate::WEAK);
  class_aurora_drone->setDescription("Cannon fodder");
  class_aurora_drone->setFlag(CategoryState::DEF_ENABLED, false);
  class_aurora_drone->setFlag(CategoryState::IMP_ENABLED, true);

  /* Aurora Engineer Class */
  class_aurora_engg =
      new Category(160, "Engineer", "engineer", stats_aurora_eng_min,
                   stats_aurora_eng_max, getSkillSet(160));
  class_aurora_engg->setVitaRegenRate(RegenRate::ZERO);
  class_aurora_engg->setQDRegenRate(RegenRate::WEAK);
  class_aurora_engg->setDescription("An engineer and much smarter than you");

  /* Player Class */
  class_player = new Category(200, "Bearanator", "player", stats_player_min,
                              stats_player_max, getSkillSet(200));
  class_player->setVitaRegenRate(RegenRate::WEAK);
  class_player->setQDRegenRate(RegenRate::WEAK);
  class_player->setDescription("This is me and I am awesome");

  /* Reverdile Class */
  class_reverdile =
      new Category(220, "Reverdile", "reverdile", stats_reverdile_min,
                   stats_reverdile_max, getSkillSet(220));
  class_reverdile->setVitaRegenRate(RegenRate::WEAK);
  class_reverdile->setQDRegenRate(RegenRate::WEAK);
  class_reverdile->setDescription("What is the master of crap called");

  /* Reverdling Class */
  class_reverdling =
      new Category(240, "Reverdling", "reverdling", stats_reverdling_min,
                   stats_reverdling_max, getSkillSet(240));
  class_reverdling->setVitaRegenRate(RegenRate::WEAK);
  class_reverdling->setQDRegenRate(RegenRate::WEAK);
  class_reverdling->setDescription("He's about as useful as fried turd");
}

/* ------------------------------------------------------------------------- */
/* Create menu */
/* ------------------------------------------------------------------------- */
void TestBattle::createMenu()
{
  for(uint8_t i = 0; i < kNUM_MENU_ITEMS; i++)
  {
    menu_items.push_back(new Text(font_normal));
    menu_items_sel.push_back(new Text(font_normal));
  }
  menu_index = 0;
}

/* ------------------------------------------------------------------------- */
/* Create person of type */
/* ------------------------------------------------------------------------- */
Person *TestBattle::createPerson(int id, TestPerson type, uint8_t level)
{
  Person *new_person = nullptr;

  /* Create person based on enum - custom logic based on type */
  if(type == ARCADIUS)
  {
    new_person = new Person(id, "Arcadius", race_arcadius, class_arcadius1);
    new_person->setCurves(Element::FOREST, ElementCurve::S, Element::PHYSICAL,
                          ElementCurve::S, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/arcadius.png");
    new_person->setDialogSpritePath(base_path +
                                    "sprites/Overlay/DialogChar/arcadius.png");
  }
  else if(type == AURORAAGENT)
  {
    std::string name = "Agent";

    if(id == 301)
      name = "Agent B";
    else if(id == 302)
      name = "Agent C";

    new_person = new Person(id, name, race_human, class_aurora_agent);
    new_person->setCurves(Element::ELECTRIC, ElementCurve::C, Element::PHYSICAL,
                          ElementCurve::C, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/auroraagent.png");
    new_person->setDialogSpritePath(
        base_path + "sprites/Overlay/DialogChar/auroraagent.png");
  }
  else if(type == AURORAHEAVY)
  {
    new_person = new Person(id, "Heavy", race_human, class_aurora_heavy);
    new_person->setCurves(Element::ELECTRIC, ElementCurve::A, Element::PHYSICAL,
                          ElementCurve::A, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/auroraheavy.png");
    new_person->setDialogSpritePath(
        base_path + "sprites/Overlay/DialogChar/auroraheavy.png");
  }
  else if(type == AURORAENGG)
  {
    new_person = new Person(id, "Engineer", class_aurora_engg, race_human);
    new_person->setCurves(Element::ELECTRIC, ElementCurve::B, Element::PHYSICAL,
                          ElementCurve::B, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/auroraengineer.png");
    new_person->setDialogSpritePath(
        base_path + "sprites/Overlay/DialogChar/auroraengineer.png");
  }
  else if(type == AURORADRONE)
  {
    std::string name = "Drone";

    if(id == 401)
      name = "Drone B";
    else if(id == 402)
      name = "Drone C";

    new_person = new Person(id, name, class_aurora_drone, race_robot);
    new_person->setCurves(Element::ELECTRIC, ElementCurve::D, Element::PHYSICAL,
                          ElementCurve::D, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/auroradrone.png");
    new_person->setDialogSpritePath(
        base_path + "sprites/Overlay/DialogChar/auroradrone.png");
  }
  else if(type == REVERDILE)
  {
    new_person = new Person(id, "Reverdile", race_treefolk, class_reverdile);
    new_person->setCurves(Element::FOREST, ElementCurve::C, Element::PHYSICAL,
                          ElementCurve::C, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/reverdile.png");
    new_person->setDialogSpritePath(base_path +
                                    "sprites/Overlay/DialogChar/treething.png");
  }
  else if(type == REVERDLING)
  {
    std::string name = "Reverdling";

    if(id == 501)
      name = "Reverdling B";
    else if(id == 502)
      name = "Reverdling C";
    else if(id == 503)
      name = "Reverdling D";
    else if(id == 504)
      name = "Reverdling E";

    new_person = new Person(id, name, race_treefolk, class_reverdling);
    new_person->setCurves(Element::FOREST, ElementCurve::D, Element::PHYSICAL,
                          ElementCurve::D, true);

    new_person->setThirdPersonPath(
        base_path + "sprites/Battle/Battle_Persons/reverdling.png");
    new_person->setDialogSpritePath(
        base_path + "sprites/Overlay/DialogChar/reverdling.png");
  }
  else if(type == PLAYER)
  {
    new_person = new Person(id, "Player", race_bear, class_player);
    new_person->setCurves(Element::FOREST, ElementCurve::C, Element::PHYSICAL,
                          ElementCurve::C, true);

    new_person->setFirstPersonPath(base_path +
                                   "sprites/Battle/Backs/player0.png");
    new_person->setDialogSpritePath(base_path +
                                    "sprites/Overlay/DialogChar/player.png");
    new_person->setActionSpritePath(base_path +
                                    "sprites/Battle/Backs/player1.png");
  }

  /* Common person logic */
  if(new_person)
  {
    new_person->loseExp(new_person->getTotalExp());
    new_person->addExp(new_person->getExpAt(level));
  }

  return new_person;
}

/* ------------------------------------------------------------------------- */
/* Create races */
/* ------------------------------------------------------------------------- */
void TestBattle::createRaces()
{
  /* Arcadius Race (He shouldn't have bear skills) */
  race_arcadius =
      new Category(1060, "The Arcadius", "arcadius", stats_arcadius_race_min,
                   stats_arcadius_race_max, getSkillSet(101));
  race_arcadius->setDescription("Blah");
  race_arcadius->setQDRegenRate(RegenRate::WEAK);
  race_arcadius->setVitaRegenRate(RegenRate::WEAK);

  /* Aurora Drone Race - Possibly needed for immunity/smaller stats */
  race_robot = new Category(1061, "Robot", "robot", stats_robot_min,
                            stats_robot_max, getSkillSet(101));
  race_robot->setVitaRegenRate(RegenRate::ZERO);
  race_robot->setQDRegenRate(RegenRate::WEAK);
  race_robot->setDescription("Artificial artificialness");

  /* Bear Race */
  race_bear = new Category(1020, "Bear", "bear", stats_bear_min, stats_bear_max,
                           getSkillSet(1020));
  // race_bear->setVitaRegenRate(RegenRate::WEAK);
  race_bear->setQDRegenRate(RegenRate::ZERO);
  race_bear->setDescription("A sentient and intelligent bear");
  // race_bear->setFlag(CategoryState::DEF_ENABLED, true);
  // race_bear->setFlag(CategoryState::GRD_ENABLED, true);
  // race_bear->setFlag(CategoryState::E_SWORD, true);

  /* Human Race */
  race_human = new Category(1000, "Human", "human", stats_human_min,
                            stats_human_max, getSkillSet(1000));
  race_human->setVitaRegenRate(RegenRate::ZERO);
  race_human->setQDRegenRate(RegenRate::WEAK);
  race_human->setDescription("A regular old joe from Earth");
  // race_human->setFlag(CategoryState::DEF_ENABLED, true);
  // race_human->setFlag(CategoryState::GRD_ENABLED, true);
  // race_human->setFlag(CategoryState::E_SWORD, true);

  /* Treefolk Race */
  race_treefolk = new Category(1040, "Treefolk", "treefolk", stats_treefolk_min,
                               stats_treefolk_max, getSkillSet(1040));
  race_treefolk->setVitaRegenRate(RegenRate::WEAK);
  // race_treefolk->setQDRegenRate(RegenRate::WEAK);
  race_treefolk->setDescription("They have long roots into society");
  // race_treefolk->setFlag(CategoryState::DEF_ENABLED, true);
  // race_treefolk->setFlag(CategoryState::GRD_ENABLED, true);
  // race_treefolk->setFlag(CategoryState::E_SWORD, true);
}

/* ------------------------------------------------------------------------- */
/* Create skills */
/* ------------------------------------------------------------------------- */
void TestBattle::createSkills()
{
  /* Light Push */
  Skill *light_push =
      new Skill(100, "Light Push", ActionScope::ONE_ENEMY, act_dmg[0], 100, 0);
  light_push->setDescription("A weak, physical hit with a long cool down");
  light_push->setPrimary(Element::PHYSICAL);
  light_push->setFlag(SkillFlags::OFFENSIVE);
  light_push->setCooldown(3);
  skills.push_back(light_push);

  /* Light Shot */
  Skill *light_shot =
      new Skill(120, "Light Shot", ActionScope::ONE_ENEMY, act_dmg[7], 95, 0);
  light_shot->setDescription("A standard, electric hit against a single foe");
  light_shot->setPrimary(Element::ELECTRIC);
  light_shot->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(light_shot);

  /* Prismatic Shot */
  Skill *prismatic_shot = new Skill(121, "Prismatic Shot",
                                    ActionScope::ONE_ENEMY, act_dmg[1], 95, 15);
  prismatic_shot->addAction(act_dmg[1]);
  prismatic_shot->addAction(act_dmg[1]);
  prismatic_shot->setDescription("A triple strike electric hit against a foe");
  prismatic_shot->setPrimary(Element::ELECTRIC);
  prismatic_shot->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(prismatic_shot);

  /* Rail Shot */
  Skill *rail_shot =
      new Skill(140, "Rail Shot", ActionScope::ONE_ENEMY, act_dmg[2], 95, 0);
  rail_shot->setDescription("A heavy damage electric hit against a foe");
  rail_shot->setPrimary(Element::ELECTRIC);
  rail_shot->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(rail_shot);

  /* Shatter Shot */
  Skill *shatter_shot = new Skill(141, "ShatterShot", ActionScope::ALL_ENEMIES,
                                  act_dmg[3], 97, 15);
  shatter_shot->addAction(act_dmg[2]);
  shatter_shot->setDescription("This description sucks.");
  shatter_shot->setPrimary(Element::ELECTRIC);
  shatter_shot->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(shatter_shot);

  /* Engineer Upgrade */
  Skill *eng_upgrade =
      new Skill(160, "Upgrade", ActionScope::ONE_ALLY, act_inf[4], 100, 5);
  eng_upgrade->addAction(act_inf[5]);
  eng_upgrade->setDescription("Team upgrade to ally for defense and speed");
  eng_upgrade->setPrimary(Element::ELECTRIC);
  eng_upgrade->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(eng_upgrade);

  /* Static Shot */

  Skill *static_shot =
      new Skill(180, "Static Shot", ActionScope::ONE_ENEMY, act_dmg[4], 95, 0);
  static_shot->setDescription("A weak charged-based attack.");
  static_shot->setPrimary(Element::ELECTRIC);
  static_shot->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(static_shot);

  /* Locked Shot */
  Skill *locked_shot = new Skill(181, "Locked Shot", ActionScope::ONE_ENEMY,
                                 act_dmg[8], 100, 25);
  locked_shot->setDescription("A charged-based attack that will not miss.");
  locked_shot->setPrimary(Element::ELECTRIC);
  locked_shot->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(locked_shot);

  /* Detonate */
  // TODO: Implement imploding

  /* Hidden Amplification */
  Skill *hidden_amp = new Skill(200, "Hidden Amplification", ActionScope::USER,
                                act_inf[3], 97, 5);
  // hidden_amp->addAction(act_inf[4]);
  hidden_amp->setDescription("Raises one's attacking power.");
  hidden_amp->setPrimary(Element::PHYSICAL);
  hidden_amp->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(hidden_amp);

  /* Surge of Will */
  // Skill* surge_will = new Skill(201, "Surge of Will",
  // ActionScope::ALL_ALLIES,
  //   act_alt[0], 90, 25);

  /* Canopy */
  Skill *canopy =
      new Skill(220, "Canopy", ActionScope::ALL_ALLIES, act_inf[4], 90, 5);
  canopy->setDescription("Entire team DEF up.");
  canopy->setPrimary(Element::PHYSICAL);
  canopy->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(canopy);

  /* Updraft */
  Skill *updraft = new Skill(221, "Updraft", ActionScope::ONE_ALLY_NOT_USER,
                             act_inf[5], 90, 10);
  updraft->setDescription("Ally dodge up");
  updraft->setPrimary(Element::PHYSICAL);
  updraft->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(updraft);

  /* Chlorophoria */
  Skill *chlorophoria = new Skill(222, "Chlorophoria", ActionScope::ALL_ALLIES,
                                  act_alt[0], 100, 15);
  chlorophoria->setDescription("Team health up");
  chlorophoria->setPrimary(Element::PHYSICAL);
  chlorophoria->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(chlorophoria);

  /* Numbing Sting */
  Skill *numbing = new Skill(240, "Numbing Sting", ActionScope::ONE_ENEMY,
                             act_inf[6], 90, 5);
  numbing->setDescription("Paralysis Inflict");
  numbing->setPrimary(Element::PHYSICAL);
  numbing->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(numbing);

  /* Hypnotic Sting */
  Skill *hypnotic =
      new Skill(241, "Sullen Sting", ActionScope::ONE_ENEMY, act_inf[2], 90, 5);
  hypnotic->setDescription("Silencing sting");
  hypnotic->setPrimary(Element::PHYSICAL);
  hypnotic->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(hypnotic);

  /* Toxic Sting */
  Skill *toxic =
      new Skill(242, "Toxic Sting", ActionScope::ONE_ENEMY, act_inf[0], 90, 5);
  toxic->setDescription("Poision Inflict");
  toxic->setPrimary(Element::PHYSICAL);
  toxic->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(toxic);

  /* Befuddling Sting */
  Skill *befuddling = new Skill(243, "Befuddling Sting", ActionScope::ONE_ENEMY,
                                act_inf[1], 90, 5);
  befuddling->setDescription("Confusion Inflict");
  befuddling->setPrimary(Element::PHYSICAL);
  befuddling->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(befuddling);

  /* Cunning */
  Skill *cunning =
      new Skill(1000, "Cunning", ActionScope::USER, act_inf[4], 90, 5);
  cunning->addAction(act_inf[5]);
  cunning->setDescription("Self dodge and defense up");
  cunning->setPrimary(Element::PHYSICAL);
  cunning->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(cunning);

  /* Strike */
  Skill *strike =
      new Skill(1001, "Strike", ActionScope::ONE_ENEMY, act_dmg[6], 90, 15);
  strike->setDescription("Physical strike against one foe");
  strike->setPrimary(Element::PHYSICAL);
  strike->setFlag(SkillFlags::OFFENSIVE);

  skills.push_back(strike);

  /* Paw Strike */
  Skill *paw_strike =
      new Skill(1020, "Paw Strike", ActionScope::ONE_ENEMY, act_dmg[9], 100, 0);
  paw_strike->setDescription("A standard, physical hit against a single foe");
  paw_strike->setPrimary(Element::PHYSICAL);
  paw_strike->setSecondary(Element::FOREST);
  paw_strike->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(paw_strike);

  /* Maul */
  Skill *maul =
      new Skill(1021, "Maul", ActionScope::ONE_ENEMY, act_dmg[10], 95, 10);
  maul->setDescription("A stronger, physical hit against a single foe");
  maul->setPrimary(Element::PHYSICAL);
  maul->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(maul);

  /* Multi-Strike */
  Skill *multi_strike = new Skill(1022, "Multi-Strike", ActionScope::ONE_ENEMY,
                                  act_dmg[9], 70, 15);
  multi_strike->addAction(act_dmg[9]);
  multi_strike->setDescription("What");
  multi_strike->setPrimary(Element::PHYSICAL);
  multi_strike->setSecondary(Element::FOREST);
  multi_strike->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(multi_strike);

  /* Hibernate */
  Skill *hibernate =
      new Skill(1023, "Hibernate", ActionScope::USER, act_inf[7], 100, 5);
  hibernate->setDescription("Heal self");
  hibernate->setPrimary(Element::PHYSICAL);
  hibernate->setFlag(SkillFlags::DEFENSIVE);
  skills.push_back(hibernate);

  /* Ensnare */
  // TODO: Slow down portion of ensnare? Ehhh...
  Skill *ensnare =
      new Skill(1040, "Ensnare", ActionScope::ONE_ENEMY, act_dmg[11], 95, 5);
  ensnare->setDescription("Damage enemy");
  ensnare->setPrimary(Element::FOREST);
  ensnare->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(ensnare);

  /* Enrich */
  // TODO: Healing plep
  Skill *enrich =
      new Skill(1041, "Enrich", ActionScope::USER, act_alt[0], 100, 5);
  enrich->setDescription("Small amount of heal to self");
  enrich->setPrimary(Element::FOREST);
  enrich->setFlag(SkillFlags::DEFENSIVE);
  enrich->setFlag(SkillFlags::HEALING);
  skills.push_back(enrich);

  Skill *rock_skill = new Skill(1042, "Rock", ActionScope::ONE_TARGET,
                                act_dmg[8], 100, 5);
  rock_skill->setPrimary(Element::PHYSICAL);
  rock_skill->setFlag(SkillFlags::OFFENSIVE);
  skills.push_back(rock_skill);


  Skill *medkit_skill =
      new Skill(1043, "Medkit", ActionScope::USER, act_alt[0], 100, 5);
  skills.push_back(medkit_skill);
}

void TestBattle::createItems()
{
  auto a_inv = party_friends->getInventory();
  // auto e_inv = party_foes->getInventory();

  rock = new Item(5001, "Rock", 100, nullptr, 0.0);
  rock->setDescription("Destroy your enemies with a magical rock.");

  if(!getSkill(1043))
    std::cout << "Medkit skill 1043 invalid" << std::endl;

  rock->setUseSkill(getSkill(1042));
  rock->setFlag(ItemFlags::CONSUMED, true);

  medkit = new Item(5002, "Medkit", 150, nullptr, 0.0);
  medkit->setDescription("HEALS ALL OF YOUR HEALTH WOOHOO.");

  medkit->setUseSkill(getSkill(1043));
  medkit->setFlag(ItemFlags::CONSUMED, true);
  medkit->setFlag(ItemFlags::HEALING_ITEM, true);

  non_base_rock = new Item(rock);
  non_base_medkit = new Item(medkit);

  a_inv->add(non_base_rock, 5);
  a_inv->add(non_base_medkit, 5);
}

void TestBattle::deleteItems()
{
  // if(party_friends && party_friends->getInventory())
  //   party_friends->getInventory()->clear();
  // if(party_foes && party_foes->getInventory())
  //   party_foes->getInventory()->clear();

  if(non_base_rock)
  {
    delete non_base_rock;
    non_base_rock = nullptr;
  }

  if(non_base_medkit)
  {
    delete non_base_medkit;
    non_base_medkit = nullptr;
  }

  if(rock)
  {
    delete rock;
    rock = nullptr;
  }

  if(medkit)
  {
    delete medkit;
    medkit = nullptr;
  }
}

/* ------------------------------------------------------------------------- */
/* Create Skill Sets */
/* ------------------------------------------------------------------------- */
void TestBattle::createSkillSets()
{
  /* ---- CLASSES ---- */

  /* The {} Set */
  SkillSet *set_empty = new SkillSet(101);
  skillsets.push_back(set_empty);

  /* Arcadius Class */
  SkillSet *set_arcadius = new SkillSet(100);
  set_arcadius->addSkill(getSkill(100), 1);
  skillsets.push_back(set_arcadius);

  /* Aurora Agent Class */
  SkillSet *set_agent = new SkillSet(120);
  set_agent->addSkill(getSkill(120), 1);
  set_agent->addSkill(getSkill(121), 12);
  skillsets.push_back(set_agent);

  /* Aurora Heavy Class */
  SkillSet *set_heavy = new SkillSet(140);
  set_heavy->addSkill(getSkill(140), 1);
  set_heavy->addSkill(getSkill(141), 12);
  skillsets.push_back(set_heavy);

  /* Aurora Engineer Class */
  SkillSet *set_engineer = new SkillSet(160);
  set_engineer->addSkill(getSkill(160), 1);
  set_engineer->addSkill(getSkill(120), 1);
  skillsets.push_back(set_engineer);

  /* Aurora Drone Class */
  SkillSet *set_drone = new SkillSet(180);
  set_drone->addSkill(getSkill(180), 1);
  set_drone->addSkill(getSkill(181), 5);
  set_drone->addSkill(getSkill(182), 13);
  skillsets.push_back(set_drone);

  /* Player Class */
  SkillSet *set_player = new SkillSet(200);
  set_player->addSkill(getSkill(200), 1);
  set_player->addSkill(getSkill(201), 15);
  skillsets.push_back(set_player);

  /* Reverdile */
  SkillSet *set_reverdile = new SkillSet(220);
  set_reverdile->addSkill(getSkill(220), 1);
  set_reverdile->addSkill(getSkill(221), 5);
  set_reverdile->addSkill(getSkill(222), 8);
  skillsets.push_back(set_reverdile);

  /* Reverdling */
  SkillSet *set_reverdling = new SkillSet(240);
  set_reverdling->addSkill(getSkill(240), 1);
  set_reverdling->addSkill(getSkill(241), 1);
  set_reverdling->addSkill(getSkill(242), 1);

  set_reverdling->addSkill(getSkill(243), 1);
  skillsets.push_back(set_reverdling);

  /* ---- RACES ---- */

  /* Human Race */
  SkillSet *set_human = new SkillSet(1000);
  // set_human->addSkill(getSkill(1000), 1);
  set_human->addSkill(getSkill(1001), 6);
  skillsets.push_back(set_human);

  /* Bear Race */
  SkillSet *set_bear = new SkillSet(1020);
  set_bear->addSkill(getSkill(1020), 1);
  set_bear->addSkill(getSkill(1021), 5);
  set_bear->addSkill(getSkill(1022), 8);
  set_bear->addSkill(getSkill(1023), 13);
  skillsets.push_back(set_bear);

  /* Treefolk Race */
  SkillSet *set_treefolk = new SkillSet(1040);
  set_treefolk->addSkill(getSkill(1040), 1);
  set_treefolk->addSkill(getSkill(1041), 6);
  skillsets.push_back(set_treefolk);

  /* ---- BUBBIFIED SKILL SET ---- */
  SkillSet *set_bubbified = new SkillSet(1);
  set_bubbified->addSkill(getSkill(1020), 1);
  skillsets.push_back(set_bubbified);
}

/* ------------------------------------------------------------------------- */
/* Delete actions */
/* ------------------------------------------------------------------------- */
void TestBattle::deleteActions()
{
  /* Alter Actions */
  for(uint8_t i = 0; i < act_alt.size(); i++)
    delete act_alt[i];
  act_alt.clear();

  /* Assign Actions */
  for(uint8_t i = 0; i < act_asn.size(); i++)
    delete act_asn[i];
  act_asn.clear();

  /* Damage Actions */
  for(uint8_t i = 0; i < act_dmg.size(); i++)
    delete act_dmg[i];
  act_dmg.clear();

  /* Inflict Actions */
  for(uint8_t i = 0; i < act_inf.size(); i++)
    delete act_inf[i];
  act_inf.clear();

  /* Relieve Actions */
  for(uint8_t i = 0; i < act_rlv.size(); i++)
    delete act_rlv[i];
  act_rlv.clear();
}

/* ------------------------------------------------------------------------- */
/* Delete classes */
/* ------------------------------------------------------------------------- */
void TestBattle::deleteClasses()
{
  delete class_arcadius1;
  class_arcadius1 = nullptr;

  delete class_aurora_agent;
  class_aurora_agent = nullptr;

  delete class_aurora_heavy;
  class_aurora_heavy = nullptr;

  delete class_aurora_drone;
  class_aurora_drone = nullptr;

  delete class_aurora_engg;
  class_aurora_engg = nullptr;

  delete class_player;
  class_player = nullptr;

  delete class_reverdile;
  class_reverdile = nullptr;

  delete class_reverdling;
  class_reverdling = nullptr;
}

/* ------------------------------------------------------------------------- */
/* Delete menu */
/* ------------------------------------------------------------------------- */
void TestBattle::deleteMenu()
{
  for(uint8_t i = 0; i < menu_items.size(); i++)
  {
    delete menu_items[i];
    delete menu_items_sel[i];
  }

  menu_items.clear();
  menu_items_sel.clear();
}

/* ------------------------------------------------------------------------- */
/* Delete races */
/* ------------------------------------------------------------------------- */
void TestBattle::deleteRaces()
{
  delete race_arcadius;
  race_arcadius = nullptr;

  delete race_robot;
  race_robot = nullptr;

  delete race_bear;
  race_bear = nullptr;

  delete race_human;
  race_human = nullptr;

  delete race_treefolk;
  race_treefolk = nullptr;
}

/* ------------------------------------------------------------------------- */
/* Delete skills */
/* ------------------------------------------------------------------------- */
void TestBattle::deleteSkills()
{
  for(auto &skill : skills)
    delete skill;
  skills.clear();
}

/* ------------------------------------------------------------------------- */
/* Delete skill sets */
/* ------------------------------------------------------------------------- */
void TestBattle::deleteSkillSets()
{
  for(auto &skillset : skillsets)
    delete skillset;
  skillsets.clear();
}

/* ------------------------------------------------------------------------- */
/* Main destruction call - ending */
/* ------------------------------------------------------------------------- */
void TestBattle::destroy()
{
  mode = NONE;

  // /* Delete battle and information */
  // deleteItems();
  destroyBattle();

  // /* Delete skill information */
  // deleteClasses();
  // deleteRaces();
  // deleteSkillSets();
  // deleteSkills();

  // deleteActions();

  // /* Delete fonts and menus */
  // deleteMenu();
}

/* Battle destruction */
void TestBattle::destroyBattle()
{
  if(battle_logic != nullptr)
  {
    delete battle_logic;
    battle_logic = nullptr;
  }

  /* Foe party list delete */
  if(party_foes)
  {
    for(auto &member : party_foes->getMembers())
    {
      if(member)
        delete member;
      member = nullptr;
    }

    delete party_foes;
    party_foes = nullptr;
  }

  /* Friend party list delete */
  if(party_friends)
  {
    for(auto &member : party_friends->getMembers())
      if(member)
        delete member;

    delete party_friends;
    party_friends = nullptr;
  }
}

/* ------------------------------------------------------------------------- */
/* Get call for Skill by ID */
/* ------------------------------------------------------------------------- */
Skill *TestBattle::getSkill(int id)
{
  Skill *found_skill = nullptr;

  for(uint16_t i = 0; i < skills.size(); i++)
    if(skills[i]->getID() == id)
      found_skill = skills[i];

  return found_skill;
}

/* ------------------------------------------------------------------------- */
/* Get call for Skill Set by ID */
/* ------------------------------------------------------------------------- */
SkillSet *TestBattle::getSkillSet(int id)
{
  SkillSet *found_set = nullptr;

  for(uint16_t i = 0; i < skillsets.size(); i++)
    if(skillsets[i]->getID() == id)
      found_set = skillsets[i];

  return found_set;
}

/* ------------------------------------------------------------------------- */
/* Set up the battle */
/* ------------------------------------------------------------------------- */
void TestBattle::initBattle(SDL_Renderer *renderer)
{
  MenuItems ref = (MenuItems)menu_index;

  /* Set up FRIEND party */
  Person *player = createPerson(100, PLAYER, lvl_friend);
  party_friends = new Party(200, player, PartyType::SLEUTH, 5);

  /* Set up FOE party based on menu index */
  if(ref == AC)
  {
    Person *arc = createPerson(300, ARCADIUS, lvl_foe);
    party_foes = new Party(400, arc, PartyType::REGULAR_FOE, 5);
  }
  else if(ref == AA)
  {
    Person *aa = createPerson(300, AURORAAGENT, lvl_foe);
    party_foes = new Party(400, aa, PartyType::REGULAR_FOE, 5);
  }
  else if(ref == AAx2)
  {
    Person *aa1 = createPerson(300, AURORAAGENT, lvl_foe);
    Person *aa2 = createPerson(301, AURORAAGENT, lvl_foe);

    party_foes = new Party(400, aa1, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(aa2);
  }
  else if(ref == AAx5)
  {
    Person *aa1 = createPerson(300, AURORAAGENT, lvl_foe);
    Person *aa2 = createPerson(301, AURORAAGENT, lvl_foe);
    Person *aa3 = createPerson(302, AURORAAGENT, lvl_foe);
    // Person *aa4 = createPerson(303, AURORAAGENT, renderer, lvl_foe);
    // Person *aa5 = createPerson(304, AURORAAGENT, renderer, lvl_foe);

    party_foes = new Party(400, aa1, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(aa2);
    party_foes->addMember(aa3);
    // party_foes->addMember(aa4);
    // party_foes->addMember(aa5);
  }
  else if(ref == AAnAH)
  {
    Person *aa = createPerson(300, AURORAAGENT, lvl_foe);
    Person *ah = createPerson(301, AURORAHEAVY, lvl_foe);

    party_foes = new Party(400, aa, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(ah);
  }
  else if(ref == AH)
  {
    Person *ah = createPerson(300, AURORAHEAVY, lvl_foe);
    party_foes = new Party(400, ah, PartyType::REGULAR_FOE, 5);
  }
  else if(ref == AEnAD)
  {
    Person *ae = createPerson(300, AURORAENGG, lvl_foe);
    Person *ad = createPerson(400, AURORADRONE, lvl_foe);

    party_foes = new Party(400, ae, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(ad);
  }
  else if(ref == AEnADx4)
  {
    Person *ae = createPerson(300, AURORAENGG, lvl_foe);
    Person *ad1 = createPerson(400, AURORADRONE, lvl_foe);
    Person *ad2 = createPerson(401, AURORADRONE, lvl_foe);
    Person *ad3 = createPerson(402, AURORADRONE, lvl_foe);
    // Person *ad4 = createPerson(304, AURORADRONE, renderer, lvl_foe);

    party_foes = new Party(400, ae, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(ad1);
    party_foes->addMember(ad2);
    party_foes->addMember(ad3);
    // party_foes->addMember(ad4);
  }
  else if(ref == AAnAHnADnAE)
  {
    Person *aa = createPerson(300, AURORAAGENT, lvl_foe);
    Person *ah = createPerson(301, AURORAHEAVY, lvl_foe);
    Person *ad = createPerson(400, AURORADRONE, lvl_foe);
    Person *ae = createPerson(303, AURORAENGG, lvl_foe);

    party_foes = new Party(400, aa, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(ah);
    party_foes->addMember(ad);
    party_foes->addMember(ae);
  }
  else if(ref == REnRG)
  {
    Person *re = createPerson(300, REVERDILE, lvl_foe);
    Person *rg = createPerson(500, REVERDLING, lvl_foe);

    party_foes = new Party(400, re, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(rg);
  }
  else if(ref == REnRGx2)
  {
    Person *re = createPerson(300, REVERDILE, lvl_foe);
    Person *rg1 = createPerson(500, REVERDLING, lvl_foe);
    Person *rg2 = createPerson(501, REVERDLING, lvl_foe);

    party_foes = new Party(400, re, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(rg1);
    party_foes->addMember(rg2);
  }
  else if(ref == REnRGx4)
  {
    Person *re = createPerson(300, REVERDILE, lvl_foe);
    Person *rg1 = createPerson(500, REVERDLING, lvl_foe);
    Person *rg2 = createPerson(501, REVERDLING, lvl_foe);
    Person *rg3 = createPerson(502, REVERDLING, lvl_foe);
    // Person *rg4 = createPerson(304, REVERDLING, renderer, lvl_foe);

    party_foes = new Party(400, re, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(rg1);
    party_foes->addMember(rg2);
    party_foes->addMember(rg3);
    // party_foes->addMember(rg4);
  }
  else if(ref == RG)
  {
    Person *rg = createPerson(300, REVERDLING, lvl_foe);
    party_foes = new Party(400, rg, PartyType::REGULAR_FOE, 5);
  }
  else if(ref == RGx2)
  {
    Person *rg1 = createPerson(500, REVERDLING, lvl_foe);
    Person *rg2 = createPerson(501, REVERDLING, lvl_foe);

    party_foes = new Party(400, rg1, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(rg2);
  }
  else if(ref == RGx5)
  {
    Person *rg1 = createPerson(500, REVERDLING, lvl_foe);
    Person *rg2 = createPerson(501, REVERDLING, lvl_foe);
    Person *rg3 = createPerson(502, REVERDLING, lvl_foe);
    Person *rg4 = createPerson(503, REVERDLING, lvl_foe);
    Person *rg5 = createPerson(504, REVERDLING, lvl_foe);

    party_foes = new Party(400, rg1, PartyType::REGULAR_FOE, 5);
    party_foes->addMember(rg2);
    party_foes->addMember(rg3);
    party_foes->addMember(rg4);
    party_foes->addMember(rg5);
  }

  /* Set up battle */
  // auto background = new Sprite(
  //     base_path + "sprites/Battle/Backdrop/battlebg00.png", renderer);

  battle_logic->setRenderer(renderer);
  battle_logic->setEventHandler(event_handler);
  display_data->setRenderer(renderer);

  if(!display_data->isDataBuilt())
    display_data->buildData();

  createItems();

  // TODO: base_path instead of background (temporary)
  battle_logic->startBattle(party_friends, party_foes, base_path);
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

/* ------------------------------------------------------------------------- */
/* The key up and down events to be handled by the class */
/* ------------------------------------------------------------------------- */
bool TestBattle::keyDownEvent(SDL_KeyboardEvent event)
{
  if(mode == TEST_BATTLE)
  {
    return battle_logic->keyDownEvent(event);
  }
  else
  {
    /* Exit the test battle: finished processing */
    if(event.keysym.sym == SDLK_UP)
    {
      if(mode == SCENARIO)
      {
        if(menu_index > 0)
          menu_index--;
      }
      else if(mode == FRIEND_LVL)
      {
        if(lvl_friend < kLVL_MAX)
          lvl_friend++;
      }
      else if(mode == FOE_LVL)
      {
        if(lvl_foe < kLVL_MAX)
          lvl_foe++;
      }
    }
    else if(event.keysym.sym == SDLK_DOWN)
    {
      if(mode == SCENARIO)
      {
        if((menu_index + 1) < kNUM_MENU_ITEMS)
          menu_index++;
      }
      else if(mode == FRIEND_LVL)
      {
        if(lvl_friend > 1)
          lvl_friend--;
      }
      else if(mode == FOE_LVL)
      {
        if(lvl_foe > 1)
          lvl_foe--;
      }
    }
    else if(event.keysym.sym == SDLK_SPACE || event.keysym.sym == SDLK_RETURN)
    {
      if(mode == SCENARIO)
      {
        if(kMENU_ITEMS[menu_index] == "Exit")
        {
          return true;
        }
        else
        {
          mode = FRIEND_LVL;
        }
      }
      else if(mode == FRIEND_LVL)
      {
        mode = FOE_LVL;
      }
      else if(mode == FOE_LVL)
      {
        battle_start = true;
      }
    }
    else if(event.keysym.sym == SDLK_ESCAPE)
    {
      if(mode == SCENARIO)
      {
        menu_index = kNUM_MENU_ITEMS - 1;
      }
      else if(mode == FRIEND_LVL)
      {
        mode = SCENARIO;
      }
      else if(mode == FOE_LVL)
      {
        mode = FRIEND_LVL;
      }
    }
  }

  return false;
}

/* ------------------------------------------------------------------------- */
/* The key up and down events to be handled by the class */
/* ------------------------------------------------------------------------- */
void TestBattle::keyUpEvent(SDL_KeyboardEvent event)
{
  (void)event;
}

/* ------------------------------------------------------------------------- */
/* Renders the title screen */
/* ------------------------------------------------------------------------- */
bool TestBattle::render(SDL_Renderer *renderer)
{
  if(renderer)
  {
    /* First run rendering */
    if(first_run)
    {
      buildBattleDisplay(renderer);
      first_run = false;
    }

    /* Battle start first rendering */
    if(battle_start)
    {
      initBattle(renderer);
      mode = TEST_BATTLE;
      battle_start = false;
    }

    /* Rendering primary */
    if(mode == TEST_BATTLE)
    {
      battle_logic->render();
    }
    else
    {
      /* Render text */
      for(uint8_t i = 0; i < menu_items.size(); i++)
      {
        if(i == menu_index)
          menu_items_sel[i]->render(renderer, 75, 40 + 40 * i);
        else
          menu_items[i]->render(renderer, 75, 40 + 40 * i);
      }

      /* Render friend level */
      if(mode == FRIEND_LVL || mode == FOE_LVL)
      {
        Text t(font_normal);
        t.setText(renderer, kMENU_FRIEND + std::to_string(lvl_friend),
                  {255, 255, 255, 255});
        t.render(renderer, 600, 40);
      }

      /* Render foe level */
      if(mode == FOE_LVL)
      {
        Text t(font_normal);
        t.setText(renderer, kMENU_FOE + std::to_string(lvl_foe),
                  {255, 255, 255, 255});
        t.render(renderer, 600, 140);
      }
    }
  }

  return true;
}

/* ------------------------------------------------------------------------- */
/* Set the running configuration, from the options class */
/* ------------------------------------------------------------------------- */
bool TestBattle::setConfiguration(Options *running_config)
{
  if(running_config)
  {
    game_config = running_config;
    base_path = game_config->getBasePath();

    if(battle_logic)
      battle_logic->setConfig(running_config);
    if(display_data)
      display_data->setConfig(running_config);

    font_normal = game_config->getFontTTF(FontName::REGULAR_FONT);

    /* Text setup */
    deleteMenu();
    createMenu();
  }

  return running_config;
}

bool TestBattle::setEventHandler(EventHandler *event_handler)
{
  this->event_handler = event_handler;

  return this->event_handler;
}

/* ------------------------------------------------------------------------- */
/* Updates the game state */
/* ------------------------------------------------------------------------- */
bool TestBattle::update(int cycle_time)
{
  if(mode == TEST_BATTLE && battle_logic)
  {
    battle_logic->update(cycle_time);

    if(battle_logic->getTurnState() == TurnState::FINISHED)
    {
      battle_logic->stopBattle();
      deleteItems();
    }
    if(battle_logic->getTurnState() == TurnState::STOPPED)
      mode = SCENARIO;
  }

  return false;
}
