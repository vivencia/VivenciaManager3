/****************************************************************************
** Meta object code from reading C++ file 'texteditor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../VivenciaManager3/texteditor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'texteditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_textEditor_t {
    QByteArrayData data[1];
    char stringdata0[11];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_textEditor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_textEditor_t qt_meta_stringdata_textEditor = {
    {
QT_MOC_LITERAL(0, 0, 10) // "textEditor"

    },
    "textEditor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_textEditor[] = {

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

void textEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject textEditor::staticMetaObject = {
    { &documentEditorWindow::staticMetaObject, qt_meta_stringdata_textEditor.data,
      qt_meta_data_textEditor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *textEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *textEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_textEditor.stringdata0))
        return static_cast<void*>(const_cast< textEditor*>(this));
    return documentEditorWindow::qt_metacast(_clname);
}

int textEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = documentEditorWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_textEditorToolBar_t {
    QByteArrayData data[40];
    char stringdata0[720];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_textEditorToolBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_textEditorToolBar_t qt_meta_stringdata_textEditorToolBar = {
    {
QT_MOC_LITERAL(0, 0, 17), // "textEditorToolBar"
QT_MOC_LITERAL(1, 18, 14), // "updateControls"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 14), // "checkAlignment"
QT_MOC_LITERAL(4, 49, 13), // "Qt::Alignment"
QT_MOC_LITERAL(5, 63, 5), // "align"
QT_MOC_LITERAL(6, 69, 22), // "btnCreateTable_clicked"
QT_MOC_LITERAL(7, 92, 22), // "btnRemoveTable_clicked"
QT_MOC_LITERAL(8, 115, 25), // "btnInsertTableRow_clicked"
QT_MOC_LITERAL(9, 141, 25), // "btnRemoveTableRow_clicked"
QT_MOC_LITERAL(10, 167, 25), // "btnInsertTableCol_clicked"
QT_MOC_LITERAL(11, 193, 25), // "btnRemoveTableCol_clicked"
QT_MOC_LITERAL(12, 219, 26), // "btnInsertBulletList_cliked"
QT_MOC_LITERAL(13, 246, 29), // "btnInsertNumberedList_clicked"
QT_MOC_LITERAL(14, 276, 22), // "btnInsertImage_clicked"
QT_MOC_LITERAL(15, 299, 15), // "btnBold_clicked"
QT_MOC_LITERAL(16, 315, 17), // "btnItalic_clicked"
QT_MOC_LITERAL(17, 333, 20), // "btnUnderline_clicked"
QT_MOC_LITERAL(18, 354, 24), // "btnStrikethrough_clicked"
QT_MOC_LITERAL(19, 379, 20), // "btnAlignLeft_clicked"
QT_MOC_LITERAL(20, 400, 21), // "btnAlignRight_clicked"
QT_MOC_LITERAL(21, 422, 22), // "btnAlignCenter_clicked"
QT_MOC_LITERAL(22, 445, 23), // "btnAlignJustify_clicked"
QT_MOC_LITERAL(23, 469, 20), // "btnTextColor_clicked"
QT_MOC_LITERAL(24, 490, 25), // "btnHighlightColor_clicked"
QT_MOC_LITERAL(25, 516, 16), // "btnPrint_clicked"
QT_MOC_LITERAL(26, 533, 23), // "btnPrintPreview_clicked"
QT_MOC_LITERAL(27, 557, 22), // "btnExportToPDF_clicked"
QT_MOC_LITERAL(28, 580, 12), // "previewPrint"
QT_MOC_LITERAL(29, 593, 9), // "QPrinter*"
QT_MOC_LITERAL(30, 603, 11), // "setFontType"
QT_MOC_LITERAL(31, 615, 4), // "type"
QT_MOC_LITERAL(32, 620, 11), // "setFontSize"
QT_MOC_LITERAL(33, 632, 4), // "size"
QT_MOC_LITERAL(34, 637, 23), // "setFontSizeFromComboBox"
QT_MOC_LITERAL(35, 661, 5), // "index"
QT_MOC_LITERAL(36, 667, 11), // "insertImage"
QT_MOC_LITERAL(37, 679, 9), // "imageFile"
QT_MOC_LITERAL(38, 689, 26), // "QTextFrameFormat::Position"
QT_MOC_LITERAL(39, 716, 3) // "pos"

    },
    "textEditorToolBar\0updateControls\0\0"
    "checkAlignment\0Qt::Alignment\0align\0"
    "btnCreateTable_clicked\0btnRemoveTable_clicked\0"
    "btnInsertTableRow_clicked\0"
    "btnRemoveTableRow_clicked\0"
    "btnInsertTableCol_clicked\0"
    "btnRemoveTableCol_clicked\0"
    "btnInsertBulletList_cliked\0"
    "btnInsertNumberedList_clicked\0"
    "btnInsertImage_clicked\0btnBold_clicked\0"
    "btnItalic_clicked\0btnUnderline_clicked\0"
    "btnStrikethrough_clicked\0btnAlignLeft_clicked\0"
    "btnAlignRight_clicked\0btnAlignCenter_clicked\0"
    "btnAlignJustify_clicked\0btnTextColor_clicked\0"
    "btnHighlightColor_clicked\0btnPrint_clicked\0"
    "btnPrintPreview_clicked\0btnExportToPDF_clicked\0"
    "previewPrint\0QPrinter*\0setFontType\0"
    "type\0setFontSize\0size\0setFontSizeFromComboBox\0"
    "index\0insertImage\0imageFile\0"
    "QTextFrameFormat::Position\0pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_textEditorToolBar[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      30,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  164,    2, 0x0a /* Public */,
       3,    1,  165,    2, 0x0a /* Public */,
       6,    0,  168,    2, 0x0a /* Public */,
       7,    0,  169,    2, 0x0a /* Public */,
       8,    0,  170,    2, 0x0a /* Public */,
       9,    0,  171,    2, 0x0a /* Public */,
      10,    0,  172,    2, 0x0a /* Public */,
      11,    0,  173,    2, 0x0a /* Public */,
      12,    0,  174,    2, 0x0a /* Public */,
      13,    0,  175,    2, 0x0a /* Public */,
      14,    0,  176,    2, 0x0a /* Public */,
      15,    1,  177,    2, 0x0a /* Public */,
      16,    1,  180,    2, 0x0a /* Public */,
      17,    1,  183,    2, 0x0a /* Public */,
      18,    1,  186,    2, 0x0a /* Public */,
      19,    0,  189,    2, 0x0a /* Public */,
      20,    0,  190,    2, 0x0a /* Public */,
      21,    0,  191,    2, 0x0a /* Public */,
      22,    0,  192,    2, 0x0a /* Public */,
      23,    0,  193,    2, 0x0a /* Public */,
      24,    0,  194,    2, 0x0a /* Public */,
      25,    0,  195,    2, 0x0a /* Public */,
      26,    0,  196,    2, 0x0a /* Public */,
      27,    0,  197,    2, 0x0a /* Public */,
      28,    1,  198,    2, 0x0a /* Public */,
      30,    1,  201,    2, 0x0a /* Public */,
      32,    1,  204,    2, 0x0a /* Public */,
      34,    1,  207,    2, 0x0a /* Public */,
      36,    2,  210,    2, 0x0a /* Public */,
      36,    1,  215,    2, 0x2a /* Public | MethodCloned */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 29,    2,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void, QMetaType::Int,   33,
    QMetaType::Void, QMetaType::Int,   35,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 38,   37,   39,
    QMetaType::Void, QMetaType::QString,   37,

       0        // eod
};

void textEditorToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        textEditorToolBar *_t = static_cast<textEditorToolBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateControls(); break;
        case 1: _t->checkAlignment((*reinterpret_cast< const Qt::Alignment(*)>(_a[1]))); break;
        case 2: _t->btnCreateTable_clicked(); break;
        case 3: _t->btnRemoveTable_clicked(); break;
        case 4: _t->btnInsertTableRow_clicked(); break;
        case 5: _t->btnRemoveTableRow_clicked(); break;
        case 6: _t->btnInsertTableCol_clicked(); break;
        case 7: _t->btnRemoveTableCol_clicked(); break;
        case 8: _t->btnInsertBulletList_cliked(); break;
        case 9: _t->btnInsertNumberedList_clicked(); break;
        case 10: _t->btnInsertImage_clicked(); break;
        case 11: _t->btnBold_clicked((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 12: _t->btnItalic_clicked((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 13: _t->btnUnderline_clicked((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 14: _t->btnStrikethrough_clicked((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 15: _t->btnAlignLeft_clicked(); break;
        case 16: _t->btnAlignRight_clicked(); break;
        case 17: _t->btnAlignCenter_clicked(); break;
        case 18: _t->btnAlignJustify_clicked(); break;
        case 19: _t->btnTextColor_clicked(); break;
        case 20: _t->btnHighlightColor_clicked(); break;
        case 21: _t->btnPrint_clicked(); break;
        case 22: _t->btnPrintPreview_clicked(); break;
        case 23: _t->btnExportToPDF_clicked(); break;
        case 24: _t->previewPrint((*reinterpret_cast< QPrinter*(*)>(_a[1]))); break;
        case 25: _t->setFontType((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 26: _t->setFontSize((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 27: _t->setFontSizeFromComboBox((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 28: _t->insertImage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QTextFrameFormat::Position(*)>(_a[2]))); break;
        case 29: _t->insertImage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject textEditorToolBar::staticMetaObject = {
    { &QDockWidget::staticMetaObject, qt_meta_stringdata_textEditorToolBar.data,
      qt_meta_data_textEditorToolBar,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *textEditorToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *textEditorToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_textEditorToolBar.stringdata0))
        return static_cast<void*>(const_cast< textEditorToolBar*>(this));
    return QDockWidget::qt_metacast(_clname);
}

int textEditorToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDockWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 30)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 30;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 30)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 30;
    }
    return _id;
}
struct qt_meta_stringdata_imageTextObject_t {
    QByteArrayData data[1];
    char stringdata0[16];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_imageTextObject_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_imageTextObject_t qt_meta_stringdata_imageTextObject = {
    {
QT_MOC_LITERAL(0, 0, 15) // "imageTextObject"

    },
    "imageTextObject"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_imageTextObject[] = {

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

void imageTextObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject imageTextObject::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_imageTextObject.data,
      qt_meta_data_imageTextObject,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *imageTextObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *imageTextObject::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_imageTextObject.stringdata0))
        return static_cast<void*>(const_cast< imageTextObject*>(this));
    if (!strcmp(_clname, "QTextObjectInterface"))
        return static_cast< QTextObjectInterface*>(const_cast< imageTextObject*>(this));
    if (!strcmp(_clname, "org.qt-project.Qt.QTextObjectInterface"))
        return static_cast< QTextObjectInterface*>(const_cast< imageTextObject*>(this));
    return QObject::qt_metacast(_clname);
}

int imageTextObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
