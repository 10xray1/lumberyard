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

#include <AzCore/std/containers/set.h>
#include <AzCore/std/containers/unordered_map.h>

#include <ScriptCanvas/CodeGen/CodeGen.h>
#include <ScriptCanvas/Core/Node.h>
#include <ScriptCanvas/Core/GraphBus.h>
#include <ScriptCanvas/Core/SlotMetadata.h>

#include <Include/ScriptCanvas/Libraries/Operators/Operator.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Operators
        {
            class OperatorBase 
                : public ScriptCanvas::Node 
                , public EndpointNotificationBus::MultiHandler
            {
            public:

                typedef AZStd::vector<const Datum*> OperatorOperands;

                enum class SourceType
                {
                    SourceInput,
                    SourceOutput
                };

                struct SourceSlotConfiguration
                {
                    SourceType m_sourceType = SourceType::SourceInput;

                    DynamicDataType m_dynamicDataType  = DynamicDataType::Any;

                    AZStd::string m_name;
                    AZStd::string m_tooltip;
                };

                struct OperatorConfiguration
                {
                    AZStd::vector< SourceSlotConfiguration > m_sourceSlotConfigurations;
                };

                ScriptCanvas_Node(OperatorBase,
                    ScriptCanvas_Node::Uuid("{30FED030-71ED-4498-AB2C-F5586DFA490E}")
                    ScriptCanvas_Node::Description("")
                    ScriptCanvas_Node::Version(2)
                    ScriptCanvas_Node::Category("Operators")
                );

                ScriptCanvas_In(ScriptCanvas_In::Name("In", "Input signal"));
                ScriptCanvas_Out(ScriptCanvas_Out::Name("Out", "Output signal"));

                using TypeList = AZStd::vector<AZ::TypeId>;
                using SlotSet = AZStd::unordered_set<SlotId>;

                ScriptCanvas_SerializeProperty(SlotSet, m_sourceSlots);

                // Contains the ScriptCanvas data types used for display and other state control
                ScriptCanvas_SerializeProperty(ScriptCanvas::Data::Type, m_sourceType);
                ScriptCanvas_SerializeProperty(ScriptCanvas::Data::Type, m_sourceDisplayType);

                // Has all of the internal AZ types that may be apart of the source type(i.e. for containers)
                TypeList m_sourceTypes;

                //
                ScriptCanvas_SerializeProperty(SlotSet, m_outputSlots);
                ScriptCanvas_SerializeProperty(SlotSet, m_inputSlots);

                OperatorBase();
                OperatorBase(const OperatorConfiguration& c);

                bool IsSourceSlotId(const SlotId& slotId) const;
                const SlotSet& GetSourceSlots() const;

                ScriptCanvas::Data::Type GetSourceType() const;
                AZ::TypeId GetSourceAZType() const;

                ScriptCanvas::Data::Type GetDisplayType() const;

            protected:

                void OnInit() override;

                virtual SlotId AddSourceSlot(SourceSlotConfiguration sourceConfiguration);
                virtual void ConfigureContracts(SourceType sourceType, AZStd::vector<ContractDescriptor>& contractDescs);

                void RemoveInputs();
                void RemoveOutputs();

                void OnEndpointConnected(const Endpoint& endpoint) override;
                void OnEndpointDisconnected(const Endpoint& endpoint) override;

                virtual void OnInputSlotAdded(const SlotId& inputSlotId) { AZ_UNUSED(inputSlotId); };
                virtual void OnDataInputSlotConnected(const SlotId& slotId, const Endpoint& endpoint) {}
                virtual void OnDataInputSlotDisconnected(const SlotId& slotId, const Endpoint& endpoint) {}

                AZ::BehaviorMethod* GetOperatorMethod(const char* methodName);

                SlotId AddSlotWithSourceType();

                void SetSourceType(ScriptCanvas::Data::Type dataType);
                void DisplaySourceType(ScriptCanvas::Data::Type dataType);
                bool HasSourceConnection() const;

                bool AreSourceSlotsFull(SourceType sourceType) const;

                void PopulateAZTypes(ScriptCanvas::Data::Type dataType);

                //! Called when the source data type of the operator has changed, it is used to mutate the node's topology into the desired type
                virtual void OnSourceTypeChanged() {}
                virtual void OnDisplayTypeChanged(ScriptCanvas::Data::Type dataType) {}
                virtual void OnSourceConnected(const SlotId& slotId) {}
                virtual void OnSourceDisconnected(const SlotId& slotId) {}

                //! Implements the operator's behavior, the vector of Datums represents the list of operands.
                virtual void Evaluate(const OperatorOperands& operands, Datum& result)
                {
                    AZ_UNUSED(operands);
                    AZ_UNUSED(result);
                }

            private:
                OperatorConfiguration m_operatorConfiguration;

                AZ::TypeId m_sourceTypeId;
            };

            struct DefaultContainerOperatorConfiguration
                : public OperatorBase::OperatorConfiguration
            {
                DefaultContainerOperatorConfiguration()
                {
                    OperatorBase::SourceSlotConfiguration containerSourceConfig;

                    containerSourceConfig.m_dynamicDataType = DynamicDataType::Container;
                    containerSourceConfig.m_name = "Source";
                    containerSourceConfig.m_tooltip = "The source object to operator on.";
                    containerSourceConfig.m_sourceType = OperatorBase::SourceType::SourceInput;

                    m_sourceSlotConfigurations.emplace_back(containerSourceConfig);
                }
            };

            // Base class for an small helper object that wraps the function to invoke
            class OperationHelper
            {
            public:

                Datum operator()(const AZStd::vector<Datum>& operands, Datum& result)
                {
                    AZStd::vector<Datum>::const_iterator operand = operands.begin();
                    result = *operand;
                    for (++operand; operand != operands.end(); ++operand)
                    {
                        result = Operator(result, *operand);
                    }

                    return result;
                }

            protected:
                virtual Datum Operator(const Datum&, const Datum&) = 0;
            };

// Helper macro to invoke an operator function for a specialized type
#define CallOperatorFunction(Operator, DataType, Type) \
    if (DataType == Data::FromAZType(azrtti_typeid<Type>())) {  Operator<Type> operation; operation(operands, result); }

        }
    }
}
