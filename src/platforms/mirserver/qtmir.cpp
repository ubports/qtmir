#include "qtmir/qtmir.h"
#include "screenadaptormodel.h"

QWeakPointer<qtmir::Screens> weakScreenModel;

QSharedPointer<qtmir::Screens> qtmir::get_screen_model()
{
    auto model = weakScreenModel.lock();
    if (!model) {
        QSharedPointer<qtmir::Screens> newModel{new ScreenAdaptorModel()};
        weakScreenModel = model;
        return newModel;
    }
    return model;
}
