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

#include <DevUI/InventoryUIBus.h>

#include <AzFramework/Entity/GameEntityContextBus.h>

namespace DevUI
{
	class InventoryUI
		: protected InventoryUIBus::Handler
		, public UiButtonNotificationBus::MultiHandler
		, public AzFramework::GameEntityContextEventBus::Handler
	{
	public: // member functions

		InventoryUI();
		~InventoryUI();

		// InventoryUIBus
		void CreateCanvas() override;
		void DestroyCanvas() override;
		// ~InventoryUIBus

		static void Reflect(AZ::ReflectContext* context);

	private: // member functions
		
		// GameEntityContextEventBus
		void OnGameEntitiesStarted() override;
		// ~GameEntityContextEventBus

		AZ_DISABLE_COPY_MOVE(InventoryUI);

		//! Create the background image
		AZ::EntityId CreateBackgroundEntity(AZ::EntityId);
		AZ::EntityId CreateScreenSpaceEntity(AZ::EntityId);
		AZ::EntityId CreateElementEntity(AZ::EntityId);
		AZ::EntityId CreateInventoryEntity(AZ::EntityId, int);

		//! Creates a component
		void CreateComponent(AZ::Entity* entity, const AZ::Uuid& componentTypeId);

	private:
		AZ::EntityId m_rootCanvasId;
		UiCanvasInterface* m_canvasBusHandler;
	};
}
