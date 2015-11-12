#include "Weapon_ShotGun.h"
#include "../Raven_Bot.h"
#include "../../Common/misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "../../Common/misc/utils.h"
#include "../../Common/fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
ShotGun::ShotGun(Raven_Bot*   owner):

                      Raven_Weapon(type_shotgun,
                                   script->GetInt("ShotGun_DefaultRounds"),
                                   script->GetInt("ShotGun_MaxRoundsCarried"),
                                   script->GetDouble("ShotGun_FiringFreq"),
                                   script->GetDouble("ShotGun_IdealRange"),
                                   script->GetDouble("Pellet_MaxSpeed"),
                                   owner),

            m_iNumBallsInShell(script->GetInt("ShotGun_NumBallsInShell")),
            m_dSpread(script->GetDouble("ShotGun_Spread"))
{

    //setup the vertex buffer
  const int NumWeaponVerts = 8;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, 0),
                                           Vector2D(0, -2),
                                           Vector2D(10, -2),
                                           Vector2D(10, 0),
                                           Vector2D(0, 0),
                                           Vector2D(0, 2),
                                           Vector2D(10, 2),
                                           Vector2D(10, 0)
                                           };

  
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
  InitializeFuzzyModule();

}

//------------------------------ ShootAt --------------------------------------

inline void ShotGun::ShootAt(Vector2D pos)
{ 
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //a shotgun cartridge contains lots of tiny metal balls called pellets. 
    //Therefore, every time the shotgun is discharged we have to calculate
    //the spread of the pellets and add one for each trajectory
    for (int b=0; b<m_iNumBallsInShell; ++b)
    {
      //determine deviation from target using a bell curve type distribution
      double deviation = RandInRange(0, m_dSpread) + RandInRange(0, m_dSpread) - m_dSpread;

      Vector2D AdjustedTarget = pos - m_pOwner->Pos();
 
      //rotate the target vector by the deviation
      Vec2DRotateAroundOrigin(AdjustedTarget, deviation);
 
      //add a pellet to the game world
      m_pOwner->GetWorld()->AddShotGunPellet(m_pOwner, AdjustedTarget + m_pOwner->Pos());

    }

    m_iNumRoundsLeft--;

    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("ShotGun_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
inline double ShotGun::GetDesirability(double DistToTarget)
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

//--------------------------- InitializeFuzzyModule ---------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void ShotGun::InitializeFuzzyModule()
{  
  FuzzyVariable& DistanceToTarget = m_FuzzyModule.CreateFLV("DistanceToTarget");

  FzSet& Target_Contact = DistanceToTarget.AddLeftShoulderSet("Target_Contact",0,25,50);
  FzSet& Target_Close = DistanceToTarget.AddTriangularSet("Target_Close",25,50,150);
  FzSet& Target_Medium =DistanceToTarget.AddTriangularSet("Target_Medium",50,150,300);
  FzSet& Target_Far = DistanceToTarget.AddTriangularSet("Target_Far",150,300,500);
  FzSet& Target_VeryFar = DistanceToTarget.AddRightShoulderSet("Target_VeryFar",300,500,1000);

  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
  
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 30, 60, 100);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 0, 30, 60);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 10, 30);
  FzSet& Ammo_VeryLow = AmmoStatus.AddLeftShoulderSet("Ammo_VeryLow", 0, 0, 10);

  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_Loads), FzFairly(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_Okay), FzFairly(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_Low), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Contact, Ammo_VeryLow), Desirable);

  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_VeryLow), FzFairly(VeryDesirable));

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), FzFairly(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), FzFairly(Undesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_VeryLow), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_VeryLow), FzVery(Undesirable));

  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Loads), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Okay), FzVery(Undesirable));
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Low), FzVery(Undesirable));
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_VeryLow), FzVery(Undesirable));
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void ShotGun::Render()
{
  m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->BrownPen();

  gdi->PolyLine(m_vecWeaponVBTrans);

}