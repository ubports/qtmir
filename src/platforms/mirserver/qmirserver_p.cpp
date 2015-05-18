/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Mir
#include <mir/main_loop.h>

// local
#include "qmirserver_p.h"

/* FIXME: QThread by default starts an event loop, which is required for correct signal/slot
 * messaging between threads. However below you'll see that the mir server run() method
 * blocks, which blocks the event loop for this thread too. Therefore while mir is running
 * the queued signal/slot mechanism does not work. As workaround, need to use direct call to
 * stop the server.
 */
void MirServerThread::run()
{
    auto const main_loop = server->the_main_loop();
    // By enqueuing the notification code in the main loop, we are
    // ensuring that the server has really and fully started before
    // leaving wait_for_startup().
    main_loop->enqueue(
        this,
        [&]
        {
            std::lock_guard<std::mutex> lock(mutex);
            mir_running = true;
            started_cv.notify_one();
        });

    server->run(); // blocks until Mir server stopped
    Q_EMIT stopped();
}

void MirServerThread::stop()
{
    server->stop();
}

bool MirServerThread::waitForMirStartup()
{
    std::unique_lock<decltype(mutex)> lock(mutex);
    started_cv.wait_for(lock, std::chrono::seconds{10}, [&]{ return mir_running; });
    return mir_running;
}
