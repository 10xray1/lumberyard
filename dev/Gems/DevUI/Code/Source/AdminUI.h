/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once

#include <AzCore/Slice/SliceAsset.h>

#include <LyShine/IDraw2d.h>
#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiTransform2dBus.h>
#include <LyShine/Bus/UiImageBus.h>
#include <LyShine/Bus/UiButtonBus.h>

#include <DevUI/AdminUIBus.h>

namespace DevUI
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//! Class for demonstrating how to programmatically create a canvas from scratch in C++
	//! The created canvas shows a few examples of interactable elements (a button, a checkbox, and a
	//! textInput) as well as a very simple example of custom behavior (a small health system with a
	//! health bar that can be damaged / healed through two buttons).
	class AdminUI
		: protected AdminUIBus::Handler
		, public UiButtonNotificationBus::MultiHandler
	{
	public: // member functions

		AdminUI();
		~AdminUI();

		// AdminUIBus
		void CreateCanvas() override;
		void DestroyCanvas() override;
		// ~AdminUIBus

		// UiButtonNotificationBus
		void OnButtonClick() override;
		// ~UiButtonNotificationBus

		static void Reflect(AZ::ReflectContext* context);

	private: // member functions

		AZ_DISABLE_COPY_MOVE(AdminUI);

		//! Create the background image
		AZ::EntityId CreateBackground();

		//! Create elements from the ground up
		void CreateElementsExample(AZ::EntityId foregroundId);

		//! Create elements with programmatic behavior
		void CreateBehaviorExample(AZ::EntityId foregroundId);

		//! Creates a component
		void CreateComponent(AZ::Entity* entity, const AZ::Uuid& componentTypeId);

		//! Creates a button element
		AZ::EntityId CreateButton(const char* name, bool atRoot, AZ::EntityId parent,
			UiTransform2dInterface::Anchors anchors, UiTransform2dInterface::Offsets offsets,
			const char* text, AZ::Color baseColor, AZ::Color selectedColor, AZ::Color pressedColor,
			AZ::Color textColor, bool scaleToDevice);

		//! Creates a checkbox element
		AZ::EntityId CreateCheckbox(const char* name, bool atRoot, AZ::EntityId parent,
			UiTransform2dInterface::Anchors anchors, UiTransform2dInterface::Offsets offsets,
			const char* text, AZ::Color baseColor, AZ::Color selectedColor, AZ::Color pressedColor,
			AZ::Color checkColor, AZ::Color textColor, bool scaleToDevice);

		//! Creates a text element
		AZ::EntityId CreateText(const char* name, bool atRoot, AZ::EntityId parent,
			UiTransform2dInterface::Anchors anchors, UiTransform2dInterface::Offsets offsets,
			const char* text, AZ::Color textColor, IDraw2d::HAlign hAlign, IDraw2d::VAlign vAlign,
			bool scaleToDevice);

		//! Creates a text input element
		AZ::EntityId CreateTextInput(const char* name, bool atRoot, AZ::EntityId parent,
			UiTransform2dInterface::Anchors anchors, UiTransform2dInterface::Offsets offsets,
			const char* text, const char* placeHolderText,
			AZ::Color baseColor, AZ::Color selectedColor, AZ::Color pressedColor,
			AZ::Color textColor, AZ::Color placeHolderColor, bool scaleToDevice);

		//! Creates an image element
		AZ::EntityId CreateImage(const char* name, bool atRoot, AZ::EntityId parent,
			UiTransform2dInterface::Anchors anchors, UiTransform2dInterface::Offsets offsets,
			AZStd::string spritePath, UiImageInterface::ImageType imageType, AZ::Color color,
			bool scaleToDevice);

		//! Change the health by change amount and update the health bar
		void UpdateHealth(int change);

	private: // data

		AZ::EntityId m_canvasId;

		AZ::EntityId m_damageButton;
		AZ::EntityId m_healButton;
		AZ::EntityId m_destroyButton;

		AZ::EntityId m_healthBar;
		UiTransform2dInterface::Offsets m_maxHealthBarOffsets;
		int m_health;
	};
}