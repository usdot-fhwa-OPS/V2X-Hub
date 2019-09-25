/****************************************************************************
** Meta object code from reading C++ file 'qiodevicecopier_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../QHTTPENGINE/src/src/qiodevicecopier_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qiodevicecopier_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QHttpEngine__QIODeviceCopierPrivate_t {
    QByteArrayData data[5];
    char stringdata0[81];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QHttpEngine__QIODeviceCopierPrivate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QHttpEngine__QIODeviceCopierPrivate_t qt_meta_stringdata_QHttpEngine__QIODeviceCopierPrivate = {
    {
QT_MOC_LITERAL(0, 0, 35), // "QHttpEngine::QIODeviceCopierP..."
QT_MOC_LITERAL(1, 36, 11), // "onReadyRead"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 21), // "onReadChannelFinished"
QT_MOC_LITERAL(4, 71, 9) // "nextBlock"

    },
    "QHttpEngine::QIODeviceCopierPrivate\0"
    "onReadyRead\0\0onReadChannelFinished\0"
    "nextBlock"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QHttpEngine__QIODeviceCopierPrivate[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,
       3,    0,   30,    2, 0x0a /* Public */,
       4,    0,   31,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QHttpEngine::QIODeviceCopierPrivate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QIODeviceCopierPrivate *_t = static_cast<QIODeviceCopierPrivate *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onReadyRead(); break;
        case 1: _t->onReadChannelFinished(); break;
        case 2: _t->nextBlock(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject QHttpEngine::QIODeviceCopierPrivate::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QHttpEngine__QIODeviceCopierPrivate.data,
      qt_meta_data_QHttpEngine__QIODeviceCopierPrivate,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *QHttpEngine::QIODeviceCopierPrivate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QHttpEngine::QIODeviceCopierPrivate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QHttpEngine__QIODeviceCopierPrivate.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int QHttpEngine::QIODeviceCopierPrivate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
