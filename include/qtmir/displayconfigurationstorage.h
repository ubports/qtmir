#ifndef DISPLAYCONFIGURATIONSTORAGE_H
#define DISPLAYCONFIGURATIONSTORAGE_H

#include "qtmir/miral/display_configuration_storage.h"

class QMirServer;

namespace qtmir
{

using DisplayConfigurationStorageBuilder = std::function<std::shared_ptr<miral::DisplayConfigurationStorage>()>;

class BasicSetDisplayConfigurationStorage
{
public:
    explicit BasicSetDisplayConfigurationStorage(DisplayConfigurationStorageBuilder const& builder);
    ~BasicSetDisplayConfigurationStorage() = default;

    void operator()(QMirServer& server);
    DisplayConfigurationStorageBuilder builder() const;

private:
    struct Self;
    std::shared_ptr<Self> self;
};

/*
    Set the display configuration policy to allow server customization
 */
template<typename Policy>
class SetDisplayConfigurationStorage : public BasicSetDisplayConfigurationStorage
{
public:
    template<typename ...Args>
    explicit SetDisplayConfigurationStorage(Args const& ...args) :
        BasicSetDisplayConfigurationStorage{
            [&args...]() { return std::make_shared<Policy>(args...); }} {}
};

}

#endif // DISPLAYCONFIGURATIONSTORAGE_H
