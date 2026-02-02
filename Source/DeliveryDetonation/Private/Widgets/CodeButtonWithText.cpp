// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CodeButtonWithText.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"


void UCodeButtonWithText::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackGroundButton)
	{
		BackGroundButton->OnClicked.AddDynamic(this, &UCodeButtonWithText::HandleClicked);
		BackGroundButton->OnHovered.AddDynamic(this, &UCodeButtonWithText::HandleHovered);
	}
}

void UCodeButtonWithText::HandleClicked()
{
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound, UISoundVolume);
	}
	OnClicked.Broadcast();
}

void UCodeButtonWithText::HandleHovered()
{
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, UISoundVolume);
	}
}

void UCodeButtonWithText::SetLabel(const FText& NewText)
{
	if (Information)
	{
		Information->SetText(NewText);
	}
}

void UCodeButtonWithText::Focus()
{
	if (BackGroundButton)
	{
		BackGroundButton->SetKeyboardFocus();
	}
}
