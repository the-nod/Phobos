#include "FogOfWar.h"

// ;; loading
// 6B8E7A = ScenarioClass_LoadSpecialFlags, 6
// 686C03 = SetScenarioFlags_FogOfWar, 5

// ;; other bug fixes
// ;//457A10 = BuildingClass_IsFogged,5
// 5F4B3E = ObjectClass_DrawIfVisible, 6
// ;//6FA2B7 = TechnoClass_Update_DrawHidden_CheckFog, 6
// 6F5190 = TechnoClass_DrawExtras_CheckFog, 6
// ;//6924C0 = DisplayClass_ProcessClickCoords_SetFogged, 7
// ;//4D1C9B = FoggedObjectClass_DrawAll_SelectColorScheme, 6
// ;//4D2158 = FoggedObjectClass_DrawAll_SelectAnimPal, 6
// ;//4D129F = FoggedObjectClass_CTOR_Building_SetRecordAnimAdjust, 6
// ;//4D19A6 = FoggedObjectClass_DrawAll_DrawRecords, 6
// 48049E = CellClass_DrawTileAndSmudge_CheckFog, 6
// 6D6EDA = TacticalClass_Overlay_CheckFog1, A
// 6D70BC = TacticalClass_Overlay_CheckFog2, A
// 71CC8C = TerrainClass_DrawIfVisible, 6
// ;//4D1714 = FoggedObjectClass_DTOR, 6
// 5865E2 = IsLocationFogged, 5

/* Hook information from Xkein
;;optimize
;//4ACD5A = MapClass_TryFogCell_SetFlag, 7
;//6D871C = TacticalClass_GetOcclusion_Optimize, 8
;//47BD4A = CellClass_CTOR_InitMore, 6

;;network
;//4C800C = Networking_RespondToEvent_20, 5
*/

DEFINE_HOOK(0x6B8E7A, ScenarioClass_LoadSpecialFlags, 0x6)
{
	GET(ScenarioClass*, pScenario, ESI);

	pScenario->SpecialFlags.FogOfWar = true;
		//RulesClass::Instance->FogOfWar || R->EAX() || GameModeOptionsClass::Instance->FogOfWar;
	
	R->ECX(pScenario);
	return 0x6B8E8B;
}

DEFINE_HOOK(0x686C03, SetScenarioFlags_FogOfWar, 0x5)
{
	GET(ScenarioFlags, SFlags, EAX);

	SFlags.FogOfWar = true;// RulesClass::Instance->FogOfWar || GameModeOptionsClass::Instance->FogOfWar;
	R->EDX<int>(*reinterpret_cast<int*>(&SFlags)); // stupid!

	return 0x686C0E;
}

DEFINE_HOOK(0x5F4B3E, ObjectClass_DrawIfVisible, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	
	enum { Skip = 0x5F4B7F, SkipWithDrawn = 0x5F4D06, DefaultProcess = 0x5F4B48 };

	if (pObject->InLimbo)
		return Skip;

	if(!ScenarioClass::Instance->SpecialFlags.FogOfWar)
		return DefaultProcess;
	
	if(pObject->WhatAmI()== AbstractType::Cell)
		return DefaultProcess;

	auto coord = pObject->GetCoords();
	if (!FogOfWar::IsLocationFogged(&coord))
		return DefaultProcess;

	pObject->NeedsRedraw = false;
	return SkipWithDrawn;
}

DEFINE_HOOK(0x6F5190, TechnoClass_DrawExtras_CheckFog, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);

	auto coord = pTechno->GetCoords();

	return FogOfWar::IsLocationFogged(&coord) ? 0x6F5EEC : 0;
}

DEFINE_HOOK(0x48049E, CellClass_DrawTileAndSmudge_CheckFog, 0x6)
{
	GET(CellClass*, pCell, ESI);

	if (pCell->SmudgeTypeIndex == -1 || pCell->IsFogged())
		return 0x4804FB;
	return 0x4804A4;
}

DEFINE_HOOK(0x6D6EDA, TacticalClass_Overlay_CheckFog1, 0xA)
{
	GET(CellClass*, pCell, EAX);

	if (pCell->OverlayTypeIndex == -1 || pCell->IsFogged())
		return 0x6D7006;
	return 0x6D6EE4;
}

DEFINE_HOOK(0x6D70BC, TacticalClass_Overlay_CheckFog2, 0xA)
{
	GET(CellClass*, pCell, EAX);

	if (pCell->OverlayTypeIndex == -1 || pCell->IsFogged())
		return 0x6D71A4;
	return 0x6D70C6;
}

DEFINE_HOOK(0x71CC8C, TerrainClass_DrawIfVisible, 0x6)
{
	GET(TerrainClass*, pTerrain, EDI);

	auto coord = pTerrain->GetCoords();
	if (pTerrain->InLimbo || FogOfWar::IsLocationFogged(&coord))
		return 0x71CD8D;
	return 0x71CC9A;
}

DEFINE_HOOK(0x5865E2, IsLocationFogged, 0x5)
{
	GET_STACK(CoordStruct*, pCoord, 0x4);

	R->EAX(FogOfWar::IsLocationFogged(pCoord));
	
	return 0;
}