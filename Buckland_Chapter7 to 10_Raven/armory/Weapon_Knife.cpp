#include "Weapon_Knife.h"
#include "../Raven_Bot.h"
#include "../../Common/misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "../../Common/fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
Knife::Knife(Raven_Bot*   owner):

                      Raven_Weapon(type_knife,
                                   script->GetInt("Knife_DefaultRounds"),
                                   script->GetInt("Knife_MaxRoundsCarried"),
                                   script->GetDouble("Knife_FiringFreq"),
                                   script->GetDouble("Knife_IdealRange"),
                                   script->GetDouble("Knife_MaxSpeed"),
                                   owner)
{
  //setup the vertex buffer
  const int NumWeaponVerts = 4;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -1),
                                           Vector2D(20, -1),
                                           Vector2D(20, 1),
                                           Vector2D(0, 1)
                                           };

  
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

}


//------------------------------ ShootAt --------------------------------------

inline void Knife::ShootAt(Vector2D pos)
{ 
  if (isReadyForNextShot())
  {
    //fire!
    m_pOwner->GetWorld()->AddKnife(m_pOwner, pos);

    UpdateTimeWeaponIsNextAvailable();

  }
}



//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double Knife::GetDesirability(double DistToTarget)
{
  //fuzzify distance and amount of ammo
	m_dLastDesirabilityScore=0;
	if (DistToTarget<40){
		m_dLastDesirabilityScore=100;
	}
  return m_dLastDesirabilityScore;
}

//----------------------- InitializeFuzzyModule -------------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Knife::Render()
{
   m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->BlackPen();

  gdi->ClosedShape(m_vecWeaponVBTrans);
}
