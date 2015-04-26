/*******************************************************************************
 * Class Name: MapNPC
 * Date Created: Oct 28 2012
 * Inheritance: MapPerson
 * Description: The MapNPC class, this covers all AI found on the map in game
 *
 * TODO: Reimplement the movement functions based on NPC scripts
 ******************************************************************************/
#ifndef MAPNPC_H
#define MAPNPC_H

#include "Game/Map/MapPerson.h"

/* Path node struct for moving */
struct Path
{
  uint16_t x;
  uint16_t y;
  uint16_t delay;
  bool xy_flip;
  Path* previous;
  Path* next;
};

class MapNPC : public MapPerson
{
public:
  /* Constructor function */
  MapNPC();
  MapNPC(int id, std::string name = "", std::string description = "");
  
  /* Destructor function */
  virtual ~MapNPC();

  /* LOOPED - Goes from node 1->2->3->1->2...
   * BACKANDFORTH - Goes from node 1->2->3->2->1->2...
   * RANDOMRANGE - Can move anywhere within the defined rect range (2 nodes)
   * RANDOM - Can move anywhere within the map
   * LOCKED - Halts movement once the next node is reached */
  enum NodeState{LOOPED, BACKANDFORTH, RANDOMRANGE, RANDOM, LOCKED};
 
  /* AVOIDPLAYER - if player comes near, the NPC will try and stay away
   * TOPLAYER - if the player comes near, the NPC will try and reach it
   * NOTRACK - maintains normal movement (path or random) */
  enum TrackingState{AVOIDPLAYER, TOPLAYER, NOTRACK};
  
  /* -------------------------- Constants ------------------------- */
  const static uint16_t kMAX_DELAY; /* The max delay for random delay calcs */
  const static uint16_t kMAX_RANGE; /* THe max range for random x,y move */
  
private:
  /* Does the NPC force interaction if possible? */
  bool forced_interaction;
  
  /* Is the NPC currently going forward or backward - used for BACKANDFORTH */
  bool moving_forward;
  
  /* The nodes for control */
  Path* node_current;
  Path* node_head;
  Path node_random;
  Path node_start;

  /* The state of movement */
  NodeState node_state;
  
  /* Should the nodes be deleted on destruction - relevant if base npc */
  bool nodes_delete;
  
  /* The active delay of the NPC on the node, if relevant */
  int npc_delay;
 
  /* Is this the first run of the path move? */
  bool starting;

  /* The state of tracking */
  TrackingState tracking_state;
  
/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/
private:
  /* Appends an empty node onto the back of the movement stack */
  void appendEmptyNode();
  
  /* Returns if the XY movement is flipped for the direction to move */
  bool getXYFlip();
  
  /* Returns the node pointer at the index */
  Path* getNode(uint16_t index);
 
  /* Initializes class - called on first construction */
  void initializeClass();

  /* Inserts a node, at the given index */
  bool insertNode(uint16_t index, Path* node);
  
  /* Takes the path node and randomizes the x, y, delay, and flip */
  void randomizeNode();
  
/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Adds npc information from the XML file. Will be virtually re-called
   * by all children for proper operation */
  virtual bool addThingInformation(XmlData data, int file_index, 
                                   int section_index, SDL_Renderer* renderer, 
                                   std::string base_path = "");

  /* Returns the class descriptor, useful for casting */
  virtual std::string classDescriptor();

  /* Clears out the NPC construct, void of painting */
  void clear();

  /* Path node handling functions */
  bool insertNode(uint16_t index, uint16_t x, uint16_t y, uint16_t delay = 0);
  bool insertNodeAtTail(uint16_t x, uint16_t y, uint16_t delay = 0);
  
  /* Returns the node movement state - how it traverses */
  NodeState getNodeState();
  
  /* Returns the number of nodes in the NPC path */
  uint16_t getPathLength();

  /* Returns the predicted move request in the class */
  Direction getPredictedMoveRequest();
  
  /* Returns the tracking state - how the NPC reacts to others */
  TrackingState getTrackingState();
  
  /* Returns if the NPC will force interaction */
  bool isForcedInteraction();
  
  /* Path nodes removal handling */
  bool removeAllNodes();
  bool removeNode(uint16_t index);
  bool removeNodeAtTail();
  
  /* Sets the base class */
  virtual bool setBase(MapThing* base);
  
  /* Sets the forced interaction of the npc */
  void setForcedInteraction(bool forced);
  
  /* Sets the node movement state - how it traverses */
  void setNodeState(NodeState state);
  
  /* Sets the starting x and y coordinate */
  void setStartingLocation(uint16_t section_id, uint16_t x, uint16_t y);

  /* Sets the tracking state - how the NPC reacts */
  void setTrackingState(TrackingState state);
  
  /* Updates the thing, based on the tick - reimplemented */
  void update(int cycle_time, std::vector<std::vector<Tile*>> tile_set);
  
/*=============================================================================
 * PUBLIC STATIC FUNCTIONS
 *============================================================================*/
public:
  /* Returns the node state string */
  static std::string getNodeString(NodeState state);
  
  /* Returns the tracking state string */
  static std::string getTrackingString(TrackingState state);
};

#endif // MAPNPC_H
