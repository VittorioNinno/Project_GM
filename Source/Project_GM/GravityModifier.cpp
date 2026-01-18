// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityModifier.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AGravityModifier::AGravityModifier()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GravityScale = 1;
	WalkingSpeedScale = 1;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillBoard"));
	Billboard->SetupAttachment(RootComponent);
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	Box->SetupAttachment(RootComponent);
	
	static ConstructorHelpers::FObjectFinder<UTexture2D> PositiveTex(TEXT("/Game/Static/LevelPrototyping/Textures/GravityIcons/GravityIconDown"));
	if (PositiveTex.Succeeded())
		PositiveGravityTexture = PositiveTex.Object;
	
	static ConstructorHelpers::FObjectFinder<UTexture2D> NegativeTex(TEXT("/Game/Static/LevelPrototyping/Textures/GravityIcons/GravityIconUp"));
	if (NegativeTex.Succeeded())
		NegativeGravityTexture = NegativeTex.Object;
	
	static ConstructorHelpers::FObjectFinder<UTexture2D> ZeroTex(TEXT("/Game/Static/LevelPrototyping/Textures/GravityIcons/GravityIconZero"));
	if (ZeroTex.Succeeded())
		ZeroGravityTexture = ZeroTex.Object;
}

void AGravityModifier::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (GravityScale > 0.0f)
	{
		if (PositiveGravityTexture)
		 Billboard->SetSprite(PositiveGravityTexture);
	}
	else if (GravityScale < 0.0f)
	{
		if (NegativeGravityTexture)
		 Billboard->SetSprite(NegativeGravityTexture);
	}
	else
	{
		if(ZeroGravityTexture)
		 Billboard->SetSprite(ZeroGravityTexture);
	}
}

// Called when the game starts or when spawned
void AGravityModifier::BeginPlay()
{
	Super::BeginPlay();
	
	if (!Box) return;
	
	Box->OnComponentBeginOverlap.AddDynamic(this, &AGravityModifier::OnBoxBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &AGravityModifier::OnBoxEndOverlap);
}

// Called every frame
/*void AGravityModifier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

void AGravityModifier::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement) return;
	DefaultGravityScale = Movement->GravityScale;
	DefaultWalkableFloorZ = Movement->GetWalkableFloorZ();
	
	Movement->GravityScale = GravityScale;
	
	Movement->MaxWalkSpeed = Movement->MaxWalkSpeed - WalkingSpeedScale * GravityScale;
	Movement->MaxWalkSpeedCrouched = Movement->MaxWalkSpeedCrouched - WalkingSpeedScale * GravityScale;
	if (GravityScale < 0.0f)
	{
		Movement->SetWalkableFloorZ(WalkableFloorZ);
	}
}

void AGravityModifier::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this) return;
	
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement) return;
	
	Movement->GravityScale = DefaultGravityScale;
	Movement->SetWalkableFloorZ(DefaultWalkableFloorZ);
	Movement->MaxWalkSpeed = Movement->MaxWalkSpeed + WalkingSpeedScale * GravityScale;
	Movement->MaxWalkSpeedCrouched = Movement->MaxWalkSpeedCrouched + WalkingSpeedScale * GravityScale;
}

