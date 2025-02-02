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

#include "LmbrCentral_precompiled.h"
#include "EditorTubeShapeComponent.h"

#include "EditorSplineComponent.h"
#include "EditorSplineComponentMode.h"
#include "EditorTubeShapeComponentMode.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <AzCore/Component/Component.h>
#include <MathConversion.h>
#include "ShapeDisplay.h"

namespace LmbrCentral
{
    void EditorTubeShapeComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorTubeShapeComponent, EditorBaseShapeComponent>()
                ->Version(2)
                ->Field("TubeShape", &EditorTubeShapeComponent::m_tubeShape)
                ->Field("TubeShapeMeshConfig", &EditorTubeShapeComponent::m_tubeShapeMeshConfig)
                ->Field("ComponentMode", &EditorTubeShapeComponent::m_componentModeDelegate)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorTubeShapeComponent>(
                    "Tube Shape", "The Tube Shape component creates a spline around the associated entity")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Shape")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Tube_Shape.png")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Viewport/Tube_Shape.png")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://docs.aws.amazon.com/lumberyard/latest/userguide/component-shapes.html")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorTubeShapeComponent::m_tubeShape, "TubeShape", "Tube Shape Configuration")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorTubeShapeComponent::ConfigurationChanged)
                        //->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly) // disabled - prevents ChangeNotify attribute firing correctly
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorTubeShapeComponent::m_componentModeDelegate, "Component Mode", "Tube Component Mode")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ;
            }
        }
    }

    void EditorTubeShapeComponent::Init()
    {
        EditorBaseShapeComponent::Init();

        SetShapeComponentConfig(&m_tubeShapeMeshConfig.m_shapeComponentConfig);
    }

    void EditorTubeShapeComponent::Activate()
    {
        EditorBaseShapeComponent::Activate();
        SplineComponentNotificationBus::Handler::BusConnect(GetEntityId());
        SplineAttributeNotificationBus::Handler::BusConnect(GetEntityId());
        m_tubeShape.Activate(GetEntityId());
        AzFramework::EntityDebugDisplayEventBus::Handler::BusConnect(GetEntityId());
        EditorTubeShapeComponentRequestBus::Handler::BusConnect(GetEntityId());

        // connect the ComponentMode delegate to this entity/component id pair
        m_componentModeDelegate.Connect<EditorTubeShapeComponent>(AZ::EntityComponentIdPair(GetEntityId(), GetId()), this);
        // setup the ComponentMode(s) to add for the editing of this Component (in this case Spline and Tube ComponentModes)
        m_componentModeDelegate.SetAddComponentModeCallback([this](const AZ::EntityComponentIdPair& entityComponentIdPair)
        {
            using namespace AzToolsFramework::ComponentModeFramework;

            // builder for TubeComponentMode
            const auto tubeComponentModeBuilder =
                CreateComponentModeBuilder<EditorTubeShapeComponent, EditorTubeShapeComponentMode>(
                    entityComponentIdPair);

            // must have a Spline to have a Tube - create builder for Spline as well
            const auto splineComponentId = GetEntity()->FindComponent<EditorSplineComponent>()->GetId();
            const auto splineComponentModeBuilder =
                CreateComponentModeBuilder<EditorSplineComponent, EditorSplineComponentMode>(
                    AZ::EntityComponentIdPair(GetEntityId(), splineComponentId));

            // aggregate builders
            const auto entityAndComponentModeBuilder =
                EntityAndComponentModeBuilders(
                    GetEntityId(), { tubeComponentModeBuilder, splineComponentModeBuilder });

            // updates modes to add when entering ComponentMode
            ComponentModeSystemRequestBus::Broadcast(
                &ComponentModeSystemRequests::AddComponentModes, entityAndComponentModeBuilder);
        });

        GenerateVertices();
    }

    void EditorTubeShapeComponent::Deactivate()
    {
        m_componentModeDelegate.Disconnect();

        EditorTubeShapeComponentRequestBus::Handler::BusDisconnect();
        AzFramework::EntityDebugDisplayEventBus::Handler::BusDisconnect();
        m_tubeShape.Deactivate();
        SplineAttributeNotificationBus::Handler::BusDisconnect();
        SplineComponentNotificationBus::Handler::BusDisconnect();
        EditorBaseShapeComponent::Deactivate();
    }

    void EditorTubeShapeComponent::GenerateVertices()
    {
        // guard against the case where for some reason we may not have a Spline
        if (!m_tubeShape.GetSpline())
        {
            AZ_Error("EditorTubeShapeComponent", false, "A TubeShape must have a Spline to work");
            return;
        }

        const AZ::u32 endSegments = m_tubeShape.GetSpline()->IsClosed() ? 0 : m_tubeShapeMeshConfig.m_endSegments;
        GenerateTubeMesh(
            m_tubeShape.GetSpline(), m_tubeShape.GetRadiusAttribute(), m_tubeShape.GetRadius(),
            endSegments, m_tubeShapeMeshConfig.m_sides, m_tubeShapeMesh.m_vertexBuffer,
            m_tubeShapeMesh.m_indexBuffer, m_tubeShapeMesh.m_lineBuffer);
    }

    void EditorTubeShapeComponent::DisplayEntityViewport(
        const AzFramework::ViewportInfo& viewportInfo,
        AzFramework::DebugDisplayRequests& debugDisplay)
    {
        DisplayShape(
            debugDisplay, [this]() { return CanDraw(); },
            [this](AzFramework::DebugDisplayRequests& debugDisplay)
            {
                DrawShape(debugDisplay, { m_shapeColor, m_shapeWireColor, m_displayFilled }, m_tubeShapeMesh);
            },
            m_tubeShape.GetCurrentTransform());
    }

    void EditorTubeShapeComponent::ConfigurationChanged()
    {
        GenerateVertices();
        ShapeComponentNotificationsBus::Event(
            GetEntityId(), &ShapeComponentNotificationsBus::Events::OnShapeChanged,
            ShapeComponentNotifications::ShapeChangeReasons::ShapeChanged);
    }

    void EditorTubeShapeComponent::OnSplineChanged()
    {
        GenerateVertices();
    }

    void EditorTubeShapeComponent::OnAttributeAdded(size_t index)
    {
        AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(
            &AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
            AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
    }

    void EditorTubeShapeComponent::OnAttributeRemoved(size_t index)
    {
        AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(
            &AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
            AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
    }

    void EditorTubeShapeComponent::OnAttributesSet(size_t size)
    {
        AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(
            &AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
            AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
    }

    void EditorTubeShapeComponent::OnAttributesCleared()
    {
        AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(
            &AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
            AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
    }

    void EditorTubeShapeComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        const bool isActive = GetEntity() ? (GetEntity()->GetState() == AZ::Entity::ES_ACTIVE) : false;
        if (isActive)
        {
            m_tubeShape.Deactivate();
        }

        gameEntity->CreateComponent<TubeShapeComponent>(m_tubeShape);

        if (m_visibleInGameView)
        {
            gameEntity->CreateComponent<TubeShapeDebugDisplayComponent>(m_tubeShapeMeshConfig);
        }

        if (isActive)
        {
            m_tubeShape.Activate(GetEntityId());
        }
    }
} // namespace LmbrCentral
