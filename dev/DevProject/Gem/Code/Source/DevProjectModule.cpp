
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include <DevProjectSystemComponent.h>

namespace DevProject
{
    class DevProjectModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(DevProjectModule, "{B21DCA80-99BA-4D32-B592-767B573F3A1F}", AZ::Module);
        AZ_CLASS_ALLOCATOR(DevProjectModule, AZ::SystemAllocator, 0);

        DevProjectModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                DevProjectSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<DevProjectSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(DevProject_d0c680129a324630990d576b8a997609, DevProject::DevProjectModule)
