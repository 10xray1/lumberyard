
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include <DevInventorySystemComponent.h>

namespace DevInventory
{
    class DevInventoryModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(DevInventoryModule, "{BACA2F5B-C2D1-4CFA-ADC8-0E137B981973}", AZ::Module);
        AZ_CLASS_ALLOCATOR(DevInventoryModule, AZ::SystemAllocator, 0);

        DevInventoryModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                DevInventorySystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<DevInventorySystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(DevInventory_12a7cb0e45c645c7bbdd44c8f6b1cbc8, DevInventory::DevInventoryModule)
