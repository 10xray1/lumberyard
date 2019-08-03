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

#include "DevUI_precompiled.h"

#include "InventoryUI.h"

#include <AzCore/Component/Entity.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <LyShine/ISprite.h>
#include <LyShine/UiSerializeHelpers.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <LyShine/Bus/UiImageBus.h>
#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiCheckboxBus.h>
#include <LyShine/Bus/UiSliderBus.h>
#include <LyShine/Bus/UiTextInputBus.h>
#include <LyShine/Bus/UiInteractableStatesBus.h>
#include <LyShine/Bus/UiInitializationBus.h>
#include <LyShine/UiComponentTypes.h>
#include <LyShine/Bus/UiAnimationBus.h>
#include <LyShine/Bus/UiNavigationBus.h>
#include <LyShine/Bus/UiCursorBus.h>

#include <Lyshine/Bus/UiLayoutGridBus.h>


/*
Background:
	Element
	transform2d
	image

	BackButton:
		Element
		Transform2d
		Image
		Button
		Tooltip
		LUA "unloadthiscanvasbutton.lua"

		Text:
			Element
			Transform2d
			Text

	Tooltip:
		Element
		Transform2d
		Image
		Tooltipdisplay

		Text:
			Element
			Transform2d
			Text

	ScreenContent:
		Element
		Transform2d

		Element:
			Element
			Transform2d

			Equiped:
				Element
				Transform2d
				Image
				LayoutGrid

				EquippedSlot:
					Element
					Transform2d
					Image
					DropTarget
					LUA "droptargetstacking.lua"

			Inventory:
				Element
				Transform2d
				Image
				LayoutGrid

				InventorySlot:
					Element
					Transform2d
					Image
					DropTarget
					LUA "droptargetstacking.lua"

					Draggable:
						Element
						Transform2d
						Image
						Draggable
						LUA "draggablestackingelement.lua"

						Image:
							Element
							Transform2d
							Image

							CounterBox:
								Element
								Transform2d
								Image

								CounterText:
									Element
									Transform2d
									Text
*/



namespace DevUI
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	InventoryUI::InventoryUI()
		: m_canvasBusHandler(nullptr)
		, m_rootCanvasId(0)
	{
		InventoryUIBus::Handler::BusConnect();
		AzFramework::GameEntityContextEventBus::Handler::BusConnect();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	InventoryUI::~InventoryUI()
	{
		AzFramework::GameEntityContextEventBus::Handler::BusDisconnect();
		InventoryUIBus::Handler::BusDisconnect();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void InventoryUI::CreateCanvas()
	{
		// Remove the existing example canvas if it exists
		//DestroyCanvas();

		m_rootCanvasId = gEnv->pLyShine->CreateCanvas();
		if (!m_rootCanvasId.IsValid())
		{
			AZ_TracePrintf("DevUI - Inventory", "Root canvas is invalid");
			return;
		}
		m_canvasBusHandler = UiCanvasBus::FindFirstHandler(m_rootCanvasId);

		// Show mouse
		bool isMouseShowing;
		UiCursorBus::BroadcastResult(isMouseShowing, &UiCursorInterface::IsUiCursorVisible);
		if (!isMouseShowing)
		{
			AZ_TracePrintf("DevUI - Inventory","Showing mouse")
			UiCursorBus::Broadcast(&UiCursorInterface::IncrementVisibleCounter);
		}

		int numOfSlots(10);

		// Create an image to be the canvas background
		AZ_TracePrintf("DevUI - Inventory", "Creating background")
		AZ::EntityId backgroundEntityId = CreateBackgroundEntity(m_rootCanvasId);
		AZ_TracePrintf("DevUI - Inventory", "Creating screenspace")
		AZ::EntityId screenSpaceEntityId = CreateScreenSpaceEntity(backgroundEntityId);
		AZ_TracePrintf("DevUI - Inventory", "Creating element")
		AZ::EntityId elementEntityId = CreateElementEntity(screenSpaceEntityId);

		AZ_TracePrintf("DevUI - Inventory", "Creating inventory")
		AZ::EntityId inventoryEntityId = CreateInventoryEntity(elementEntityId, numOfSlots);

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void InventoryUI::DestroyCanvas()
	{
		// Hide mouse
		bool isMouseShowing;
		UiCursorBus::BroadcastResult(isMouseShowing, &UiCursorInterface::IsUiCursorVisible);
		if (isMouseShowing)
		{
			UiCursorBus::Broadcast(&UiCursorInterface::DecrementVisibleCounter);
		}

		if (m_rootCanvasId.IsValid())
		{
			gEnv->pLyShine->ReleaseCanvas(m_rootCanvasId, false);
			m_rootCanvasId.SetInvalid();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void InventoryUI::Reflect(AZ::ReflectContext* context)
	{
		AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
		if (behaviorContext)
		{
			behaviorContext->EBus<InventoryUIBus>("InventoryUIRequestBus")
				->Event("CreateCanvas", &InventoryUIBus::Events::CreateCanvas)
				->Event("DestroyCanvas", &InventoryUIBus::Events::DestroyCanvas);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIVATE MEMBER FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void InventoryUI::OnGameEntitiesStarted()
	{
		AZ_TracePrintf("DevUI - InventoryUI", "Level loaded, creating canvas");
		CreateCanvas();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	AZ::EntityId InventoryUI::CreateBackgroundEntity(AZ::EntityId rootCanvasId)
	{
		AZ::Entity* background = m_canvasBusHandler->CreateChildElement("Background");

		CreateComponent(background, LyShine::UiTransform2dComponentUuid);
		CreateComponent(background, LyShine::UiImageComponentUuid);

		AZ::EntityId backgroundId = background->GetId();
		EBUS_EVENT_ID(backgroundId, UiTransform2dBus, SetAnchors, UiTransform2dInterface::Anchors(0.0f, 0.0f, 1.0f, 1.0f), false, false);
		EBUS_EVENT_ID(backgroundId, UiImageBus, SetColor, AZ::Color(0.f, 0.f, 0.f, 0.7f));

		return backgroundId;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	AZ::EntityId InventoryUI::CreateScreenSpaceEntity(AZ::EntityId backgroundId)
	{
		AZ::Entity* screenSpace = m_canvasBusHandler->CreateChildElement("ScreenSpace");

		CreateComponent(screenSpace, LyShine::UiTransform2dComponentUuid);
		CreateComponent(screenSpace, LyShine::UiImageComponentUuid);

		AZ::EntityId screenSpaceId = screenSpace->GetId();
		EBUS_EVENT_ID(screenSpaceId, UiTransform2dBus, SetAnchors, UiTransform2dInterface::Anchors(0.1f, 0.1f, 0.9f, 0.9f), false, false);
		EBUS_EVENT_ID(screenSpaceId, UiImageBus, SetColor, AZ::Color(1.f, 0.f, 0.f, 0.75f));

		return screenSpaceId;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	AZ::EntityId InventoryUI::CreateElementEntity(AZ::EntityId screenSpaceId)
	{
		AZ::Entity* element = m_canvasBusHandler->CreateChildElement("Element");

		CreateComponent(element, LyShine::UiTransform2dComponentUuid);
		CreateComponent(element, LyShine::UiImageComponentUuid);

		AZ::EntityId elementId = element->GetId();
		EBUS_EVENT_ID(elementId, UiTransform2dBus, SetAnchors, UiTransform2dInterface::Anchors(0.2f, 0.2f, 0.8f, 0.8f), false, false);
		EBUS_EVENT_ID(elementId, UiImageBus, SetColor, AZ::Color(0.f, 1.f, 0.f, 0.75f));

		return elementId;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	AZ::EntityId InventoryUI::CreateInventoryEntity(AZ::EntityId elementEntityId, int numOfSlots)
	{
		AZ::Entity* inventory = m_canvasBusHandler->CreateChildElement("Inventory");

		CreateComponent(inventory, LyShine::UiTransform2dComponentUuid);
		CreateComponent(inventory, LyShine::UiImageComponentUuid);
		CreateComponent(inventory, LyShine::UiLayoutGridComponentUuid);

		AZ::EntityId inventoryId = inventory->GetId();
		EBUS_EVENT_ID(inventoryId, UiTransform2dBus, SetAnchors, UiTransform2dInterface::Anchors(0.3f, 0.3f, 0.7f, 0.7f), false, false);
		EBUS_EVENT_ID(inventoryId, UiImageBus, SetColor, AZ::Color(0.f, 0.f, 1.f, 0.75f));
		EBUS_EVENT_ID(inventoryId, UiLayoutGridBus, SetHorizontalOrder, UiLayoutInterface::HorizontalOrder::LeftToRight);
		EBUS_EVENT_ID(inventoryId, UiLayoutGridBus, SetVerticalOrder, UiLayoutInterface::VerticalOrder::TopToBottom);
		EBUS_EVENT_ID(inventoryId, UiLayoutGridBus, SetStartingDirection, UiLayoutGridInterface::StartingDirection::HorizontalOrder);

		return inventoryId;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void InventoryUI::CreateComponent(AZ::Entity* entity, const AZ::Uuid& componentTypeId)
	{
		entity->Deactivate();
		entity->CreateComponent(componentTypeId);
		entity->Activate();
	}
}
