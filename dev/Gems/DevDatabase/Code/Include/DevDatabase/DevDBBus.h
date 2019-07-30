#pragma once

#include <AzCore/EBus/EBus.h>

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>


namespace DevDatabase
{
    class DevDBRequests
        : public AZ::EBusTraits
    {

    public:
		//////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        // Put your public methods here
		virtual void GetItem(Aws::DynamoDB::Model::GetItemRequest& request) = 0;
		virtual void AddItem(Aws::DynamoDB::Model::PutItemRequest& request) = 0;
		virtual void UpdateItem(Aws::DynamoDB::Model::UpdateItemRequest& request) = 0;

		virtual void CreateTable(Aws::DynamoDB::Model::CreateTableRequest& request) = 0;
		virtual void ListTables(Aws::DynamoDB::Model::ListTablesRequest& request) = 0;
		virtual void DescribeTable(Aws::DynamoDB::Model::DescribeTableRequest& request) = 0;
		virtual void UpdateTable(Aws::DynamoDB::Model::UpdateTableRequest& request) = 0;
		virtual void DeleteTable(Aws::DynamoDB::Model::DeleteTableRequest& request) = 0;

    };
    using DevDBRequestBus = AZ::EBus<DevDBRequests>;
} // namespace DevDatabase
