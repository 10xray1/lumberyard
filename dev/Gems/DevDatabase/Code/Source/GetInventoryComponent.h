#pragma once

#include <AzCore/Component/Component.h>

#include <aws/dynamodb/model/GetItemRequest.h>

#include <CloudCanvasCommonBus.h>
#include <DevDatabase/GetInventoryBus.h>


namespace DevDatabase
{
	namespace AwsModel
	{
		using namespace Aws::DynamoDB::Model;
	}

    class GetInventoryComponent
        : public AZ::Component
        , protected GetInventoryRequestBus::Handler
		, protected CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler
    {
    public:
        AZ_COMPONENT(GetInventoryComponent, "{E4ED20EE-9222-40C8-9BE6-BC9A4C069F0B}");
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
        // GetInventoryRequestBus interface implementation
		void GetInventoryList(Aws::String& accountId, Aws::String& playerID);
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
