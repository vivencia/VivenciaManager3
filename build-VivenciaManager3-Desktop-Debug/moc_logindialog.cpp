/****************************************************************************
** Meta object code from reading C++ file 'logindialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../VivenciaManager3/logindialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'logindialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_loginDialog_t {
    QByteArrayData data[7];
    char stringdata0[109];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_loginDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_loginDialog_t qt_meta_stringdata_loginDialog = {
    {
QT_MOC_LITERAL(0, 0, 11), // "loginDialog"
QT_MOC_LITERAL(1, 12, 21), // "btnTryToLogin_clicked"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 22), // "btnCancelLogin_clicked"
QT_MOC_LITERAL(4, 58, 13), // "checkUserName"
QT_MOC_LITERAL(5, 72, 13), // "checkPassword"
QT_MOC_LITERAL(6, 86, 22) // "btnConfigUsers_clicked"

    },
    "loginDialog\0btnTryToLogin_clicked\0\0"
    "btnCancelLogin_clicked\0checkUserName\0"
    "checkPassword\0btnConfigUsers_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_loginDialog[] = {

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
       1,    0,   39,    2, 0x0a /* Public */,
       3,    0,   40,    2, 0x0a /* Public */,
       4,    0,   41,    2, 0x0a /* Public */,
       5,    0,   42,    2, 0x0a /* Public */,
       6,    0,   43,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void loginDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        loginDialog *_t = static_cast<loginDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->btnTryToLogin_clicked(); break;
        case 1: _t->btnCancelLogin_clicked(); break;
        case 2: _t->checkUserName(); break;
        case 3: _t->checkPassword(); break;
        case 4: _t->btnConfigUsers_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject loginDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_loginDialog.data,
      qt_meta_data_loginDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *loginDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *loginDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_loginDialog.stringdata0))
        return static_cast<void*>(const_cast< loginDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int loginDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_END_MOC_NAMESPACE
