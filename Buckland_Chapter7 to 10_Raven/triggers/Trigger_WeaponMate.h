#ifndef WEAPON_MATE_H
#define WEAPON_MATE_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_WeaponGiver.h
//
//  Author:   Mat Buckland
//
//  Desc:     This trigger 'gives' the triggering bot a weapon of the
//            specified type 
//
//-----------------------------------------------------------------------------
#include "../../Common/Triggers/Trigger_Respawning.h"
#include "../Raven_Bot.h"
#include <iosfwd>




class Trigger_WeaponMate : public Trigger_Respawning<Raven_Bot>
{
private:

  //vrtex buffers for rocket shape
  std::vector<Vector2D>         m_vecRLVB;
  std::vector<Vector2D>         m_vecRLVBTrans;
  bool							equipe;
  
public:

  //this type of trigger is created when reading a map file
  Trigger_WeaponMate(bool _equipe);
  //void WeaponMate(bool equipe,int projectileNade,int projectilePellet, int projectileRocket, int projectileSlug);
  //if triggered, this trigger will call the PickupWeapon method of the
  //bot. PickupWeapon will instantiate a weapon of the appropriate type.
  void Try(Raven_Bot*);
  
  //draws a symbol representing the weapon type at the trigger's location
  void Render();

  void Read ();
};




#endif