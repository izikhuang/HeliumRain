
#include "../Flare.h"
#include "FlareSoundManager.h"
#include "FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareOrbitalEngine.h"
#include "AudioDevice.h"


#define LOCTEXT_NAMESPACE "UFlareSoundManager"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSoundManager::UFlareSoundManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Gameplay data
	ShipPawn = NULL;
	MasterVolume = 1.0f;
	MusicVolume = 1.0f;
	EffectsVolume = 1.0f;
	MusicChanging = false;
	MusicDesiredTrack = EFlareMusicTrack::None;

	// Music track references
	static ConstructorHelpers::FObjectFinder<USoundCue> MenuMusicObj(TEXT("/Game/Master/Music/A_Menu_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> ExplorationMusicObj(TEXT("/Game/Master/Music/A_Exploration_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> TravelMusicObj(TEXT("/Game/Master/Music/A_Exploration_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> DangerMusicObk(TEXT("/Game/Master/Music/A_Danger_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> PacificMusicObj(TEXT("/Game/Master/Music/A_Exploration_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> CombatMusicObj(TEXT("/Game/Master/Music/A_Combat_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> WarMusicObj(TEXT("/Game/Master/Music/A_Combat_Cue"));

	// Mix references
	static ConstructorHelpers::FObjectFinder<USoundClass> MasterClassObj(TEXT("/Engine/EngineSounds/Master"));
	static ConstructorHelpers::FObjectFinder<USoundClass> MusicClassObj(TEXT("/Engine/EngineSounds/Music"));
	static ConstructorHelpers::FObjectFinder<USoundClass> EffectsClassObj(TEXT("/Engine/EngineSounds/SFX"));
	static ConstructorHelpers::FObjectFinder<USoundMix> MasterSoundMixObj(TEXT("/Game/Master/Sound/Mix_Master"));
	
	// Sound references
	static ConstructorHelpers::FObjectFinder<USoundCue> TargetWarningSoundObj(TEXT("/Game/Master/Sound/Sounds/A_TargetWarning"));
	static ConstructorHelpers::FObjectFinder<USoundCue> AttackWarningSoundObj(TEXT("/Game/Master/Sound/Sounds/A_AttackWarning"));
	static ConstructorHelpers::FObjectFinder<USoundCue> HealthWarningSoundObj(TEXT("/Game/Master/Sound/Sounds/A_HealthWarning"));
	static ConstructorHelpers::FObjectFinder<USoundCue> HealthWarningHeavySoundObj(TEXT("/Game/Master/Sound/Sounds/A_HealthWarningHeavy"));

	// Sound class
	MasterSoundClass = MasterClassObj.Object;
	MusicSoundClass = MusicClassObj.Object;
	EffectsSoundClass = EffectsClassObj.Object;
	MasterSoundMix = MasterSoundMixObj.Object;

	// Music track store
	MusicTracks.Add(NULL);
	MusicTracks.Add(MenuMusicObj.Object);
	MusicTracks.Add(ExplorationMusicObj.Object);
	MusicTracks.Add(TravelMusicObj.Object);
	MusicTracks.Add(PacificMusicObj.Object);
	MusicTracks.Add(DangerMusicObk.Object);
	MusicTracks.Add(CombatMusicObj.Object);
	MusicTracks.Add(WarMusicObj.Object);

	// Sound references
	TargetWarningSound = TargetWarningSoundObj.Object;
	AttackWarningSound = AttackWarningSoundObj.Object;
	HealthWarningSound = HealthWarningSoundObj.Object;
	HealthWarningHeavySound = HealthWarningHeavySoundObj.Object;
	
	// Music sound
	MusicPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("MusicSound"));
	MusicPlayer.Sound->bAutoActivate = false;
	MusicPlayer.Sound->bAutoDestroy = false;
	MusicPlayer.PitchedFade = false;
	MusicPlayer.FadeSpeed = 5.0;
	MusicPlayer.Volume = 0;
	
	// Power sound
	PowerPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("PowerSound"));
	PowerPlayer.Sound->bAutoActivate = false;
	PowerPlayer.Sound->bAutoDestroy = false;
	PowerPlayer.PitchedFade = true;
	PowerPlayer.FadeSpeed = 0.3;
	PowerPlayer.Volume = 0;

	// Engine sound
	EnginePlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineSound"));
	EnginePlayer.Sound->bAutoActivate = false;
	EnginePlayer.Sound->bAutoDestroy = false;
	EnginePlayer.PitchedFade = true;
	EnginePlayer.FadeSpeed = 2.0;
	EnginePlayer.Volume = 0;

	// RCS sound
	RCSPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("RCSSound"));
	RCSPlayer.Sound->bAutoActivate = false;
	RCSPlayer.Sound->bAutoDestroy = false;
	RCSPlayer.PitchedFade = true;
	RCSPlayer.FadeSpeed = 5.0;
	RCSPlayer.Volume = 0;

	// Warning sound on targeting
	TargetWarningPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("TargetWarningSound"));
	TargetWarningPlayer.Sound->bAutoActivate = false;
	TargetWarningPlayer.Sound->bAutoDestroy = false;
	TargetWarningPlayer.PitchedFade = false;
	TargetWarningPlayer.FadeSpeed = 10.0f;
	TargetWarningPlayer.Volume = 0;

	// Warning sound on attack
	AttackWarningPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AttackWarningSound"));
	AttackWarningPlayer.Sound->bAutoActivate = false;
	AttackWarningPlayer.Sound->bAutoDestroy = false;
	AttackWarningPlayer.PitchedFade = false;
	AttackWarningPlayer.FadeSpeed = 10.0f;
	AttackWarningPlayer.Volume = 0;

	// Warning sound on low health
	HealthWarningPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("HealthWarningSound"));
	HealthWarningPlayer.Sound->bAutoActivate = false;
	HealthWarningPlayer.Sound->bAutoDestroy = false;
	HealthWarningPlayer.PitchedFade = false;
	HealthWarningPlayer.FadeSpeed = 1.0f;
	HealthWarningPlayer.Volume = 0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSoundManager::Setup(AFlarePlayerController* Player)
{
	FLOG("UFlareSoundManager::Setup");
	PC = Player;

	// Attach all sounds
	if (Player)
	{
		USceneComponent* RootComponent = Player->GetRootComponent();
		FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);

		// Music
		MusicPlayer.Sound->AttachToComponent(RootComponent, AttachRules);
		MusicPlayer.Sound->SetSound(NULL);

		// Engine sounds
		PowerPlayer.Sound->AttachToComponent(RootComponent, AttachRules);
		EnginePlayer.Sound->AttachToComponent(RootComponent, AttachRules);
		RCSPlayer.Sound->AttachToComponent(RootComponent, AttachRules);

		// Warning sounds
		TargetWarningPlayer.Sound->AttachToComponent(RootComponent, AttachRules);
		AttackWarningPlayer.Sound->AttachToComponent(RootComponent, AttachRules);
		HealthWarningPlayer.Sound->AttachToComponent(RootComponent, AttachRules);
		TargetWarningPlayer.Sound->SetSound(TargetWarningSound);
		AttackWarningPlayer.Sound->SetSound(AttackWarningSound);
		HealthWarningPlayer.Sound->SetSound(HealthWarningSound);
		
		// Device
		FAudioDevice* AudioDevice = Player->GetWorld()->GetAudioDevice();
		FCHECK(AudioDevice);
		FCHECK(MasterSoundClass);
		FCHECK(MasterSoundMix);
		AudioDevice->SetBaseSoundMix(MasterSoundMix);
	}
}

void UFlareSoundManager::SetMusicVolume(int32 Volume)
{
	FLOGV("UFlareSoundManager::SetMusicVolume %d", Volume);

	MusicVolume = FMath::Clamp(Volume / 10.0f, 0.0f, 1.0f);

	// Update
	FAudioDevice* AudioDevice = PC->GetWorld()->GetAudioDevice();
	FCHECK(AudioDevice);
	AudioDevice->SetSoundMixClassOverride(MasterSoundMix, MusicSoundClass, MusicVolume, 1.0f, 0.5f, true);
}

void UFlareSoundManager::SetMasterVolume(int32 Volume)
{
	FLOGV("UFlareSoundManager::SetMasterVolume %d", Volume);

	MasterVolume = FMath::Clamp(Volume / 10.0f, 0.0f, 1.0f);

	// Update
	FAudioDevice* AudioDevice = PC->GetWorld()->GetAudioDevice();
	FCHECK(AudioDevice);
	AudioDevice->SetSoundMixClassOverride(MasterSoundMix, MasterSoundClass, MasterVolume, 1.0f, 0.5f, true);
}

void UFlareSoundManager::Update(float DeltaSeconds)
{
	// Update the ship if necessary
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
	if (PlayerShip != ShipPawn)
	{
		SetCurrentSpacecraft(PlayerShip);
	}

	// Music management
	if (MusicChanging)
	{
		if (MusicPlayer.Volume > 0)
		{
			UpdatePlayer(MusicPlayer, -DeltaSeconds);
		}
		else
		{
			SetDesiredMusicTrack();
		}
	}
	else if (MusicDesiredTrack == EFlareMusicTrack::None)
	{
		UpdatePlayer(MusicPlayer, -DeltaSeconds);
	}
	else
	{
		UpdatePlayer(MusicPlayer, +DeltaSeconds);
	}

	// Has a ship : update ship sounds
	if (ShipPawn)
	{
		// Engine values
		float RCSAlpha = 0;
		float EngineAlpha = 0;
		int32 RCSCount = 0;
		int32 EngineCount = 0;

		// Check all engines for engine alpha values
		TArray<UActorComponent*> Engines = ShipPawn->GetComponentsByClass(UFlareEngine::StaticClass());
		for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
		{
			UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
			if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
			{
				EngineAlpha += Engine->GetEffectiveAlpha();
				EngineCount++;
			}
			else
			{
				RCSAlpha += Engine->GetEffectiveAlpha();
				RCSCount++;
			}
		}

		// Get player threats
		bool Targeted, FiredUpon;
		UFlareSimulatedSpacecraft* Threat;
		PC->GetPlayerShipThreatStatus(Targeted, FiredUpon, Threat);
		UFlareSimulatedSpacecraftDamageSystem* DamageSystem = PlayerShip->GetParent()->GetDamageSystem();
		bool PlayerShipEndangered = (DamageSystem->IsCrewEndangered() || DamageSystem->IsUncontrollable()) && DamageSystem->IsAlive();
		
		// Update engine sounds
		UpdatePlayer(PowerPlayer,  (!DamageSystem->IsUncontrollable() && !DamageSystem->HasPowerOutage() ? 1 : -1) * DeltaSeconds);
		UpdatePlayer(EnginePlayer, (EngineAlpha > 0 ? EngineAlpha / EngineCount : -1) * DeltaSeconds);
		UpdatePlayer(RCSPlayer,    (RCSAlpha > 0 ? RCSAlpha / RCSCount : -1)          * DeltaSeconds);

		// Update alarms
		bool IsHeavy = (ShipPawn->GetParent()->GetDescription()->Size == EFlarePartSize::L);
		UpdatePlayer(TargetWarningPlayer, (!IsHeavy && !FiredUpon & Targeted ?  1.0f : -1.0f) * DeltaSeconds);
		UpdatePlayer(AttackWarningPlayer, (!IsHeavy &&  FiredUpon ?             1.0f : -1.0f) * DeltaSeconds);
		UpdatePlayer(HealthWarningPlayer, (PlayerShipEndangered && !FiredUpon ? 1.0f : -1.0f) * DeltaSeconds);
	}

	// No ship : stop all ship sounds
	else
	{
		UpdatePlayer(PowerPlayer,         -DeltaSeconds);
		UpdatePlayer(EnginePlayer,        -DeltaSeconds);
		UpdatePlayer(RCSPlayer,           -DeltaSeconds);
		UpdatePlayer(TargetWarningPlayer, -DeltaSeconds);
		UpdatePlayer(AttackWarningPlayer, -DeltaSeconds);
		UpdatePlayer(HealthWarningPlayer, -DeltaSeconds);
	}

	UpdateEffectsVolume(DeltaSeconds);
}

void UFlareSoundManager::RequestMusicTrack(EFlareMusicTrack::Type NewTrack)
{
	FLOGV("UFlareSoundManager::RequestMusicTrack : requested %d", (int32)(NewTrack - EFlareMusicTrack::None));
	if (NewTrack != MusicDesiredTrack)
	{
		FLOG("UFlareSoundManager::RequestMusicTrack : switching");
		MusicDesiredTrack = NewTrack;
		MusicChanging = true;
	}
}


/*----------------------------------------------------
	Internal methods
----------------------------------------------------*/

void UFlareSoundManager::SetCurrentSpacecraft(AFlareSpacecraft* Ship)
{
	ShipPawn = Ship;

	if (Ship)
	{
		// Setup power sound
		FFlareSpacecraftDescription* ShipDescription = Ship->GetParent()->GetDescription();
		PowerPlayer.Sound->SetSound(ShipDescription ? ShipDescription->PowerSound : NULL);

		// Setup orbital engine sound
		FFlareSpacecraftComponentDescription* EngineDescription = Ship->GetOrbitalEngineDescription();
		EnginePlayer.Sound->SetSound(EngineDescription ? EngineDescription->EngineCharacteristics.EngineSound : NULL);

		// Setup RCS sound
		FFlareSpacecraftComponentDescription* RCSDescription = Ship->GetRCSDescription();
		RCSPlayer.Sound->SetSound(RCSDescription ? RCSDescription->EngineCharacteristics.EngineSound : NULL);

		// Setup warning sound
		HealthWarningPlayer.Sound->Sound = ShipDescription->Size == EFlarePartSize::L ? HealthWarningHeavySound : HealthWarningSound;
	}
}

void UFlareSoundManager::SetDesiredMusicTrack()
{
	int32 TrackIndex = (int32)(MusicDesiredTrack - EFlareMusicTrack::None);
	FLOGV("UFlareSoundManager::SetDesiredMusicTrack : starting %d", TrackIndex);
	FCHECK(TrackIndex >= 0 && TrackIndex < MusicTracks.Num());

	MusicPlayer.Sound->SetSound(MusicTracks[TrackIndex]);
	MusicChanging = false;
}

void UFlareSoundManager::UpdateEffectsVolume(float DeltaSeconds)
{
	// Fade in menu transitions
	if (PC)
	{
		if (PC->GetMenuManager()->IsFading())
		{
			EffectsVolume -= DeltaSeconds / PC->GetMenuManager()->GetFadeDuration();
		}
		else
		{
			EffectsVolume += DeltaSeconds / PC->GetMenuManager()->GetFadeDuration();
		}
		EffectsVolume = FMath::Clamp(EffectsVolume, 0.01f, 1.0f);
	}

	// Get device and update
	FAudioDevice* AudioDevice = PC->GetWorld()->GetAudioDevice();
	FCHECK(AudioDevice);
	AudioDevice->SetSoundMixClassOverride(MasterSoundMix, EffectsSoundClass, EffectsVolume, 1.0f, 0.0f, true);
}

void UFlareSoundManager::UpdatePlayer(FFlareSoundPlayer& Player, float VolumeDelta)
{
	float NewVolume = FMath::Clamp(Player.Volume + VolumeDelta * Player.FadeSpeed, 0.0f, 1.0f);

	if (NewVolume != Player.Volume)
	{
		if (NewVolume == 0)
		{
			Player.Sound->Stop();
		}
		else if (Player.Volume == 0)
		{
			Player.Sound->Play();
		}
		else
		{
			Player.Sound->SetVolumeMultiplier(NewVolume);
			Player.Sound->SetPitchMultiplier(Player.PitchedFade ? 0.5f + 0.5f * NewVolume : 1.0f);
		}
		Player.Volume = NewVolume;
	}
}

void UFlareSoundManager::StopPlayer(FFlareSoundPlayer& Player)
{
	Player.Sound->Stop();
	Player.Volume = 0;
}

#undef LOCTEXT_NAMESPACE
