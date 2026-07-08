#include "widgets/home_window.hh"

#include <QByteArray>

#include <creeper-qt/core/application.hh>

using namespace creeper;

auto main(int argc, char** argv) -> int {
#if defined(Q_OS_LINUX)
    // The PulseAudio compatibility layer exposes Bluetooth sinks more reliably
    // on PipeWire desktops than Qt's native PipeWire audio backend.
    if (!qEnvironmentVariableIsSet("QT_AUDIO_BACKEND")) {
        qputenv("QT_AUDIO_BACKEND", QByteArray("pulseaudio"));
    }
#endif

    app::init {
        app::pro::Complete { argc, argv },
    };

    auto window = TopWindow {};
    window.show();

    return app::exec();
}
