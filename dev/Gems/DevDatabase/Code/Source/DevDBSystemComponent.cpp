
#include <DevDBSystemComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/Outcome.h>

// Buses we send stuff to
#include <CommandRegistrationBus.h>
#include <Include/DevDatabase/UpdateInventoryBus.h>
#include <Include/DevDatabase/GetInventoryBus.h>
#include <Include/DevDatabase/AddInventoryBus.h>

#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

#include <AzCore/IO/FileIO.h>
#include <AzCore/XML/rapidxml.h>


namespace DevDatabase
{
    void DevDBSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<DevDBSystemComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<DevDBSystemComponent>("DevDatabase", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }
    void DevDBSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("DevDatabaseService"));
    }
    void DevDBSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("DevDatabaseService"));
    }
    void DevDBSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        AZ_UNUSED(required);
    }
    void DevDBSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
	void DevDBSystemComponent::Init()
    {
    }
    void DevDBSystemComponent::Activate()
    {
		CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusConnect();
		DevDBRequestBus::Handler::BusConnect();
	}
    void DevDBSystemComponent::Deactivate()
    {
		Aws::Utils::Logging::ShutdownAWSLogging();

        DevDBRequestBus::Handler::BusDisconnect();
		CloudCanvasCommon::CloudCanvasCommonNotificationsBus::Handler::BusDisconnect();
    }

	void DevDBSystemComponent::LoadConfigXML(AZStd::string& AWSCredsXML)
	{
		AZ_TracePrintf("DevDatabase - System", "Loading AWS XML");

		AZStd::string fileContents;
		AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();

		// Check if file exists
		if (fileIO && fileIO->Exists(AWSCredsXML.c_str()))
		{
			AZ::IO::HandleType AWSFileHandle = AZ::IO::InvalidHandle;
			AZ::u64 size = 0;
			AZ::IO::FileIOBase::GetInstance()->Size(AWSCredsXML.c_str(), size);

			if (size)
			{
				if (AZ::IO::FileIOBase::GetInstance()->Open(AWSCredsXML.c_str(), AZ::IO::OpenMode::ModeRead, AWSFileHandle))
				{
					// Open and read file
					fileContents.resize_no_construct(size);
					if (!AZ::IO::FileIOBase::GetInstance()->Read(AWSFileHandle, fileContents.data(), fileContents.size(), true))
					{
						AZ_Error("DevDatabase - System", false, "File %s failed read - read was truncated!", AWSFileHandle);
						fileContents.set_capacity(0);
					}
					AZ::IO::FileIOBase::GetInstance()->Close(AWSFileHandle);
				}

				if (!fileContents.empty())
				{
					// Parse XML
					AZ::rapidxml::xml_document<> configXML;
					configXML.parse<AZ::rapidxml::parse_full>(fileContents.data());

					if (!configXML.isError())
					{
						AZ::rapidxml::xml_node<>* cur_node;
					
						cur_node = configXML.first_node("AWSCredentialsConfig");
						cur_node = cur_node->first_node("AWSAccessKeyId");
						Aws::String DynamoDB_IDKey = cur_node->first_attribute("id")->value();

						cur_node = configXML.first_node("AWSCredentialsConfig");
						cur_node = cur_node->first_node("AWSSecretKey");
						Aws::String DynamoDB_SecretKey = cur_node->first_attribute("key")->value();

						// Create Config
						Aws::Client::ClientConfiguration clientConfiguration;
						clientConfiguration.region = "us-west-1";
						clientConfiguration.endpointOverride = "dynamodb.us-west-1.amazonaws.com";
						m_pDynamoDBClient = new Aws::DynamoDB::DynamoDBClient(Aws::Auth::AWSCredentials(DynamoDB_IDKey, DynamoDB_SecretKey), clientConfiguration);
						AZ_TracePrintf("DevDatabase - System", "Client config created");
					}
				}
			}
		}
	}

	void DevDBSystemComponent::GetItem(AwsModel::GetItemRequest& request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::GetItemOutcome& result = m_pDynamoDBClient->GetItem(request);
			if (!result.IsSuccess()) {
				AZ_TracePrintf("DevDatabase - System", "Failed to get item: %s", result.GetError().GetMessage());
				return;
			}

			const Aws::Map<Aws::String, AwsModel::AttributeValue>& item = result.GetResult().GetItem();
			if (item.empty())
			{
				AZ_TracePrintf("DevDatabase - System", "No item found with provided key.");
				return;
			}

			for (const auto& i : item)
				AZ_TracePrintf("DevDatabase - System", "AttrName: %s, AttrValue: %s", i.first, i.second.GetS());
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}
	}

	void DevDBSystemComponent::AddItem(AwsModel::PutItemRequest& request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::PutItemOutcome result = m_pDynamoDBClient->PutItem(request);
			if (!result.IsSuccess())
			{
				AZ_TracePrintf("DevDatabase - System", "Error: %s ", result.GetError().GetMessage());
				return;
			}
			AZ_TracePrintf("DevDatabase - System", "Item added.");
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}
	}

	void DevDBSystemComponent::UpdateItem(AwsModel::UpdateItemRequest& request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::UpdateItemOutcome& result = m_pDynamoDBClient->UpdateItem(request);
			if (!result.IsSuccess())
			{
				AZ_TracePrintf("DevDatabase - System", "Error: %s", result.GetError().GetMessage());
				return;
			}
			AZ_TracePrintf("DevDatabase - System", "Item Updated.");
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}

	}

	void DevDBSystemComponent::CreateTable(AwsModel::CreateTableRequest& request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::CreateTableOutcome& result = m_pDynamoDBClient->CreateTable(request);
			if (!result.IsSuccess())
			{
				AZ_TracePrintf("DevDatabase - System", "Failed to create table: %s", result.GetError().GetMessage());
				return;
			}
			AZ_TracePrintf("DevDatabase - System", "Table %s created.", result.GetResult().GetTableDescription().GetTableName());
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}
	}

	void DevDBSystemComponent::ListTables(AwsModel::ListTablesRequest& request)
	{
		if (m_pDynamoDBClient)
		{
			do
			{
				const AwsModel::ListTablesOutcome& result = m_pDynamoDBClient->ListTables(request);
				if (!result.IsSuccess())
				{
					AZ_TracePrintf("DevDatabase - System", "Error: %s", result.GetError().GetMessage());
					return;
				}
				for (const auto& s : result.GetResult().GetTableNames())
					AZ_TracePrintf("DevDatabase - System", "%s", s);
				request.SetExclusiveStartTableName(result.GetResult().GetLastEvaluatedTableName());
			} while (!request.GetExclusiveStartTableName().empty());
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}

	}

	void DevDBSystemComponent::DescribeTable(AwsModel::DescribeTableRequest & request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::DescribeTableOutcome& result = m_pDynamoDBClient->DescribeTable(request);

			if (result.IsSuccess())
			{
				const AwsModel::TableDescription& td = result.GetResult().GetTable();
				AZ_TracePrintf("DevDatabase - System", "Table name  : %s", td.GetTableName());
				AZ_TracePrintf("DevDatabase - System", "Table ARN   : %s", td.GetTableArn());
				AZ_TracePrintf("DevDatabase - System", "Status      : %s", AwsModel::TableStatusMapper::GetNameForTableStatus(td.GetTableStatus()));
				AZ_TracePrintf("DevDatabase - System", "Item count  : %s", td.GetItemCount());
				AZ_TracePrintf("DevDatabase - System", "Size (bytes): %s", td.GetTableSizeBytes());

				const AwsModel::ProvisionedThroughputDescription& ptd = td.GetProvisionedThroughput();
				AZ_TracePrintf("DevDatabase - System", "Throughput");
				AZ_TracePrintf("DevDatabase - System", "  Read Capacity : %s", ptd.GetReadCapacityUnits());
				AZ_TracePrintf("DevDatabase - System", "  Write Capacity: %s", ptd.GetWriteCapacityUnits());

				const Aws::Vector<AwsModel::AttributeDefinition>& ad = td.GetAttributeDefinitions();
				AZ_TracePrintf("DevDatabase - System", "Attributes");
				for (const auto& a : ad)
					AZ_TracePrintf("DevDatabase - System", "  %s (%s)", a.GetAttributeName(), AwsModel::ScalarAttributeTypeMapper::GetNameForScalarAttributeType(a.GetAttributeType()));
			}
			else
			{
				AZ_TracePrintf("DevDatabase - System", "Failed to describe table: %s", result.GetError().GetMessage());
			}
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}
	}

	void DevDBSystemComponent::UpdateTable(AwsModel::UpdateTableRequest & request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::UpdateTableOutcome& result = m_pDynamoDBClient->UpdateTable(request);
			if (!result.IsSuccess())
			{
				AZ_TracePrintf("DevDatabase - System", "%s", result.GetError().GetMessage());
				return;
			}
			AZ_TracePrintf("DevDatabase - System", "Table Updated");
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}
	}

	void DevDBSystemComponent::DeleteTable(AwsModel::DeleteTableRequest & request)
	{
		if (m_pDynamoDBClient)
		{
			const AwsModel::DeleteTableOutcome& result = m_pDynamoDBClient->DeleteTable(request);
			if (result.IsSuccess())
			{
				AZ_TracePrintf("DevDatabase - System", "Table %s deleted", result.GetResult().GetTableDescription().GetTableName());
			}
			else
			{
				AZ_TracePrintf("DevDatabase - System", "Failed to delete table: ", result.GetError().GetMessage());
			}
		}
		else
		{
			AZ_Error("DevDatabase - System", false, "Bad AWS Client Config!");
		}
	}

	void DevDBSystemComponent::OnPostInitialization()
	{
		AZ_TracePrintf("DevDatabase - System", "AWS initialization complete");

		//Enable logging
		Aws::Utils::Logging::InitializeAWSLogging(Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>("RunUnitTests", Aws::Utils::Logging::LogLevel::Trace, "aws_sdk_"));

		// Load xml config and set the dbclient
		AZStd::string AWSCredsXML = "Config/AWSCreds.xml";
		LoadConfigXML(AWSCredsXML);

		// Add item into inventory console command
		AzFramework::CommandRegistrationBus::Broadcast(&AzFramework::CommandRegistrationBus::Events::RegisterCommand
			, "dev_addInventory"
			, "Add item to player inventory: dev_addInventory <accountId> <playerId> <itemId> <amount> <slotId>"
			, AzFramework::CommandFlags::NoValue, [](const AZStd::vector<AZStd::string_view>& args) -> AzFramework::CommandResult
		{
			if (args.size() == 6)
			{
				UpdateInventoryRequestBus::Broadcast(&UpdateInventoryRequestBus::Events::AddItemToInventory
					, Aws::String(args[1].data()), Aws::String(args[2].data()), Aws::String(args[3].data()), Aws::String(args[4].data()), Aws::String(args[5].data()));
			}
			else
			{
				return AzFramework::CommandResult::ErrorWrongNumberOfArguements;
			}
			return AzFramework::CommandResult::Success;
		});

		// List inventory console command
		AzFramework::CommandRegistrationBus::Broadcast(&AzFramework::CommandRegistrationBus::Events::RegisterCommand
			, "dev_listInventory"
			, "List player inventory: dev_listInventory <accountId> <playerId>"
			, AzFramework::CommandFlags::NoValue, [](const AZStd::vector<AZStd::string_view>& args) -> AzFramework::CommandResult
		{
			if (args.size() == 3)
			{
				AZ_TracePrintf("DevDatabase - System", "Sending account/player to GetInventory: %s/%s", Aws::String(args[1].data()), Aws::String(args[2].data()));
				GetInventoryRequestBus::Broadcast(&GetInventoryRequestBus::Events::GetInventoryList, Aws::String(args[1].data()), Aws::String(args[2].data()));
			}
			else
			{
				return AzFramework::CommandResult::ErrorWrongNumberOfArguements;
			}
			return AzFramework::CommandResult::Success;
		});

		// Add new player inventory item console command
		AzFramework::CommandRegistrationBus::Broadcast(&AzFramework::CommandRegistrationBus::Events::RegisterCommand
			, "dev_newInventory"
			, "Create new inventory for player: dev_newInventory <accountId> <playerId>"
			, AzFramework::CommandFlags::NoValue, [](const AZStd::vector<AZStd::string_view>& args) -> AzFramework::CommandResult
		{
			if (args.size() == 3)
			{
				AddInventoryRequestBus::Broadcast(&AddInventoryRequestBus::Events::AddInventory, Aws::String(args[1].data()), Aws::String(args[2].data()));
			}
			else
			{
				return AzFramework::CommandResult::ErrorWrongNumberOfArguements;
			}
			return AzFramework::CommandResult::Success;
		});
	}
}
