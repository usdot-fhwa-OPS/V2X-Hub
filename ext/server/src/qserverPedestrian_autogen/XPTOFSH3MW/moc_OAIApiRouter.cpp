/****************************************************************************
** Meta object code from reading C++ file 'OAIApiRouter.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../handlers/OAIApiRouter.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OAIApiRouter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OpenAPI__OAIApiRequestHandler_t {
    QByteArrayData data[5];
    char stringdata0[75];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OpenAPI__OAIApiRequestHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OpenAPI__OAIApiRequestHandler_t qt_meta_stringdata_OpenAPI__OAIApiRequestHandler = {
    {
QT_MOC_LITERAL(0, 0, 29), // "OpenAPI::OAIApiRequestHandler"
QT_MOC_LITERAL(1, 30, 15), // "requestReceived"
QT_MOC_LITERAL(2, 46, 0), // ""
QT_MOC_LITERAL(3, 47, 20), // "QHttpEngine::Socket*"
QT_MOC_LITERAL(4, 68, 6) // "socket"

    },
    "OpenAPI::OAIApiRequestHandler\0"
    "requestReceived\0\0QHttpEngine::Socket*\0"
    "socket"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OpenAPI__OAIApiRequestHandler[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void OpenAPI::OAIApiRequestHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OAIApiRequestHandler *_t = static_cast<OAIApiRequestHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->requestReceived((*reinterpret_cast< QHttpEngine::Socket*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QHttpEngine::Socket* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (OAIApiRequestHandler::*_t)(QHttpEngine::Socket * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OAIApiRequestHandler::requestReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject OpenAPI::OAIApiRequestHandler::staticMetaObject = {
    { &QHttpEngine::QObjectHandler::staticMetaObject, qt_meta_stringdata_OpenAPI__OAIApiRequestHandler.data,
      qt_meta_data_OpenAPI__OAIApiRequestHandler,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *OpenAPI::OAIApiRequestHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpenAPI::OAIApiRequestHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OpenAPI__OAIApiRequestHandler.stringdata0))
        return static_cast<void*>(this);
    return QHttpEngine::QObjectHandler::qt_metacast(_clname);
}

int OpenAPI::OAIApiRequestHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QHttpEngine::QObjectHandler::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void OpenAPI::OAIApiRequestHandler::requestReceived(QHttpEngine::Socket * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_OpenAPI__OAIApiRouter_t {
    QByteArrayData data[1];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OpenAPI__OAIApiRouter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OpenAPI__OAIApiRouter_t qt_meta_stringdata_OpenAPI__OAIApiRouter = {
    {
QT_MOC_LITERAL(0, 0, 21) // "OpenAPI::OAIApiRouter"

    },
    "OpenAPI::OAIApiRouter"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OpenAPI__OAIApiRouter[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void OpenAPI::OAIApiRouter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject OpenAPI::OAIApiRouter::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_OpenAPI__OAIApiRouter.data,
      qt_meta_data_OpenAPI__OAIApiRouter,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *OpenAPI::OAIApiRouter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpenAPI::OAIApiRouter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OpenAPI__OAIApiRouter.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int OpenAPI::OAIApiRouter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
