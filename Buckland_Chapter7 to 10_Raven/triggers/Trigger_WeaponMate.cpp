#include "Trigger_WeaponMate.h"
#include "../../Common/misc/Cgdi.h"
#include "../../Common/misc/Stream_Utility_Functions.h"
#include <fstream>
#include "../lua/Raven_Scriptor.h"
#include "../constants.h"
#include "../Raven_ObjectEnumerations.h"
#include "../Raven_WeaponSystem.h"


///////////////////////////////////////////////////////////////////////////////

Trigger_WeaponMate::Trigger_WeaponMate(std::ifstream& datafile):
      
          Trigger_Respawning<Raven_Bot>(GetValueFromStream<int>(datafile))
{
  Read(datafile);

  //create the vertex buffer for the rocket shape
  const int NumRocketVerts = 8;
  const Vector2D rip[NumRocketVerts] = {Vector2D(0, 3),
                                       Vector2D(1, 2),
                                       Vector2D(1, 0),
                                       Vector2D(2, -2),
                                       Vector2D(-2, -2),
                                       Vector2D(-1, 0),
                                       Vector2D(-1, 2),
                                       Vector2D(0, 3)};
  
  for (int i=0; i<NumRocketVerts; ++i)
  {
    m_vecRLVB.push_back(rip[i]);
  }
}


void Trigger_WeaponMate::Try(Raven_Bot* pBot)
{
  if (this->isActive() && this->isTouchingTrigger(pBot->Pos(), pBot->BRadius()))
  {
    pBot->GetWeaponSys()->AddWeapon(EntityType());

    Deactivate();
  } 
}




void Trigger_WeaponMate::Read(std::ifstream& in)
{
  double x, y, r;
  int GraphNodeIndex;
  

  in >>  x >> y  >> r >> GraphNodeIndex;

  SetPos(Vector2D(x,y)); 
  SetBRadius(r);
  SetGraphNodeIndex(GraphNodeIndex);

  //create this trigger's region of fluence
  AddCircularTriggerRegion(Pos(), script->GetDouble("DefaultGiverTriggerRange"));


  SetRespawnDelay((unsigned int)(script->GetDouble("Weapon_RespawnDelay") * FrameRate));
}


/*void Trigger_WeaponMate::WeaponMate(bool equipe,int projectileNade,int projectilePellet, int projectileRocket, int projectileSlug)
{
	Vector2D posSpawn=Vector2D(0,0);
	if (equipe){
		posSpawn=Vector2D(0,0);
	}
	int r=7;
	if (projectileNade>1){
		SetPos(posSpawn);
		SetBRadius(r);
	}
	if (projectileRocket>14){
		SetPos(posSpawn);
		SetBRadius(r);
	}
	if (projectilePellet>14){
		SetPos(posSpawn);
		SetBRadius(r);
	}
	if (projectileSlug>14){
		SetPos(posSpawn);
		SetBRadius(r);
	}


  //SetPos(Vector2D(x,y)); 
  //SetBRadius(r);

  //create this trigger's region of fluence
  AddCircularTriggerRegion(Pos(), script->GetDouble("DefaultGiverTriggerRange"));

}*/


void Trigger_WeaponMate::Render()
{
  if (isActive())
  {
    switch (EntityType())
    {
      case type_rail_gun:
        {
          gdi->BluePen();
          gdi->BlueBrush();
          gdi->Circle(Pos(), 3);
          gdi->ThickBluePen();
          gdi->Line(Pos(), Vector2D(Pos().x, Pos().y-9));
        }

        break;

      case type_shotgun:
        {
          gdi->BlackBrush();
          gdi->BrownPen();
          const double sz = 3.0;
          gdi->Circle(Pos().x-sz,Pos().y, sz);
          gdi->Circle(Pos().x+sz,Pos().y, sz);
        }

        break;

		case type_grenade:
        {
		  gdi->GreenBrush();
          gdi->BrownPen();
          const double sz = 3.0;
          gdi->Circle(Pos().x,Pos().y+2.5, sz);
          gdi->Circle(Pos().x,Pos().y-2, 1.5);
         
        }

        break;
      case type_rocket_launcher:
        {

           Vector2D facing(-1,0);

           m_vecRLVBTrans = WorldTransform(m_vecRLVB,
                                          Pos(),
                                          facing,
                                          facing.Perp(),
                                          Vector2D(2.5,2.5));

            gdi->RedPen();
            gdi->ClosedShape(m_vecRLVBTrans);
        }
      
        break;

    }//end switch
  }
}