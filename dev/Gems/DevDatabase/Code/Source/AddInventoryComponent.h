#pragma once

#include <AzCore/Component/Component.h>

#include <aws/dynamodb/model/PutItemRequest.h>

// Buses we receive stuff from
#include <CloudCanvasCommonBus.h>
#include <DevDatabase/AddInventoryBus.h>

namespace DevDatabase
{
	namespace AwsModel
	{
		using namespace Aws::DynamoDB::Model;
	}

    class AddInventoryComponent
        : public AZ::Component
        , protected AddInventoryRequestBus::Handler
		, protected CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler
    {
    public:
        AZ_COMPONENT(AddInventoryComponent, "{C5342685-8109-474D-9942-5F39D052BD83}");
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

	private:
		const Aws::String m_tableName = "DevProject";
		const Aws::String m_primaryKeyName = "Account";
		const Aws::String m_sortKeyName = "System";

    protected:
        ////////////////////////////////////////////////////////////////////////
        // AddInventoryRequestBus interface implementation
		void AddInventory(Aws::String& accountId, Aws::String& playerID);
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// CloudCanvasCommonBus interface implementation
		void OnPostInitialization() override;
		////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
