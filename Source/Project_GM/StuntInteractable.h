#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StuntInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UStuntInteractable : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_GM_API IStuntInteractable
{
	GENERATED_BODY()

public:
	/** Triggered when a character stunt impacts this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Environment|Interaction")
	void OnStuntImpact(AActor* Stigator, FName StuntType);
};