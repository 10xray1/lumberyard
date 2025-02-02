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

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Module/Environment.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

#include <GraphCanvas/GraphCanvasBus.h>
#include <GraphCanvas/Styling/PseudoElement.h>

namespace GraphCanvas
{
    class GraphCanvasSystemComponent
        : public AZ::Component
        , private GraphCanvasRequestBus::Handler
        , protected Styling::PseudoElementFactoryRequestBus::Handler
    {
    public:
        AZ_COMPONENT(GraphCanvasSystemComponent, "{F9F7BE55-4C28-4B8A-A722-D47C9EF24E60}")

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        GraphCanvasSystemComponent() = default;
        ~GraphCanvasSystemComponent() override = default;

    private:
        // AZ::Component
        void Activate() override;
        void Deactivate() override;
        ////

        // GraphCanvasRequestBus
        AZ::Entity* CreateBookmarkAnchor() const override;

        AZ::Entity* CreateScene() const override;

        AZ::Entity* CreateCoreNode() const override;
        AZ::Entity* CreateGeneralNode(const char* nodeType) const override;
        AZ::Entity* CreateCommentNode() const override;
        AZ::Entity* CreateWrapperNode(const char* nodeType) const override;

        AZ::Entity* CreateNodeGroup() const override;
        AZ::Entity* CreateCollapsedNodeGroup(const CollapsedNodeGroupConfiguration& groupedNodeConfiguration) const override;

        AZ::Entity* CreateSlot(const AZ::EntityId& nodeId, const SlotConfiguration& slotConfiguration) const override;

        NodePropertyDisplay* CreateBooleanNodePropertyDisplay(BooleanDataInterface* dataInterface) const override;
        NodePropertyDisplay* CreateNumericNodePropertyDisplay(NumericDataInterface* dataInterface) const override;
        NodePropertyDisplay* CreateEntityIdNodePropertyDisplay(EntityIdDataInterface* dataInterface) const override;
        NodePropertyDisplay* CreateItemModelNodePropertyDisplay(ItemModelDataInterface* dataInterface) const override;
        NodePropertyDisplay* CreateReadOnlyNodePropertyDisplay(ReadOnlyDataInterface* dataInterface) const override;
        NodePropertyDisplay* CreateStringNodePropertyDisplay(StringDataInterface* dataInterface) const override;
        NodePropertyDisplay* CreateVectorNodePropertyDisplay(VectorDataInterface* dataInterface) const override;

        AZ::Entity* CreatePropertySlot(const AZ::EntityId& nodeId, const AZ::Crc32& propertyId, const SlotConfiguration& slotConfiguration) const override;
        ////

        // PseudoElementRequestBus
        AZ::EntityId CreateStyleEntity(const AZStd::string& style) const override;
        AZ::EntityId CreateVirtualChild(const AZ::EntityId& real, const AZStd::string& virtualChildElement) const override;
        ////
    };
}