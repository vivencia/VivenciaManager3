/****************************************************************************
** Meta object code from reading C++ file 'todolist.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../VivenciaManager3/todolist.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'todolist.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_todoList_t {
    QByteArrayData data[10];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_todoList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_todoList_t qt_meta_stringdata_todoList = {
    {
QT_MOC_LITERAL(0, 0, 8), // "todoList"
QT_MOC_LITERAL(1, 9, 13), // "cellActivated"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 11), // "current_row"
QT_MOC_LITERAL(4, 36, 11), // "current_col"
QT_MOC_LITERAL(5, 48, 8), // "prev_row"
QT_MOC_LITERAL(6, 57, 8), // "prev_col"
QT_MOC_LITERAL(7, 66, 11), // "showMessage"
QT_MOC_LITERAL(8, 78, 9), // "playSound"
QT_MOC_LITERAL(9, 88, 7) // "execApp"

    },
    "todoList\0cellActivated\0\0current_row\0"
    "current_col\0prev_row\0prev_col\0showMessage\0"
    "playSound\0execApp"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_todoList[] = {

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
       1,    4,   34,    2, 0x08 /* Private */,
       7,    1,   43,    2, 0x08 /* Private */,
       8,    1,   46,    2, 0x08 /* Private */,
       9,    1,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,    3,    4,    5,    6,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void todoList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        todoList *_t = static_cast<todoList *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->cellActivated((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 1: _t->showMessage((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 2: _t->playSound((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 3: _t->execApp((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject todoList::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_todoList.data,
      qt_meta_data_todoList,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *todoList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *todoList::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_todoList.stringdata0))
        return static_cast<void*>(const_cast< todoList*>(this));
    return QObject::qt_metacast(_clname);
}

int todoList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
