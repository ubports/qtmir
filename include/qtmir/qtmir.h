#ifndef QTMIR_H
#define QTMIR_H

#include <QSharedPointer>

namespace qtmir
{
class Screens;

QSharedPointer<Screens> get_screen_model();

}

#endif // QTMIR_H
