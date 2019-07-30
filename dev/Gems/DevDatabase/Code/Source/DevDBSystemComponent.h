#pragma once

#include <AzCore/Component/Component.h>

#include <DevDatabase/DevDBBus.h>
#include <CloudCanvasCommonBus.h>
#include <AzCore/Component/EntityBus.h>

#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/CreateTableRequest.h>
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/UpdateItemRequest.h>
#include <aws/dynamodb/model/ListTablesRequest.h>
#include <aws/dynamodb/model/DescribeTableRequest.h>

namespace DevDatabase
{
	namespace AwsModel
	{
		using namespace Aws::DynamoDB::Model;
	}

    class DevDBSystemComponent
        : public AZ::Component
        , protected DevDBRequestBus::Handler
		, protected CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler
		, protected AZ::EntityBus::Handler

    {
    public:
        AZ_COMPONENT(DevDBSystemComponent, "{C5CFBC1F-8B2D-4C12-89E2-86E47358388A}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

	private:
		Aws::DynamoDB::DynamoDBClient* m_pDynamoDBClient = nullptr;
		//UpdateInventoryRequestBus* m_pUpdateInventory = nullptr;
		//GetInventoryRequestBus* m_pGetInventory = nullptr;
		//AddInventoryRequestBus* m_pAddInventory = nullptr;

    protected:
        ////////////////////////////////////////////////////////////////////////
        // DevDBRequestBus interface implementation
		void GetItem(AwsModel::GetItemRequest& request);
		void AddItem(AwsModel::PutItemRequest& request);
		void UpdateItem(AwsModel::UpdateItemRequest& request);

		void CreateTable(AwsModel::CreateTableRequest& request);
		void ListTables(AwsModel::ListTablesRequest& request);
		void DescribeTable(AwsModel::DescribeTableRequest& request);
		void UpdateTable(AwsModel::UpdateTableRequest& request);
		void DeleteTable(AwsModel::DeleteTableRequest& request);
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// EntityBus interface implementation
		void OnEntityActivated(const AZ::EntityId&) override;
		void OnEntityDeactivated(const AZ::EntityId&) override;
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
