// Fill out your copyright notice in the Description page of Project Settings.


#include "ElectrifiedZone.h"

#include "Components/CapsuleComponent.h"
#include "Effects/ElectricEffect.h"
#include "GameFramework/Character.h"

// Sets default values
AElectrifiedZone::AElectrifiedZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	//Box = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	//Box->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AElectrifiedZone::BeginPlay()
{
	Super::BeginPlay();
	UActorComponent* ActorComponent = GetComponentByClass(UShapeComponent::StaticClass());
	if (UShapeComponent* CapsuleComponent = Cast<UShapeComponent>(ActorComponent))
	{
		CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AElectrifiedZone::OnBoxBeginOverlap);
		CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &AElectrifiedZone::OnBoxEndOverlap);
	}
}

// Called every frame
void AElectrifiedZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!OverlappedCharacter) return;
	
	if (current_Time_Before_Starting_Effect_Again <= current_Effect_Time)
	{
		current_Effect_Time += DeltaTime;
		if (current_Effect_Time >= Effect_Time_Duration)
		{
			current_Effect_Time = 0;
			RemovedElectrifiedEffect(OverlappedCharacter);
		}
	}
	
	current_Time_Before_Starting_Effect_Again += DeltaTime;
	if (current_Time_Before_Starting_Effect_Again >= Effect_Time_Duration + Time_Before_Starting_Effect_Again)
	{
		current_Time_Before_Starting_Effect_Again = 0;
		ApplyElectrifiedEffect(OverlappedCharacter, Effect_Time_Duration);
	}
}

void AElectrifiedZone::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Electrified Zone Begin Overlap"));
	OverlappedCharacter = Cast<ACharacter>(OtherActor);
	ApplyElectrifiedEffect(OverlappedCharacter, Effect_Time_Duration);
}

void AElectrifiedZone::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this) return;
	
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Electrified Zone End Overlap"));
	OverlappedCharacter = nullptr;
	current_Effect_Time = 0;
	current_Time_Before_Starting_Effect_Again = 0;
}

void AElectrifiedZone::ApplyElectrifiedEffect(ACharacter* Character, float duration)
{
	if (!Character) return;
	
	UElectricEffect* ExistingComp = Character->FindComponentByClass<UElectricEffect>();

	if (ExistingComp)
	{
		ExistingComp->RefreshDuration(duration);
	}
	else
	{
		UElectricEffect* NewComp = NewObject<UElectricEffect>(Character, UElectricEffect::StaticClass());
		if (NewComp)
		{
			NewComp->InitializeEffect(duration);
			
#if WITH_EDITOR
			Character->AddInstanceComponent(NewComp);
#endif
			
			NewComp->RegisterComponent();
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Zone Applied Electricity Effect"));
}

void AElectrifiedZone::RemovedElectrifiedEffect(ACharacter* Character)
{
	UElectricEffect* ExistingComp = Character->FindComponentByClass<UElectricEffect>();
	if (!ExistingComp) return;
	ExistingComp->DestroyComponent();
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Removed Electrified Effect"));
}



