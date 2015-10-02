#include "mock_settings.h"

namespace qtmir
{

MockSettings::MockSettings()
{
    using namespace ::testing;

    QVariantList lifecycleExemptAppIds;
    lifecycleExemptAppIds << "com.ubuntu.music";
    ON_CALL(*this, get(_))
            .WillByDefault(
                Return(lifecycleExemptAppIds));

}

MockSettings::~MockSettings()
{

}

} // namespace qtmir

