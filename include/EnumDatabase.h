/******************************************************************************
* Class Name: EnumDatabase
* Date Created: January 30th, 2013
* Inheritance:
* Description:  EnumDatabase holds non-flag Enums and QFlag include for
*               classes which use flags.
 *****************************************************************************/
#ifndef ENUMDATABASE_H
#define ENUMDATABASE_H

#include <QFlag>

enum ApplicationMode {TITLESCREEN, GAME};
enum Direction {NORTH,EAST,SOUTH,WEST};

enum Infliction
{
  POISON, BURN, SCALD, CHAR, BERSERK, CONFUSE, SILENCE, BUBBIFY, DEATHTIMER,
  PARALYSIS, BLINDNESS, DREADSTRUCK, DREAMSNARE, HELLBOUND, BOND,
  ALLATKBUFF, ALLDEFBUFF, PHYATKBUFF, PHYDEFBUFF, THRATKBUFF, THRDEFBUFF,
  POLATKBUFF, POLDEFBUFF, PRIATKBUFF, PRIDEFBUFF, CHGATKBUFF, CHGDEFBUFF,
  CYBATKBUFF, CYBDEFBUFF, NIHATKBUFF, NIHDEFBUFF, LIMBUFF, UNBBUFF, MOMBUFF,
  VITBUFF, QDBUFF, ROOTBOUND, DOUBLECAST, TRIPLECAST, HALFCOST
};

#endif // ENUMDATABASE_H
