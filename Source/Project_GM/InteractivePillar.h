#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StuntInteractable.h"
#include "InteractivePillar.generated.h"

UCLASS()
class PROJECT_GM_API AInteractivePillar : public AActor, public IStuntInteractable
{
	GENERATED_BODY()
	
public:	
	AInteractivePillar();

	virtual void OnStuntImpact_Implementation(AActor* Stigator, FName StuntType) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PillarMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName RequiredStuntType = FName("Dash");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bIsToppled = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void PlayFallAnimation();
};