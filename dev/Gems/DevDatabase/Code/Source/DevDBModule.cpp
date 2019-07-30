
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include <DevDBSystemComponent.h>
#include <UpdateInventoryComponent.h>
#include <GetInventoryComponent.h>
#include <AddInventoryComponent.h>

namespace DevDatabase
{
    class DevDatabaseModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(DevDatabaseModule, "{2591DB84-57B7-462D-8A20-FC8434908A35}", AZ::Module);
        AZ_CLASS_ALLOCATOR(DevDatabaseModule, AZ::SystemAllocator, 0);

        DevDatabaseModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
			m_descriptors.insert(m_descriptors.end(), {
				DevDBSystemComponent::CreateDescriptor(),
				UpdateInventoryComponent::CreateDescriptor(),
				GetInventoryComponent::CreateDescriptor(),
				AddInventoryComponent::CreateDescriptor()
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<DevDBSystemComponent>(),
                azrtti_typeid<UpdateInventoryComponent>(),
				azrtti_typeid<GetInventoryComponent>(),
				azrtti_typeid<AddInventoryComponent>()
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(DevDatabase_05db9f11910943ee9f77c3eaf111e4a1, DevDatabase::DevDatabaseModule)
