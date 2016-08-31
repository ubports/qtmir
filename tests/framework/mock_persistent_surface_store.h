#ifndef MOCKPERSISTENTSURFACESTORE_H
#define MOCKPERSISTENTSURFACESTORE_H

#include <mir/shell/persistent_surface_store.h>
#include <gmock/gmock.h>

namespace mir
{
namespace shell
{

class MockPersistentSurfaceStore : public PersistentSurfaceStore
{
public:
    MockPersistentSurfaceStore();

    MOCK_METHOD1(id_for_surface, Id(std::shared_ptr<scene::Surface> const& surface));
    MOCK_CONST_METHOD1(surface_for_id, std::shared_ptr<scene::Surface>(Id const& id));
};

}
}

#endif // MOCKPERSISTENTSURFACESTORE_H
