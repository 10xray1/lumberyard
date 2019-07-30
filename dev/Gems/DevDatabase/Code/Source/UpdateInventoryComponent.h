#pragma once

#include <AzCore/Component/Component.h>
#include <CloudCanvasCommonBus.h>
#include <AzCore/Component/EntityBus.h>

#include <aws/dynamodb/model/UpdateItemRequest.h>

#include <DevDatabase/UpdateInventoryBus.h>
#include <DevDatabase/DevDBBus.h>


namespace DevDatabase
{
	namespace AwsModel
	{
		using namespace Aws::DynamoDB::Model;
	}

    class UpdateInventoryComponent
        : public AZ::Component
        , protected UpdateInventoryRequestBus::Handler
		, protected CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler
		, protected AZ::EntityBus::Handler
    {
    public:
        AZ_COMPONENT(UpdateInventoryComponent, "{A9C8BD81-144D-499D-B936-3AEA5BB87208}");
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

	private:
		const Aws::String m_tableName = "DevProject";
		const Aws::String m_primaryKeyName = "Account";
		const Aws::String m_sortKeyName = "System";

		DevDBRequests* m_pDevRequests;

    protected:
        ////////////////////////////////////////////////////////////////////////
        // UpdateInventoryRequestBus interface implementation
		void AddItemToInventory(Aws::String& accountId, Aws::String& playerID, Aws::String& itemId, Aws::String& amount, Aws::String& slotId);
        ////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// CloudCanvasCommonBus interface implementation
		void OnPostInitialization() override;
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// EntityBus interface implementation
		void OnEntityActivated(const AZ::EntityId&) override;
		void OnEntityDeactivated(const AZ::EntityId&) override;
		////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
