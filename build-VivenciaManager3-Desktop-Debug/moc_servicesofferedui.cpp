/****************************************************************************
** Meta object code from reading C++ file 'servicesofferedui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../VivenciaManager3/servicesofferedui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'servicesofferedui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_servicesOfferedUI_t {
    QByteArrayData data[7];
    char stringdata0[63];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_servicesOfferedUI_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_servicesOfferedUI_t qt_meta_stringdata_servicesOfferedUI = {
    {
QT_MOC_LITERAL(0, 0, 17), // "servicesOfferedUI"
QT_MOC_LITERAL(1, 18, 11), // "readRowData"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 12), // "tableChanged"
QT_MOC_LITERAL(4, 44, 3), // "row"
QT_MOC_LITERAL(5, 48, 3), // "col"
QT_MOC_LITERAL(6, 52, 10) // "rowRemoved"

    },
    "servicesOfferedUI\0readRowData\0\0"
    "tableChanged\0row\0col\0rowRemoved"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_servicesOfferedUI[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    4,   34,    2, 0x0a /* Public */,
       3,    2,   43,    2, 0x0a /* Public */,
       6,    1,   48,    2, 0x0a /* Public */,
       6,    0,   51,    2, 0x2a /* Public | MethodCloned */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,    2,    2,    2,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    4,    5,
    QMetaType::Void, QMetaType::UInt,    4,
    QMetaType::Void,

       0        // eod
};

void servicesOfferedUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        servicesOfferedUI *_t = static_cast<servicesOfferedUI *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->readRowData((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3])),(*reinterpret_cast< const int(*)>(_a[4]))); break;
        case 1: _t->tableChanged((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 2: _t->rowRemoved((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 3: _t->rowRemoved(); break;
        default: ;
        }
    }
}

const QMetaObject servicesOfferedUI::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_servicesOfferedUI.data,
      qt_meta_data_servicesOfferedUI,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *servicesOfferedUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *servicesOfferedUI::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_servicesOfferedUI.stringdata0))
        return static_cast<void*>(const_cast< servicesOfferedUI*>(this));
    return QDialog::qt_metacast(_clname);
}

int servicesOfferedUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
