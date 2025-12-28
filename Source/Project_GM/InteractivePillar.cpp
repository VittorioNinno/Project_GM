#include "InteractivePillar.h"

AInteractivePillar::AInteractivePillar()
{
	PrimaryActorTick.bCanEverTick = false;
	PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	RootComponent = PillarMesh;
	PillarMesh->SetCanEverAffectNavigation(true);
}

void AInteractivePillar::OnStuntImpact_Implementation(AActor* Stigator, FName StuntType)
{
	if (bIsToppled || StuntType != RequiredStuntType) return;

	bIsToppled = true;
	PillarMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	PlayFallAnimation();
}