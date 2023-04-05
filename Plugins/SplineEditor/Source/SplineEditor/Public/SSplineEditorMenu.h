// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SPLINEEDITOR_API SSplineEditorMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSplineEditorMenu)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
};
