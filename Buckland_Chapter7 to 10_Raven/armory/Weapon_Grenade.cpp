#include "Weapon_Grenade.h"
#include "../Raven_Bot.h"
#include "../../Common/misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "../../Common/fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
Grenade::Grenade(Raven_Bot*   owner):

                      Raven_Weapon(type_grenade,
                                   script->GetInt("Grenade_DefaultRounds"),
                                   script->GetInt("Grenade_MaxRoundsCarried"),
                                   script->GetDouble("Grenade_FiringFreq"),
                                   script->GetDouble("Grenade_IdealRange"),
                                   script->GetDouble("Nade_MaxSpeed"),
                                   owner)
{
  //setup the vertex buffer
  const int NumWeaponVerts = 4;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -1),
                                           Vector2D(10, -1),
                                           Vector2D(10, 1),
                                           Vector2D(0, 1)
                                           };

  
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
  InitializeFuzzyModule();
}


//------------------------------ ShootAt --------------------------------------

inline void Grenade::ShootAt(Vector2D pos)
{ 
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire a round
    m_pOwner->GetWorld()->AddNade(m_pOwner, pos);

    UpdateTimeWeaponIsNextAvailable();

    m_iNumRoundsLeft--;

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("Grenade_SoundRange"));
  }
}



//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double Grenade::GetDesirability(double DistToTarget)
{
  //fuzzify distance and amount of ammo

 if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  } else {
		m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
		m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

		m_dLastDesirabilityScore =m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
 }
  

  return m_dLastDesirabilityScore;
}

//----------------------- InitializeFuzzyModule -------------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void Grenade::InitializeFuzzyModule()
{
  FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

  FzSet& Target_Contact = DistToTarget.AddLeftShoulderSet("Target_Contact",0,25,50);
  FzSet& Target_Close = DistToTarget.AddTriangularSet("Target_Close",25,50,150);
  FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium",50,150,200);
  FzSet& Target_Far = DistToTarget.AddTriangularSet("Target_Far",150,200,300);
  FzSet& Target_VeryFar = DistToTarget.AddRightShoulderSet("Target_VeryFar",200,300,1000);

  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 4, 5, 6);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 2, 4, 5);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 2, 4);
  FzSet& Ammo_VeryLow = AmmoStatus.AddTriangularSet("Ammo_VeryLow", 0, 0, 2);

  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability"); 
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

  m_FuzzyModule.AddRule(Target_Contact, FzVery(Undesirable));
  m_FuzzyModule.AddRule(Target_Close, Desirable);
  m_FuzzyModule.AddRule(Target_Medium, FzFairly(VeryDesirable));
  m_FuzzyModule.AddRule(Target_Far, FzFairly(Undesirable));
  m_FuzzyModule.AddRule(Target_VeryFar, FzVery(Undesirable));

  m_FuzzyModule.AddRule(Ammo_Loads, Desirable);
  m_FuzzyModule.AddRule(Ammo_Okay, FzFairly(Undesirable));
  m_FuzzyModule.AddRule(Ammo_Low, Undesirable);
  m_FuzzyModule.AddRule(Ammo_VeryLow, Undesirable);

}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Grenade::Render()
{
   m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->DarkGreenPen();

  gdi->ClosedShape(m_vecWeaponVBTrans);
}