/****************************************************************************
** Meta object code from reading C++ file 'proxysocket.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../QHTTPENGINE/src/src/proxysocket.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'proxysocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ProxySocket_t {
    QByteArrayData data[9];
    char stringdata0[152];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ProxySocket_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ProxySocket_t qt_meta_stringdata_ProxySocket = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ProxySocket"
QT_MOC_LITERAL(1, 12, 21), // "onDownstreamReadyRead"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 24), // "onDownstreamDisconnected"
QT_MOC_LITERAL(4, 60, 19), // "onUpstreamConnected"
QT_MOC_LITERAL(5, 80, 19), // "onUpstreamReadyRead"
QT_MOC_LITERAL(6, 100, 15), // "onUpstreamError"
QT_MOC_LITERAL(7, 116, 23), // "QTcpSocket::SocketError"
QT_MOC_LITERAL(8, 140, 11) // "socketError"

    },
    "ProxySocket\0onDownstreamReadyRead\0\0"
    "onDownstreamDisconnected\0onUpstreamConnected\0"
    "onUpstreamReadyRead\0onUpstreamError\0"
    "QTcpSocket::SocketError\0socketError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProxySocket[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    1,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void ProxySocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ProxySocket *_t = static_cast<ProxySocket *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onDownstreamReadyRead(); break;
        case 1: _t->onDownstreamDisconnected(); break;
        case 2: _t->onUpstreamConnected(); break;
        case 3: _t->onUpstreamReadyRead(); break;
        case 4: _t->onUpstreamError((*reinterpret_cast< QTcpSocket::SocketError(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ProxySocket::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ProxySocket.data,
      qt_meta_data_ProxySocket,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ProxySocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProxySocket::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ProxySocket.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ProxySocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
