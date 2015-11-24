#include "Weapon_RailGun.h"
#include "../Raven_Bot.h"
#include "../../Common/misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "../../Common/fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
RailGun::RailGun(Raven_Bot*   owner):

                      Raven_Weapon(type_rail_gun,
                                   script->GetInt("RailGun_DefaultRounds"),
                                   script->GetInt("RailGun_MaxRoundsCarried"),
                                   script->GetDouble("RailGun_FiringFreq"),
                                   script->GetDouble("RailGun_IdealRange"),
                                   script->GetDouble("Slug_MaxSpeed"),
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

inline void RailGun::ShootAt(Vector2D pos)
{ 
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire a round
    m_pOwner->GetWorld()->AddRailGunSlug(m_pOwner, pos);

    UpdateTimeWeaponIsNextAvailable();

    m_iNumRoundsLeft--;

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("RailGun_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double RailGun::GetDesirability(double DistToTarget)
{
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
    //fuzzify distance and amount of ammo
    m_FuzzyModule.Fuzzify("DistanceToTarget", DistToTarget);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

  return m_dLastDesirabilityScore;
}

//----------------------- InitializeFuzzyModule -------------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void RailGun::InitializeFuzzyModule()
{ 

  FuzzyVariable& DistanceToTarget = m_FuzzyModule.CreateFLV("DistanceToTarget");
  
  FzSet& Target_Contact = DistanceToTarget.AddLeftShoulderSet("Target_Contact",0,25,50);
  FzSet& Target_Close = DistanceToTarget.AddTriangularSet("Target_Close",25,50,150);
  FzSet& Target_Medium = DistanceToTarget.AddTriangularSet("Target_Medium",50,150,200);
  FzSet& Target_Far = DistanceToTarget.AddTriangularSet("Target_Far",150,200,300);
  FzSet& Target_VeryFar = DistanceToTarget.AddRightShoulderSet("Target_VeryFar",200,300,1000);

  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
  
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 15, 40, 100);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 5, 20, 40);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 5, 20);
  FzSet& Ammo_VeryLow = AmmoStatus.AddLeftShoulderSet("Ammo_VeryLow", 0, 0, 5);

  
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_Loads), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_Okay),  Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_Low), FzVery(Undesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_VeryLow), FzVery(Undesirable));

  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), FzFairly(Desirable));
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay),  FzFairly(Desirable));
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_VeryLow), FzVery(Undesirable));

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_VeryLow), FzFairly(Desirable));

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_VeryLow), Desirable);

  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Loads), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Okay), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Low), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_VeryLow), VeryDesirable);

}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void RailGun::Render()
{
    m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->BluePen();

  gdi->ClosedShape(m_vecWeaponVBTrans);
}