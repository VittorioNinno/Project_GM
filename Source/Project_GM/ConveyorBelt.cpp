#include "ConveyorBelt.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"

AConveyorBelt::AConveyorBelt()
{
	PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
    MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

    OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
    OverlapComp->SetupAttachment(MeshComp);
    OverlapComp->SetBoxExtent(FVector(50.f, 50.f, 10.f)); 
    OverlapComp->SetRelativeLocation(FVector(0.f, 0.f, 55.f));
    OverlapComp->SetCollisionProfileName(TEXT("Trigger"));

    DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
    DirectionArrow->SetupAttachment(OverlapComp);
    DirectionArrow->ArrowSize = 2.0f;

    PushSpeed = 600.0f;
}

void AConveyorBelt::BeginPlay()
{
	Super::BeginPlay();
	
    OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AConveyorBelt::OnOverlapBegin);
    OverlapComp->OnComponentEndOverlap.AddDynamic(this, &AConveyorBelt::OnOverlapEnd);
}

void AConveyorBelt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (OverlappingActors.Num() == 0)
        return;

    FVector PushVector = GetActorForwardVector() * PushSpeed * DeltaTime;

    for (int32 i = OverlappingActors.Num() - 1; i >= 0; i--)
    {
        AActor* Actor = OverlappingActors[i];

        if (Actor && Actor->IsValidLowLevel())
        {
            ACharacter* Character = Cast<ACharacter>(Actor);
            if (Character)
            {
                Character->AddActorWorldOffset(PushVector, true);
            }
            else
            {
                UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (RootPrim && RootPrim->IsSimulatingPhysics())
                {
                    RootPrim->AddForce(GetActorForwardVector() * PushSpeed * 100.0f);
                }
            }
        }
        else
        {
            OverlappingActors.RemoveAt(i);
        }
    }
}

void AConveyorBelt::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this) && !OverlappingActors.Contains(OtherActor))
    {
        OverlappingActors.Add(OtherActor);
    }
}

void AConveyorBelt::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        OverlappingActors.Remove(OtherActor);
    }
}