#include "widgets/home_window.hh"

#include <QByteArray>

#include <creeper-qt/core/application.hh>

using namespace creeper;

auto main(int argc, char** argv) -> int {
    qputenv("QT_MEDIA_BACKEND", QByteArray("gstreamer"));

    app::init {
        app::pro::Complete { argc, argv },
    };

    auto window = TopWindow {};
    window.show();

    return app::exec();
}
