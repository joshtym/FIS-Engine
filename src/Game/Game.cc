/*******************************************************************************
 * Class Name: Game
 * Date Created: Dec 2 2012
 * Inheritance: none
 * Description: This class is the top layer that handles all interclass
 *              communication between map, player, and battle as well as all
 *              setup and overall insight and control. The painting control
 *              won't be handled here and this will just act as an intermediate
 *              data highway / event handler.
 *
 * TODO:
 *  1. Add Event Handler. Integrate in to handle all exchanges between class
 *  2. Talk to Mike about Battle and how we should keep it constructed and then
 *     just change the player and foes when it switches. This is primarily
 *     for dealing with the stacked widget.
 *  3. Add victory screen logic
 *  4. Add speed up button to allow the game to accelerate movement and
 *     everything else. Do it by multiplying the time elapsed.
 ******************************************************************************/
#include "Game/Game.h"

/*=============================================================================
 * CONSTANTS
 *============================================================================*/

const uint32_t Game::kMONEY_ITEM_ID{0};

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/* Constructor function */
Game::Game(Options* running_config)
{
  /* Initalize class variables */
  active_renderer = nullptr;
  base_path = "";
  game_battle    = nullptr;
  game_inventory = nullptr;
  game_config    = nullptr;
  game_map       = nullptr;
  mode = DISABLED;
  
  /* Set game configuration */
  setConfiguration(running_config);

  /* Set up the render classes */
  setupPlayerInventory();
  setupMap();
}

/* Destructor function */
Game::~Game()
{
  game_config = nullptr;
  mode = DISABLED;

  /* Delete battle */
  if(game_battle != nullptr)
  {
    delete game_battle;
    game_battle = nullptr;
  }

  /* Delete map */
  if(game_map != nullptr)
  {
    delete game_map;
    game_map = nullptr;
  }

  //TODO: Delete player -> delete inventory instead.
  if(game_inventory != nullptr)
  {
    delete game_inventory;
    game_inventory = nullptr;
  }

  /* Delete all game actions */
  for(auto it = begin(action_list); it != end(action_list); ++it)
    delete(*it);

  /* Delete battle class categories actions */
  for(auto it = begin(battle_class_list); it != end(battle_class_list); ++it)
    delete(*it);

  /* Delete all race categories*/
  for(auto it = begin(race_list); it != end(race_list); ++it)
    delete(*it);

  /* Delete all flavour categories */
  for(auto it = begin(flavour_list); it != end(flavour_list); ++it)
    delete(*it);

  /* Delete all skills */
  for(auto it = begin(skill_list); it != end(skill_list); ++it)
    delete(*it);

  for(auto it = begin(base_person_list); it != end(base_person_list); ++it)
  /* Delete all game actions */
    delete(*it);

  /* Delete all game base items */
  for(auto it = begin(base_item_list); it != end(base_item_list); ++it)
    delete(*it);
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/

/* A give item event, based on an ID and count (triggered from stored event */
bool Game::eventGiveItem(int id, int count)
{
  /* Attempt to find the item */
  Item* found_item = nullptr;
  for(auto it = base_item_list.begin(); it != base_item_list.end(); it++)
    if((*it)->getGameID() == id)
      found_item = (*it);

  /* If the item was inserted, display pickup notification */
  if(found_item != nullptr)
  {
    // TODO: Attempt insert. For now, according to variable
    bool inserted = true;
    
    /* If inserted, notify that the pickup was a success */
    if(inserted)
    {
      if(game_map != nullptr)
        game_map->initNotification(found_item->getThumb(), count);
      return true;
    }
    /* Otherwise, notify that item could not be received */
    else
    {
      if(game_map != nullptr)
        game_map->initNotification("Insufficient room in inventory to fit " + 
                                   std::to_string(count) + " " + 
                                   found_item->getName());
    }
  }
  
  return false;
}
  
 /* Initiates a conversation event */
 void Game::eventInitConversation(Conversation* convo, MapThing* source)
{
  if(game_map != nullptr)
    game_map->initConversation(convo, source);
}

/* Initiates a notification event (in map) */
void Game::eventInitNotification(std::string notification)
{
  if(game_map != nullptr)
    game_map->initNotification(notification);
}

/* The pickup item event - from walking over or triggering from action key */
void Game::eventPickupItem(MapItem* item, bool walkover)
{
  if(item != nullptr && item->isWalkover() == walkover)
  {
    bool was_inserted = eventGiveItem(item->getCoreID(), item->getCount());
    
    /* If the insert was successful, pickup the item */
    if(game_map != nullptr && was_inserted)
     game_map->pickupItem(item);
  }
}

/* Starts a battle event. Using the given information - TODO */
void Game::eventStartBattle()
{
  setupBattle();
  mode = BATTLE;
}

/* Teleport thing event, based on ID and coordinates */
void Game::eventTeleportThing(int thing_id, int x, int y, int section_id)
{
  if(game_map != nullptr)
    game_map->teleportThing(thing_id, x, y, section_id);
}
  
void Game::pollEvents()
{
  do
  {
    EventClassifier classification = event_handler.pollEventType();
    
    /* Poll classification */
    if(classification == EventClassifier::GIVEITEM)
    {
      int id;
      int count;
      event_handler.pollGiveItem(&id, &count);
      eventGiveItem(id, count);
    }
    else if(classification == EventClassifier::NOTIFICATION)
    {
      std::string notification;
      event_handler.pollNotification(&notification);
      eventInitNotification(notification);
    }
    else if(classification == EventClassifier::PICKUPITEM)
    {
      MapItem* item;
      bool walkover;
      event_handler.pollPickupItem(&item, &walkover);
      eventPickupItem(item, walkover);
    }
    else if(classification == EventClassifier::RUNBATTLE)
    {
      eventStartBattle();
    }
    else if(classification == EventClassifier::RUNMAP)
    {
    
    }
    else if(classification == EventClassifier::STARTCONVO)
    {
      Conversation* convo;
      MapThing* source;
      event_handler.pollConversation(&convo, &source);
      eventInitConversation(convo, source);
    }
    else if(classification == EventClassifier::TELEPORTTHING)
    {
      int thing_id, x, y, section_id;
      event_handler.pollTeleportThing(&thing_id, &x, &y, &section_id);
      eventTeleportThing(thing_id, x, y, section_id);
    }

  } while(event_handler.pollEvent());
  
  event_handler.pollClear();
}

/* Set up the battle - old battle needs to be deleted prior to calling */
void Game::setupBattle()
{
  // Test Actions
  std::vector<Action*> damage_actions;
  damage_actions.push_back(new Action("1,DAMAGE,,,,VITA,AMOUNT.20,AMOUNT.5,,95"));
  damage_actions.push_back(new Action("2,DAMAGE,,,,VITA,AMOUNT.20,AMOUNT.5,,95"));
  damage_actions.push_back(new Action("3,DAMAGE,,,,VITA,AMOUNT.20,AMOUNT.5,,95"));
  damage_actions.push_back(new Action("4,DAMAGE,,,,VITA,AMOUNT.20,AMOUNT.5,,95"));
  damage_actions.push_back(new Action("5,DAMAGE,,,,VITA,AMOUNT.60,AMOUNT.20,,95"));
  damage_actions.push_back(new Action("6,DAMAGE,,,,VITA,AMOUNT.75,AMOUNT.25,,95"));
  damage_actions.push_back(new Action("7,DAMAGE,,,,VITA,AMOUNT.90,AMOUNT.25,,95"));
  damage_actions.push_back(new Action("8,DAMAGE,,,,VITA,AMOUNT.100,AMOUNT.30,,95"));
  damage_actions.push_back(new Action("9,DAMAGE,,,,VITA,AMOUNT.120,AMOUNT.35,,95"));
  damage_actions.push_back(new Action("10,DAMAGE,,,,VITA,AMOUNT.150,AMOUNT.50,,95"));
  damage_actions.push_back(new Action("11,DAMAGE,,,,VITA,AMOUNT.20,PC.1,,95"));
  damage_actions.push_back(new Action("12,DAMAGE,,,,VITA,AMOUNT.40,PC.5,,95"));
  damage_actions.push_back(new Action("13,DAMAGE,,,,VITA,AMOUNT.60,PC.10,,95"));
  damage_actions.push_back(new Action("14,DAMAGE,,,,VITA,AMOUNT.75,PC.15,,95"));
  damage_actions.push_back(new Action("15,DAMAGE,,,,VITA,AMOUNT.100,PC.25,,95"));
  damage_actions.push_back(new Action("16,DAMAGE,,,,VITA,AMOUNT.150,PC.30,,95"));
  damage_actions.push_back(new Action("17,DAMAGE,,,,VITA,PC.2,PC.1,,95"));
  damage_actions.push_back(new Action("18,DAMAGE,,,,VITA,PC.5,PC.2,,95"));
  damage_actions.push_back(new Action("19,DAMAGE,,,,VITA,PC.10,PC.5,,95"));
  damage_actions.push_back(new Action("20,DAMAGE,,,,VITA,PC.10,PC.9,,95"));
  damage_actions.push_back(new Action("22,DAMAGE,,,,VITA,PC.10,PC.20,,95"));
  damage_actions.push_back(new Action("23,DAMAGE,,,,VITA,PC.15,PC.25,,95"));

  // for(auto it = begin(damage_actions); it != end(damage_actions); ++it)
  //   std::cout << (*it)->actionFlag(ActionFlags::VALID) << std::endl;

  // Test Skills
  Skill* physical_01 = new Skill(100, "Wee Strike", ActionScope::ONE_ENEMY, 
      damage_actions[0], 95, 0);
  physical_01->setPrimary(Element::PHYSICAL);

  Skill* physical_02 = new Skill(101, "Whelp", ActionScope::ONE_ENEMY, 
      damage_actions[1],  95, 10);
  physical_02->setPrimary(Element::PHYSICAL);

  Skill* physical_03 = new Skill(102, "Two Smackeroos", 
      ActionScope::TWO_ENEMIES, damage_actions[2], 95, 10);
  physical_03->setPrimary(Element::PHYSICAL);

  Skill* physical_04 = new Skill(103, "Mass Smack", ActionScope::ALL_ENEMIES, 
      damage_actions[3], 85, 25);
  physical_04->setPrimary(Element::PHYSICAL);

  Skill* fire_01 = new Skill(110, "Fire Strike", ActionScope::ONE_ENEMY, 
    damage_actions[4], 90, 3);
  fire_01->setPrimary(Element::FIRE);

  Skill* forest_01 = new Skill(111, "Earth Strike", ActionScope::ONE_ENEMY,
    damage_actions[4], 91, 4);
  forest_01->setPrimary(Element::FOREST);

  Skill* ice_01 = new Skill(112, "Frosty Spray", ActionScope::TWO_ENEMIES, 
    damage_actions[5], 99, 5);
  ice_01->setPrimary(Element::ICE);

  Skill* electric_01 = new Skill(113, "Shock", ActionScope::ALL_ENEMIES,
    damage_actions[6], 99, 6);
  electric_01->setPrimary(Element::ELECTRIC);

  Skill* digital_01 = new Skill(114, "Compile", ActionScope::ONE_ENEMY,
    damage_actions[6], 99, 6);
  digital_01->setPrimary(Element::DIGITAL);

  Skill* void_01 = new Skill(115, "Demi", ActionScope::ONE_ENEMY,
    damage_actions[7], 99, 7);
  void_01->setPrimary(Element::NIHIL);

  // Test Skill Sets
  SkillSet* physical_skills = new SkillSet(physical_01, 1);
  physical_skills->addSkill(physical_02, 1);
  physical_skills->addSkill(physical_03, 1);
  physical_skills->addSkill(physical_04, 1);

  SkillSet* elemental_skills = new SkillSet(fire_01, 1);
  elemental_skills->addSkill(forest_01, 1);
  elemental_skills->addSkill(ice_01, 1);
  elemental_skills->addSkill(electric_01, 1);
  elemental_skills->addSkill(digital_01, 1);
  elemental_skills->addSkill(void_01, 1);

  // Test Attribute Sets
  AttributeSet weak_stats        = AttributeSet(1, true, false);
  AttributeSet not_as_weak_stats = AttributeSet(2, true, false);
  AttributeSet normal_stats      = AttributeSet(3, true, false);
  AttributeSet medium_stats      = AttributeSet(4, true, false);
  AttributeSet top_stats         = AttributeSet(5, true, false);
  AttributeSet boss_stats        = AttributeSet(6, true, false);

  // Test Categories
  Category* human = new Category(200, "Human", "human", normal_stats, 
      top_stats, physical_skills);
  human->setDescription("A regular old joe from Earth.");
  human->setFlag(CategoryState::DEF_ENABLED, true);
  human->setFlag(CategoryState::GRD_ENABLED, true);
  human->setFlag(CategoryState::E_SWORD, true);

  Category* bear = new Category(201, "Bear", "bear", normal_stats,
    top_stats, physical_skills);
  bear->setDescription("A sentient and intelligent bear!");
  bear->setFlag(CategoryState::DEF_ENABLED, true);
  bear->setFlag(CategoryState::GRD_ENABLED, true);
  bear->setFlag(CategoryState::E_CLAWS, true);

  Category* bloodclaw_scion = new Category(251, "Bloodclaw Scion", "scions", 
      weak_stats, boss_stats, elemental_skills);
  bloodclaw_scion->setDescription("A class of soldier excelling in physical combat.");
  bloodclaw_scion->setFlag(CategoryState::DEF_ENABLED, true);
  bloodclaw_scion->setFlag(CategoryState::POWER_DEFENDER, false);
  bloodclaw_scion->setFlag(CategoryState::POWER_GUARDER, false);
  bloodclaw_scion->setFlag(CategoryState::E_SWORD, true);
  bloodclaw_scion->setFlag(CategoryState::E_CLAWS, true);

  Category* tactical_samurai = new Category(252, "Tactical Samurai", "samurai",
      not_as_weak_stats, medium_stats, elemental_skills);
  tactical_samurai->setDescription("A class of swordsman like no other.");
  tactical_samurai->setFlag(CategoryState::DEF_ENABLED, true);
  tactical_samurai->setFlag(CategoryState::POWER_DEFENDER, true);
  tactical_samurai->setFlag(CategoryState::POWER_GUARDER, false);
  tactical_samurai->setFlag(CategoryState::E_SWORD, true);
  tactical_samurai->setFlag(CategoryState::E_CLAWS, false);

  tactical_samurai->setVitaRegenRate(RegenRate::ZERO);

  bear->setVitaRegenRate(RegenRate::WEAK);

  bloodclaw_scion->setVitaRegenRate(RegenRate::WEAK);

  human->setQDRegenRate(RegenRate::NORMAL);

  // Test Persons
  base_person_list.push_back(new Person(300, "Malgidus", human,
      tactical_samurai));
  getPerson(300)->addExp(1500000);

  base_person_list.push_back(new Person(301, "Arcadius", bear, 
    tactical_samurai));
  getPerson(301)->addExp(2100000);
  getPerson(301)->setCurves(Element::FIRE, ElementCurve::B,
                            Element::FOREST, ElementCurve::A, true);


  // Person* berran   = new Person(302, "Berran", bear, bloodclaw_scion);
  // Person* atkst    = new Person(303, "Atkst", human, bloodclaw_scion);
  // Person* kevin    = new Person(304, "Kevin", human, bloodclaw_scion);
  // Person* george   = new Person(305, "George", human, bloodclaw_scion);
  // Person* gregory  = new Person(306, "Gregory", human, bloodclaw_scion);
  // Person* georgina = new Person(307, "Georgina", human, bloodclaw_scion);
  // Person* gerald   = new Person(308, "Gerald", human, bloodclaw_scion);
  // Person* geoff    = new Person(309, "Geoff", human, bloodclaw_scion);

  std::vector<BattleItem> items;

  base_person_list.push_back(new Person(310, "Frosty", human, bloodclaw_scion));
  getPerson(310)->addExp(4000000);
  getPerson(310)->setCurves(Element::ICE, ElementCurve::C,
                            Element::PHYSICAL, ElementCurve::D, true);

  base_person_list.push_back(new Person(311, "Cloud Dude", human, bloodclaw_scion));
  getPerson(311)->addExp(2000);
  
  // Person* thruster_barrow = new Person(301, "Thruster Barrow", human, bloodclaw_scion);
  // Person* dragon = new Person(301, "Dragon", human, bloodclaw_scion);

  // Person* splurge = new Person(301, "Splurge", human, bloodclaw_scion);
  // Person* schwep = new Person(301, "Schwep", human, bloodclaw_scion);
  // Person* hamburger = new Person(301, "Hamburger", human, bloodclaw_scion);
  // Person* swiss_cheese = new Person(301, "Swiss Cheese", human, bloodclaw_scion);

  AIModule* frosty_module = new AIModule();
  frosty_module->setParent(getPerson(310));
  getPerson(310)->setAI(frosty_module);

  AIModule* cloud_module = new AIModule();
  cloud_module->setParent(getPerson(311));
  getPerson(311)->setAI(cloud_module);

  // AIModule* thruster_barrow_module = new AIModule();
  // thruster_barrow_module->setParent(thruster_barrow);
  // thruster_barrow->setAI(thruster_barrow_module);

  // AIModule* dragon_module = new AIModule();
  // dragon_module->setParent(dragon);
  // dragon->setAI(dragon_module);

  // AIModule* splurge_module = new AIModule();
  // splurge_module->setParent(splurge);
  // splurge->setAI(splurge_module);

  // AIModule* schwep_module = new AIModule();
  // schwep_module->setParent(schwep);
  // schwep->setAI(schwep_module);

  // AIModule* hamburger_module = new AIModule();
  // hamburger_module->setParent(hamburger);
  // hamburger->setAI(hamburger_module);

  // AIModule* swiss_cheese_module = new AIModule();
  // swiss_cheese_module->setParent(swiss_cheese);
  // swiss_cheese->setAI(swiss_cheese_module);

 // Inventory Testinggd
  Inventory* friends_pouch = new Inventory(401, "Teh Pouch");
  Inventory* foes_pouch = new Inventory(402, "Der Pouch");

  // Party Testing
  Party* friends = new Party(401, getPerson(300), PartyType::SLEUTH, 10,
      friends_pouch);
  friends->addMember(getPerson(301));
  // friends->addMember(berran);
  // friends->addMember(atkst);
  // friends->addMember(kevin);
  // friends->addMember(george);
  // friends->addMember(gregory);
  // friends->addMember(georgina);
  // friends->addMember(gerald);
  // friends->addMember(geoff);

  Party* enemies = new Party(402, getPerson(310), PartyType::REGULAR_FOE, 
      10, foes_pouch);
  // enemies->addMember(getPerson(311));
  // enemies->addMember(thruster_barrow);
  // enemies->addMember(dragon);
  // enemies->addMember(splurge);
  // enemies->addMember(schwep);
  // enemies->addMember(hamburger);
  // enemies->addMember(swiss_cheese);
  
  // Battle Testing
  for (uint32_t i = 0; i < friends->getSize(); i++)
    friends->getMember(i)->battlePrep();
  for (uint32_t i = 0; i < enemies->getSize(); i++)
    enemies->getMember(i)->battlePrep();

  game_battle = new Battle(game_config, friends, enemies);

  //malgidus->print(false, false, true, true);
  //frosty->print(false, false, true, true);

  // Begin Time Test
  //using namespace std::chrono;

  //system_clock::time_point tp = system_clock::now();
  //system_clock::duration dtn = tp.time_since_epoch();
  //std::cout << "Starting clock: " << dtn.count() << std::endl;

  //std::vector<std::string> the_strings(1000000, " abcd ");

  // for (auto &s : the_strings)
  // s = Helpers::trim(s);

  // Helpers Testing
  // auto length =  10;

  //std::vector<int> values(length, 0);

  //for (auto i = 0; i < length * 100; i++)
  //{
    //auto value = Helpers::randU32();
    //std::cout << value << std::endl;
    //values[value]++;
  //}
  //for (auto i = 0; i < length * 100000; i++)
  //{
    //auto value = Helpers::randU64();
    //std::cout << value << std::endl;
    //values[value]++;
  //}
  //std::cout << "DONE!" << std::endl;

  //system_clock::time_point tp2 = system_clock::now();
  //system_clock::duration dtn2 = tp2.time_since_epoch();
  //std::cout << "Ending clock: " << dtn2.count() << std::endl;
  //std::cout << "Periods elapsed: " << (dtn2.count() - dtn.count()) / 1000 << std::endl;

  // General Item Testing
  // Item* potion      = new Item(45, "Potion", 70, nullptr, 1.01);
  // potion->setFlag(ItemFlags::CONSUMED, true);
  // potion->setFlag(ItemFlags::DEFENSIVE, true);
  // potion->setFlag(ItemFlags::HEALING_ITEM, true);
  // potion->setOccasion(ActionOccasion::ALWAYS);

  // Item* bubby_bomb  = new Item(47, "Bubby Bomb", 85, nullptr, 2.62);
  // bubby_bomb->setFlag(ItemFlags::CONSUMED, true);
  // bubby_bomb->setFlag(ItemFlags::OFFENSIVE, true);
  // bubby_bomb->setOccasion(ActionOccasion::BATTLE);

  // //Item* unique_item = new Item(46, "Unique Item", 75, nullptr, 1.08);s
  // Item* new_potion     = new Item(potion);
  // Item* new_bubby_bomb = new Item(bubby_bomb);

  // Key Item Testing
  // Item* master_key  = new Item(115, "Master Key", nullptr);
  // Item* master_key2 = new Item(master_key);
  // Item* super_box = new Item(116, "Super Box", nullptr);
  // Item* boxxy_box = new Item(117, "Boxxy Box", nullptr);
  // Item* epic_sword_comp = new Item(118, "Epic Sword Comp", nullptr);
  // Item* alpha_omega = new Item(119, "Alpha Omega", nullptr);

  // Flavour Testing
  // Flavour* spark = new Flavour(101, "Spark", spark_set, 1.04, 35);
  // Flavour* tumor = new Flavour(102, "Tumor", tumor_set, 1.09, 65);
  // Flavour* moldy = new Flavour(103, "Moldy", moldy_set, 1.11, 48);

  // Bubby* first  = new Bubby(spark);
  // Bubby* second = new Bubby(spark, 1);
  // Bubby* third  = new Bubby(tumor,1);
  // Bubby* fourth = new Bubby(moldy);
  // Bubby* fifth = new Bubby(tumor,2);
  // Bubby* sixth = new Bubby(moldy,2);

  // Signature Testing

  // Equipment Testing
  //Equipment* fated = new Equipment(201, "Fated Oak Saber", 1, 1, 100, nullptr, 10, 10);
  //Equipment* suit  = new Equipment(202, "Suit", 100, 3, 100, nullptr, 10, 10);

  //test_pouch->add(new_potion, 10);
  // test_pouch->add(first, 6);
  // test_pouch->add(first, 3);
  // test_pouch->add(second, 3);
  // test_pouch->add(third, 2);
  // test_pouch->add(fourth, 3);
  // test_pouch->removeBubbyID(first->getGameID(), 2);
  // test_pouch->add(fifth, 3);
  // test_pouch->add(sixth, 4);
  // test_pouch->add(fated, 2);
  // test_pouch->add(suit, 1);

  // test_shoppy->add(new_potion, 110);
  // test_shoppy->add(first, 1012);
  // test_shoppy->add(second, 1140);
  // test_shoppy->add(first, 1410);
  // test_shoppy->add(first, 10116);
  // test_shoppy->add(fated, 6401);
  // test_shoppy->add(suit, 18);

  // test_shoppy->removeEquipID(suit->getGameID(), 18);
  // test_shoppy->removeEquipID(fated->getGameID(), 6401);

  //test_pouch->removeBubbyID(fifth->getGameID());
  //test_pouch->removeBubbyID(fifth->getGameID());
  //test_pouch->removeBubbyID(fifth->getGameID());

  //test_pouch->removeBubbyID(fifth->getGameID());
  //test_pouch->removeBubbyID(fifth->getGameID());
  //test_pouch->removeBubbyID(fifth->getGameID());

  //test_pouch->removeBubbyID(fifth->getGameID());
  //test_pouch->addBubby(fifth);

  //test_pouch->removeBubbyID(fifth->getGameID());
  //test_pouch->removeBubbyIndex(0);
  //test_pouch->removeBubbyIndex(0);
  //test_pouch->removeBubbyIndex(0);
  //test_pouch->removeBubbyIndex(0);
  //test_pouch->removeBubbyIndex(0);

  //test_pouch->addBubby(fifth);
  //test_pouch->addBubby(fifth);
  //test_pouch->addBubby(fifth, 1);

  //test_pouch->addEquipment(fated);
  //test_pouch->addEquipment(suit);
  //test_pouch->removeEquipID(fated->getGameID());
  //test_pouch->removeEquipID(suit->getGameID());

  // test_pouch->sort(SortType::NAME, SortObjects::ZERO_BUBBIES, false);
  // test_pouch->print(false);

  // auto key_items = test_pouch->getKeyItems();

  // for (auto it = begin(key_items); it != end(key_items); ++it)
  // {
  //  std::cout << "Item: " << (*it).first->getName() << " " 
  //           << static_cast<int>((*it).second) << "\n";
  // }

  // delete test_pouch;
  // delete spark;
  // delete moldy;
  // delete tumor

  // new_potion->setUseSkill(medium_attack);
  // new_bubby_bomb->setUseSkill(normal_attack);
}

/* Set up the map - old map needs to be deleted prior to calling */
void Game::setupMap()
{
  /* Create the map */
  game_map = new Map(game_config, &event_handler);
  mode = MAP;
  
//  /* Load the map - temporary location */
//  game_map->loadMap("maps/test_04");
}

/* Sets up the default player inventory */
void Game::setupPlayerInventory()
{
  /* Create the inventory */
  game_inventory = new Inventory(7, "Default Inventory", nullptr);

  /* General inventory setup */
  Inventory::setMoneyID(kMONEY_ITEM_ID);

  /* Add starting items to the Inventory */
  //TODO: Starting items [01-11-14]
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

/* Returns a pointer to a given action by index or by ID */
Action* Game::getAction(const int32_t &index, const bool& by_id)
{
  if (by_id)
  {
    for (auto it = begin(action_list); it != end(action_list); ++it)
      if ((*it)->getID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < action_list.size())
  {
    return action_list.at(index);
  }
 
  return nullptr;
}

/* Returns a pointer to a battle class by index or by ID */
Category* Game::getBattleClass(const int32_t &index, const bool &by_id)
{
  if (by_id)
  {
    for (auto it = begin(battle_class_list); it != end(battle_class_list); ++it)
      if ((*it)->getID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < battle_class_list.size())
  {
    return battle_class_list.at(index);
  }

  return nullptr;
}

/* Returns a pointer to a race category by index or by ID */
Category* Game::getCategory(const int32_t &index, const bool &by_id)
{
  if (by_id)
  {
    for (auto it = begin(race_list); it != end(race_list); ++it)
      if ((*it)->getID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < race_list.size())
  {
    return race_list.at(index);
  }

  return nullptr;
}

/* Returns a pointer to a flavour by index or by ID */
Flavour* Game::getFlavour(const int32_t &index, const bool &by_id)
{
  if (by_id)
  {
    for (auto it = begin(flavour_list); it != end(flavour_list); ++it)
      if ((*it)->getGameID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < flavour_list.size())
  {
    return flavour_list.at(index);
  }

  return nullptr;
}

/* Returns a pointer to a skill by index or by ID */
Skill* Game::getSkill(const int32_t &index, const bool &by_id)
{
  if (by_id)
  {
    for (auto it = begin(skill_list); it != end(skill_list); ++it)
      if ((*it)->getID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < skill_list.size())
  {
    return skill_list.at(index);
  }

  return nullptr;
}

/* Returns a pointer to a person by index or by ID */
Person* Game::getPerson(const int32_t &index, const bool &by_id)
{
  if (by_id)
  {
    for (auto it = begin(base_person_list); it != end(base_person_list); ++it)
      if ((*it)->getGameID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < base_person_list.size())
  {
    return base_person_list.at(index);
  }

  return nullptr;
}

/* Returns a pointer to a person by index or by ID */
Item* Game::getItem(const int32_t &index, const bool &by_id)
{
  if (by_id)
  {
    for (auto it = begin(base_item_list); it != end(base_item_list); ++it)
      if ((*it)->getGameID() == index)
        return (*it);
  }
  else if (static_cast<uint32_t>(index) < base_item_list.size())
  {
    return base_item_list.at(index);
  }

  return nullptr;
}

/* The key down events to be handled by the class */
bool Game::keyDownEvent(SDL_KeyboardEvent event)
{
  /* Exit the game, game has finished processing */
  if(mode == BATTLE && event.keysym.sym == SDLK_ESCAPE)
  {
    return true;
  }
  /* TESTING section - probably remove at end */
  /* Switch the view to the map */
  else if(event.keysym.sym == SDLK_F1)
  {
    mode = MAP;

    if (game_battle != nullptr)
    {
      delete game_battle;
      game_battle = nullptr; 
    }
  }
  /* Switch the view to the battle */
  else if(event.keysym.sym == SDLK_F2)
  {
    if (game_battle == nullptr)
      eventStartBattle();
  }
  else if (event.keysym.sym == SDLK_F3)
  {

  }
  else if(event.keysym.sym == SDLK_F5 && mode == MAP && game_map != nullptr)
  {
    game_map->reloadMap(active_renderer);
  }
  /* Show item store dialog in map */
  else if(event.keysym.sym == SDLK_5)
  {
    if (game_map != nullptr)
    {
      std::vector<Item*> items;
      items.push_back(base_item_list[0]);
      items.push_back(base_item_list[0]);
      std::vector<uint32_t> counts;
      counts.push_back(2);
      counts.push_back(3);
      std::vector<int32_t> cost_modifiers;
      cost_modifiers.push_back(0);
      cost_modifiers.push_back(10);
      
      game_map->initStore(ItemStore::BUY, items, counts, 
                          cost_modifiers, "Is Ttly Kevin's Store", false);
    }
  }
  /* Otherwise, send keys to the active view */
  else
  {
    if(mode == MAP)
      return game_map->keyDownEvent(event);
    else if(mode == BATTLE)
      return game_battle->keyDownEvent(event);
  }
  
  return false;
}

/* The key up events to be handled by the class */
void Game::keyUpEvent(SDL_KeyboardEvent event)
{
  if(mode == MAP)
    game_map->keyUpEvent(event);
}

/* Renders the title screen */
bool Game::render(SDL_Renderer* renderer)
{
  // TODO Create temporary list of items - Pull into file */
  // if(base_item_list.empty())
  // {
  //   Item* item1 = new Item(5, "Sword of Power", 125, 
  //                         new Frame("sprites/Map/_TEST/sword_AA_A00.png", renderer));
  //   Item* item2 = new Item(7, "Frost Bubby", 5, 
  //                          new Frame("sprites/Battle/Bubbies/frosty_t1.png", 
  //                                    renderer));
  //   Item* item3 = new Item(0, "Coins", 1, 
  //                          new Frame("sprites/Map/_TEST/coins_AA_A00.png", renderer));
    
  //   base_item_list.push_back(item1);
  //   base_item_list.push_back(item2);
  //   base_item_list.push_back(item3);
  // }
  
  /* Make sure the active renderer is set up */
  // TODO: Possibly revise. Change how the game handles maps and changing
  if(active_renderer == NULL)
    active_renderer = renderer;
  
  /* Map initialization location */
  if(!game_map->isLoaded())
   game_map->loadMap(base_path + "maps/test_06", renderer);

  if(mode == MAP)
    return game_map->render(renderer);
  
  return true;
}

/* Set the running configuration, from the options class */
bool Game::setConfiguration(Options* running_config)
{
  if(running_config != nullptr)
  {
    game_config = running_config;
    base_path   = game_config->getBasePath();
    
    /* Set in secondary classes */
    if(game_map != nullptr)
      game_map->setConfiguration(running_config);

    if (game_battle != nullptr)
      game_battle->setConfiguration(running_config);

    return true;
  }
  
  return false;
}

/* Updates the game state. Returns true if the class is finished */
bool Game::update(int32_t cycle_time)
{
  /* Poll System Events */
  pollEvents();

  if(mode == MAP && game_map != nullptr)
    return game_map->update(cycle_time);

  if(mode == BATTLE && game_battle != nullptr)
    return game_battle->update(cycle_time);

  return false;
}
