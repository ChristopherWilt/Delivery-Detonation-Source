// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeButtonWithText.generated.h"

class UButton;
class UTextBlock;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCodeButtonClicked);
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UCodeButtonWithText : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Button")
	FOnCodeButtonClicked OnClicked;

	UFUNCTION(BlueprintCallable)
	void SetLabel(const FText& NewText);

	UFUNCTION(BlueprintCallable)
	void Focus();

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* BackGroundButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Information;

	// --- Audio ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* ClickSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* HoverSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	float UISoundVolume = 1.0f;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleHovered();
};
