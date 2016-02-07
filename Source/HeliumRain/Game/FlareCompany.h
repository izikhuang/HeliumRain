
#pragma once

#include "Object.h"
#include "FlareFleet.h"
#include "FlareGameTypes.h"
#include "FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "FlareCompany.generated.h"


class UFlareFleet;
class UFlareCompanyAI;

/** Hostility status */
UENUM()
namespace EFlareHostility
{
	enum Type
	{
		Hostile,
		Neutral,
		Friendly,
		Owned
	};
}

/** Catalog data */
USTRUCT()
struct FFlareCompanyDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Company)
	FText Name;

	/** Short name */
	UPROPERTY(EditAnywhere, Category = Company)
	FName ShortName;

	/** Emblem */
	UPROPERTY(EditAnywhere, Category = Company)
	UTexture2D* Emblem;

	/** Base color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationBasePaintColorIndex;
	
	/** Paint color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPaintColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationOverlayColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationLightColorIndex;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

};


class AFlareGame;
class UFlareSimulatedSpacecraft;

UCLASS()
class HELIUMRAIN_API UFlareCompany : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the company from a save file */
	virtual void Load(const FFlareCompanySave& Data);

	/** Post Load to perform task needing sectors to be loaded */
	virtual void PostLoad();

	/** Save the company to a save file */
	virtual FFlareCompanySave* Save();

	/** Spawn a simulated spacecraft from save data */
	virtual UFlareSimulatedSpacecraft* LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData);

	/** Load a fleet from save */
	virtual UFlareFleet* LoadFleet(const FFlareFleetSave& FleetData);

	/** Load a trade route from save */
	virtual UFlareTradeRoute* LoadTradeRoute(const FFlareTradeRouteSave& TradeRouteData);


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void SimulateAI();

	/** Check if we are friend or for toward the player */
	virtual EFlareHostility::Type GetPlayerHostility() const;

	/** Check if we are friend or foe toward this target company */
	virtual EFlareHostility::Type GetHostility(const UFlareCompany* TargetCompany) const;

	/** Set whether this company is hostile to an other company */
	virtual void SetHostilityTo(const UFlareCompany* TargetCompany, bool Hostile);

	/** Get an info string for this company */
	virtual FText GetShortInfoText();

	/** Create a new fleet in a sector */
	virtual UFlareFleet* CreateFleet(FText FleetName, UFlareSimulatedSector* FleetSector);

	/** Destroy a fleet */
	virtual void RemoveFleet(UFlareFleet* Fleet);

	/** Create a new trade route */
	virtual UFlareTradeRoute* CreateTradeRoute(FText TradeRouteName);

	/** Destroy a trade route */
	virtual void RemoveTradeRoute(UFlareTradeRoute* TradeRoute);


	/** Destroy a spacecraft */
	virtual void DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft);

	/** Set a sector discovered */
	virtual void DiscoverSector(UFlareSimulatedSector* Sector);

	/** Set a sector visited */
	virtual void VisitSector(UFlareSimulatedSector* Sector);

	/** Take a money amount from the company */
	virtual void TakeMoney(uint64 Amount);

	/** Give a money amount to the company */
	virtual void GiveMoney(uint64 Amount);

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/
	
	/** Update all ships and stations */
	virtual void UpdateCompanyCustomization();

	/** Apply customization to a component's material */
	virtual void CustomizeComponentMaterial(UMaterialInstanceDynamic* Mat);

	/** Apply customization to a component's special effect material */
	virtual void CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat);
	
	/** Normalize a color */
	FLinearColor NormalizeColor(FLinearColor Col) const;

	/** Setup the emblem for this company, using the company colors */
	void SetupEmblem();

	/** Get the company emblem */
	const FSlateBrush* GetEmblem() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	const FFlareCompanyDescription*         CompanyDescription;
	FFlareCompanySave                       CompanyData;

	UPROPERTY()
	UFlareCompanyAI*                         CompanyAI;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyStations;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyShips;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanySpacecrafts;

	UPROPERTY()
	TArray<UFlareFleet*>                    CompanyFleets;

	UPROPERTY()
	TArray<UFlareTradeRoute*>               CompanyTradeRoutes;

	UPROPERTY()
	UMaterialInstanceDynamic*               CompanyEmblem;

	UPROPERTY()
	FSlateBrush                             CompanyEmblemBrush;

	AFlareGame*                             Game;
	TArray<UFlareSimulatedSector*>          KnownSectors;
	TArray<UFlareSimulatedSector*>          VisitedSectors;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline FName GetIdentifier() const
	{
		return CompanyData.Identifier;
	}

	inline const FFlareCompanyDescription* GetDescription() const
	{
		return CompanyDescription;
	}

	inline FText GetCompanyName() const
	{
		check(CompanyDescription);
		return CompanyDescription->Name;
	}

	inline FName GetShortName() const
	{
		check(CompanyDescription);
		return CompanyDescription->ShortName;
	}

	inline int32 GetBasePaintColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationBasePaintColorIndex;
	}

	inline int32 GetPaintColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationPaintColorIndex;
	}

	inline int32 GetOverlayColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationOverlayColorIndex;
	}

	inline int32 GetLightColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationLightColorIndex;
	}

	inline int32 GetPatternIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationPatternIndex;
	}

	inline uint64 GetMoney() const
	{
		return CompanyData.Money;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetCompanyStations()
	{
		return CompanyStations;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetCompanyShips()
	{
		return CompanyShips;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetCompanySpacecrafts()
	{
		return CompanySpacecrafts;
	}

	inline TArray<UFlareFleet*>& GetCompanyFleets()
	{
		return CompanyFleets;
	}

	inline TArray<UFlareTradeRoute*>& GetCompanyTradeRoutes()
	{
		return CompanyTradeRoutes;
	}

	inline TArray<UFlareSimulatedSector*>& GetKnownSectors()
	{
		return KnownSectors;
	}

	inline TArray<UFlareSimulatedSector*>& GetVisitedSectors()
	{
		return VisitedSectors;
	}

	UFlareFleet* FindFleet(FName Identifier) const
	{
		for (int i = 0; i < CompanyFleets.Num(); i++)
		{
			if (CompanyFleets[i]->GetIdentifier() == Identifier)
			{
				return CompanyFleets[i];
			}
		}
		return NULL;
	}

	UFlareTradeRoute* FindTradeRoute(FName Identifier) const
	{
		for (int i = 0; i < CompanyTradeRoutes.Num(); i++)
		{
			if (CompanyTradeRoutes[i]->GetIdentifier() == Identifier)
			{
				return CompanyTradeRoutes[i];
			}
		}
		return NULL;
	}

	UFlareSimulatedSpacecraft* FindSpacecraft(FName ShipImmatriculation);

	bool HasVisitedSector(const UFlareSimulatedSector* Sector) const;

};