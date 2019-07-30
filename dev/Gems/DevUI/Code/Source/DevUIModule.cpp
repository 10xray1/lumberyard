
#include "DevUI_precompiled.h"
#include <platform_impl.h>

#include <DevUISystemComponent.h>

#include <IGem.h>

namespace DevUI
{
    class DevUIModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(DevUIModule, "{93F82F54-17A4-4C01-BC90-78C8CA5794AF}", CryHooksModule);

        DevUIModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                DevUISystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<DevUISystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(DevUI_b2b3e047f55f444b856f20b9ffaae6d7, DevUI::DevUIModule)
