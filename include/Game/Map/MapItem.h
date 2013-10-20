/******************************************************************************
* Class Name: MapItem
* Date Created: Oct 28 2012
* Inheritance: MapThing
* Description: The MapItem class - TODO: description
******************************************************************************/
#ifndef MAPITEM_H
#define MAPITEM_H

#include "Game/Map/MapThing.h"

class MapItem : public MapThing
{
public:
  /* Constructor functions */
  MapItem();
  MapItem(Sprite* frames, int width, int height, QString name = "",
          QString description = "", int id = kUNSET_ID);

  /* Destructor function */
  virtual ~MapItem();
  
private:
  /* Status if the item is being brightened. Otherwise, darken it */
  bool brighter;
  
  /* The number of items available */
  int count;

  /* -------------------------- Constants ------------------------- */
  const static short kDEFAULT_COUNT; /* Default number of items on setup */
  const static float kMAX_BRIGHTNESS; /* The max brightness setting */
  const static float kMIN_BRIGHTNESS; /* The min brightness setting */

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Returns the class descriptor, useful for casting */
  virtual QString classDescriptor();

  /* Clears out the item construct, void of painting */
  void clear();
  
  /* Returns the count of how many of these items are available */
  int getCount();
  
  /* Sets the number of this item */
  bool setCount(int count);
  
  /* Set the tile to hook the map item to */
  bool setStartingTile(int section_id, Tile* new_tile, bool no_events = false);
  
  /* Updates the thing, based on the tick */
  void updateThing(float cycle_time, Tile* next_tile);
};

#endif // MAPITEM_H
