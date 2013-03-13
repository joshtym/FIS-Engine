/******************************************************************************
* Class Name: Item
* Date Created: Oct 28 2012
* Inheritance: InventoryUnit
* Description: The Item Header that defines all elements within Inventory.
*              This includes Equipment and Bubby.
* TODO: Gets / Sets for stats [01-26-13]
******************************************************************************/
#ifndef ITEM_H
#define ITEM_H
#include <QImage>
#include <QString>

class Item
{
public:
  /* Constructor function */
  Item(QWidget *parent = 0);

  /* Destructor function */
  ~Item();

  /* Enumerated flags for item class */
  enum ItemState
  {
    BATTLEREADY   = 1ul << 0, /* Can the item be used in battle */
    MENUREADY     = 1ul << 1, /* Can the item be used in the menu? */
    CONSUMABLE    = 1ul << 2, /* (BATTLEREADY || MENUREADY) && !INDEFINITE */
    HEALITEM      = 1ul << 3, /* Does the item heal vitality? */
    CURE          = 1ul << 4, /* Does the item cure ailments? */
    OFFENSIVE     = 1ul << 5, /* Does the item have an offensive battle use? */
    INDEFINITE    = 1ul << 5, /* If item has infinite uses */
    STACKABLE     = 1ul << 6, /* If item can be grouped with others */
    EQUIPMENT     = 1ul << 7, /* Is the item a piece of equipment? */
    KEYITEM       = 1ul << 8, /* is the item a unique quest item? */
    MULTIITEM     = 1ul << 9, /* Does the item hit more than one target? */
    PARTYTITEM    = 1ul << 10 /* Does the item effect all members of a party? */
  };
  Q_DECLARE_FLAGS(ItemFlags, ItemState)
  ItemFlags iflag_set;

private:
  /* Category unit belongs to */
  QString item_category;

  /* Description of unit */
  QString description;

  /* Name of unit*/
  QString name;

  /* Thumbnail of unit*/
  QImage thumbnail;

  /* Number of turns the item will last in battle */
  uint duration;

  /* Value of currency in the shop */
  uint value;

public:
  /* Gets category of unit */
  QString getCategory();

  /* Gets description of unit */
  QString getDescription();

  /* Gets name of unit */
  QString getName();

  /* Gets thumbnail of unit */
  QImage getThumbnail();

  /* Gets the value of an ItemFlag */
  bool getItemFlag(ItemState flag);

  /* Gets the turn count */
  uint getDuration();

  /* Sets unit count */
  void setDuration(uint value);

  /* Sets an ItemFlag */
  void setItemFlag(ItemState flag, bool set_value);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Item::ItemFlags);

#endif // ITEM_H

