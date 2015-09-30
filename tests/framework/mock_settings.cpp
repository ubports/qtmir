#include "mock_settings.h"

testing::MockSettings::MockSettings()
{
    QVariantList lifecycleExemptAppIds;
    lifecycleExemptAppIds << "com.ubuntu.music";
    ON_CALL(*this, get(_))
            .WillByDefault(
                Return(lifecycleExemptAppIds));

}

testing::MockSettings::~MockSettings()
{

}
