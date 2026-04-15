/****************************************************************************
** Meta object code from reading C++ file 'player_bar.hh'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../MyMusicPlayer/src/widgets/player_bar.hh"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'player_bar.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN9PlayerBarE_t {};
} // unnamed namespace

template <> constexpr inline auto PlayerBar::qt_create_metaobjectdata<qt_meta_tag_ZN9PlayerBarE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlayerBar",
        "control_signal",
        "",
        "action",
        "play_pause_clicked",
        "next_clicked",
        "previous_clicked",
        "order_clicked",
        "seek_requested",
        "progress",
        "volume_changed",
        "volume",
        "favorite_toggled",
        "checked",
        "add_to_playlist_clicked",
        "cover_clicked",
        "on_play_pause_clicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'control_signal'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'play_pause_clicked'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'next_clicked'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'previous_clicked'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'order_clicked'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'seek_requested'
        QtMocHelpers::SignalData<void(double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 9 },
        }}),
        // Signal 'volume_changed'
        QtMocHelpers::SignalData<void(double)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 11 },
        }}),
        // Signal 'favorite_toggled'
        QtMocHelpers::SignalData<void(bool)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Signal 'add_to_playlist_clicked'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'cover_clicked'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'on_play_pause_clicked'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PlayerBar, qt_meta_tag_ZN9PlayerBarE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlayerBar::staticMetaObject = { {
    QMetaObject::SuperData::link<creeper::Widget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9PlayerBarE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9PlayerBarE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9PlayerBarE_t>.metaTypes,
    nullptr
} };

void PlayerBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlayerBar *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->control_signal((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->play_pause_clicked(); break;
        case 2: _t->next_clicked(); break;
        case 3: _t->previous_clicked(); break;
        case 4: _t->order_clicked(); break;
        case 5: _t->seek_requested((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->volume_changed((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->favorite_toggled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->add_to_playlist_clicked(); break;
        case 9: _t->cover_clicked(); break;
        case 10: _t->on_play_pause_clicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)(const QString & )>(_a, &PlayerBar::control_signal, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)()>(_a, &PlayerBar::play_pause_clicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)()>(_a, &PlayerBar::next_clicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)()>(_a, &PlayerBar::previous_clicked, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)()>(_a, &PlayerBar::order_clicked, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)(double )>(_a, &PlayerBar::seek_requested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)(double )>(_a, &PlayerBar::volume_changed, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)(bool )>(_a, &PlayerBar::favorite_toggled, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)()>(_a, &PlayerBar::add_to_playlist_clicked, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlayerBar::*)()>(_a, &PlayerBar::cover_clicked, 9))
            return;
    }
}

const QMetaObject *PlayerBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlayerBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9PlayerBarE_t>.strings))
        return static_cast<void*>(this);
    return creeper::Widget::qt_metacast(_clname);
}

int PlayerBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = creeper::Widget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void PlayerBar::control_signal(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void PlayerBar::play_pause_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PlayerBar::next_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlayerBar::previous_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PlayerBar::order_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PlayerBar::seek_requested(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void PlayerBar::volume_changed(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void PlayerBar::favorite_toggled(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void PlayerBar::add_to_playlist_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void PlayerBar::cover_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
