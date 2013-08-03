/******************************************************************************
* Class Name: Player
* Date Created: December 2nd, 2012
* Inheritance: none
* Description: Abstract player class, containing the two parties -- sleuth and
*              bearacks, and also represents the player's position and speed
*              on the map
******************************************************************************/
#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui/QWidget>

#include "Game/Player/Party.h"

class Player
{
public:
  /* Player constructor object */
  Player(Party* p_sleth = 0, Party* p_racks = 0, int x = 0, int y = 0);

  /* Annihilates a player object */
  ~Player(); 

private:
  /* Tick speed for the maps */
  int kMAPSPEED;

  /* Current party (Sleuth) object, and base party (Bearacks) */
  Party* sleuth;
  Party* bearacks;

  /* Your position on the current map */
  int x_pos;
  int y_pos;

  /* Carry weight */
  double carry_weight;
  double gravity;

  /* Money */
  uint32_t credits;

  /* ------------ Constants --------------- */
  static const uint kMAX_CREDITS;

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/
private:
  /* Calculates the carry weight of the player */
  void calcCarryWeight();

/*============================================================================
 * PUBLIC FUNCTIONS
 w*===========================================================================*/
public:
  /* Adds an amount of money to credits */
  void addCredits(unsigned long value);

  /* Methods for printing ou the info of the player */
  void printInfo();

  /* Removes a sleuth member by index by calling Party's remove function */
  bool removeSleuthMember(int index);

  /* Removes a sleuth member by name by calling Party's remove function */
  bool removeSleuthMember(QString value);

  /* Removes a bearacks member by index by calling Party's remove function */
  bool removeRacksMember(int index);

  /* Removes a bearacks member by name by calling Party's remove function */
  bool removeRacksMember(QString value);

  /* Returns the amount of credits (money) the player has */
  unsigned long getCredits();

  /* Gets # of frames in the main persons sprite multiplied by kMAPSPEED */
  int getSpeed(); 

  /* Gets the bearacks party */
  Party* getBearacks();

  /* Returns the carry weight */
  double getCarryWeight();

  /* Returns the gravity */
  double getGravity();

  /* Gets the sleuth party */
  Party* getSleuth();

  /* Gets the x-position on the map */
  int getXPos(); 

  /* Gets the y-position on the map */
  int getYPos();

  /* Sets the amount of money the player has */
  void setCredits(unsigned long value);

  /* Sets the sleuth party */
  void setSleuth(Party* p = 0);

  /* Sets the bearcks party */
  void setBearacks(Party* p = 0);

  /* Sets the gravity the player is experiencing */
  void setGravity(double new_value);

  /* Sets the x-position on the map */
  void setXPos(int new_x_pos);

  /* Sets the y-position on the map */
  void setYPos(int new_y_pos);
};

#endif // PLAYER_H
