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

#include "OperatorPushBack.h"
#include <ScriptCanvas/Libraries/Core/MethodUtility.h>
#include <ScriptCanvas/Core/Contracts/SupportsMethodContract.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Operators
        {
            void OperatorPushBack::ConfigureContracts(SourceType sourceType, AZStd::vector<ContractDescriptor>& contractDescs)
            {
                if (sourceType == SourceType::SourceInput)
                {
                    ContractDescriptor supportsMethodContract;
                    supportsMethodContract.m_createFunc = [this]() -> SupportsMethodContract* { return aznew SupportsMethodContract("PushBack"); };
                    contractDescs.push_back(AZStd::move(supportsMethodContract));
                }
            }

            void OperatorPushBack::OnSourceTypeChanged()
            {
                if (Data::IsVectorContainerType(GetSourceAZType()))
                {
                    // Add the INPUT slot for the data to insert
                    Data::Type type = Data::FromAZType(m_sourceTypes[0]);
                    m_inputSlots.insert(AddInputDatumSlot(Data::GetName(type), "", type, Datum::eOriginality::Original, false));

                }

                // TODO: Get the output slot name from the connection.
                Data::Type containerType = Data::FromAZType(GetSourceAZType());
                m_outputSlots.insert(AddOutputTypeSlot("Container", "", containerType, Node::OutputStorage::Required, false));
            }

            void OperatorPushBack::InvokeOperator()
            {
                const SlotSet& slotSets = GetSourceSlots();

                if (!slotSets.empty())
                {
                    SlotId sourceSlotId = (*slotSets.begin());

                    const Datum* containerDatum = GetInput(sourceSlotId);
                    if (containerDatum != nullptr && !containerDatum->Empty() && IsConnected(sourceSlotId))
                    {
                        AZ::BehaviorMethod* method = GetOperatorMethod("PushBack");
                        AZ_Assert(method, "The contract must have failed because you should not be able to invoke an operator for a type that does not have the method");

                        AZStd::array<AZ::BehaviorValueParameter, BehaviorContextMethodHelper::MaxCount> params;

                        AZ::BehaviorValueParameter* paramFirst(params.begin());
                        AZ::BehaviorValueParameter* paramIter = paramFirst;

                        if (Data::IsVectorContainerType(GetSourceAZType()))
                        {
                            // Container
                            auto behaviorParameter = method->GetArgument(0);

                            AZ::Outcome<AZ::BehaviorValueParameter, AZStd::string> param0 = containerDatum->ToBehaviorValueParameter(*behaviorParameter);
                            paramIter->Set(param0.GetValue());
                            ++paramIter;

                            const auto& inputSlotIterator = m_inputSlots.begin();

                            // Value to push
                            behaviorParameter = method->GetArgument(1);
                            const Datum* inputDatum = GetInput(*inputSlotIterator);
                            AZ_Assert(inputDatum, "Unable to GetInput for Input slot Id");
                            if (inputDatum)
                            {
                                AZ::Outcome<AZ::BehaviorValueParameter, AZStd::string> param1 = inputDatum->ToBehaviorValueParameter(*behaviorParameter);
                                paramIter->Set(param1.GetValue());
                                ++paramIter;
                            }
                        }
                        else if (Data::IsMapContainerType(GetSourceAZType()))
                        {
                            SCRIPTCANVAS_REPORT_ERROR((*this), "PushBack is not a supported operation on maps");
                            return;
                        }

                        AZStd::vector<SlotId> resultSlotIDs;
                        resultSlotIDs.push_back();
                        BehaviorContextMethodHelper::Call(*this, false, method, paramFirst, paramIter, resultSlotIDs);

                        PushOutput(*containerDatum, *GetSlot(*m_outputSlots.begin()));
                    }
                }

                SignalOutput(GetSlotId("Out"));
            }

            void OperatorPushBack::OnInputSignal(const SlotId& slotId)
            {
                const SlotId inSlotId = OperatorBaseProperty::GetInSlotId(this);
                if (slotId == inSlotId)
                {
                    InvokeOperator();
                }
            }
        }
    }
}

#include <Include/ScriptCanvas/Libraries/Operators/Containers/OperatorPushBack.generated.cpp>