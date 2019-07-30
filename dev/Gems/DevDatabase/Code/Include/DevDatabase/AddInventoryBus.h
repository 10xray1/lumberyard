#pragma once

#include <AzCore/EBus/EBus.h>

namespace DevDatabase
{
    class AddInventoryRequests
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        // Put your public methods here
		virtual void AddInventory(Aws::String& accountId, Aws::String& playerId) = 0;

    };
    using AddInventoryRequestBus = AZ::EBus<AddInventoryRequests>;
} // namespace DevDatabase
