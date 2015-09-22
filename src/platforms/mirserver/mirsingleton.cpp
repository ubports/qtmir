#include "mirsingleton.h"

qtmir::Mir *qtmir::Mir::m_instance = nullptr;

qtmir::Mir::Mir()
{
}

qtmir::Mir::~Mir()
{
    m_instance = nullptr;
}

qtmir::Mir *qtmir::Mir::instance()
{
    if (!m_instance) {
        m_instance = new qtmir::Mir;
    }
    return m_instance;
}

void qtmir::Mir::setCursorName(const QString &cursorName)
{
    if (m_cursorName != cursorName) {
        m_cursorName = cursorName;
        Q_EMIT cursorNameChanged(m_cursorName);
    }
}

QString qtmir::Mir::cursorName() const
{
    return m_cursorName;
}
