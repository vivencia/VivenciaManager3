# -------------------------------------------------
# Project created by QtCreator 2010-03-22T17:08:20
# -------------------------------------------------
QT += sql \
	  dbus \
      printsupport

TARGET = VivenciaManager3
TEMPLATE = app
SOURCES += main.cpp \
	dbrecord.cpp \
	vmnumberformats.cpp \
	client.cpp \
	job.cpp \
	payment.cpp \
	purchases.cpp \
	quickproject.cpp \
	supplierrecord.cpp \
	vivenciadb.cpp \
	data.cpp \
	db_image.cpp \
	suppliersdlg.cpp \
	textdb.cpp \
	separatewindow.cpp \
	completers.cpp \
	texteditwithcompleter.cpp \
	calculator.cpp \
	simplecalculator.cpp \
	spellcheck.cpp \
	reportgenerator.cpp \
	standardprices.cpp \
	logindialog.cpp \
	vmcompress.cpp \
	backupdialog.cpp \
	todolist.cpp \
	fileops.cpp \
	configops.cpp \
	documenteditor.cpp \
	documenteditorwindow.cpp \
	texteditor.cpp \
	usermanagement.cpp \
	quickprojectui.cpp \
	emailconfigdialog.cpp \
	vmnotify.cpp \
	vmwidgets.cpp \
	crashrestore.cpp \
	companypurchases.cpp \
	companypurchasesui.cpp \
	dbsupplies.cpp \
	inventory.cpp \
	inventoryui.cpp \
	dbsuppliesui.cpp \
	estimates.cpp \
	generaltable.cpp \
	userrecord.cpp \
	fixdatabase.cpp \
	fixdatabaseui.cpp \
	tristatetype.cpp \
	cleanup.cpp \
	wordhighlighter.cpp \
	vmwidget.cpp \
	stringrecord.cpp \
	mainwindow.cpp \
    vmtaskpanel.cpp \
    vmactiongroup.cpp \
    actionpanelscheme.cpp \
    configdialog.cpp \
    completerrecord.cpp \
    dbcalendar.cpp \
    searchui.cpp \
    usermanagementui.cpp \
    contactsmanagerwidget.cpp \
    machinesrecord.cpp \
    machinesdlg.cpp \
    vmtablewidget.cpp \
    vmtableitem.cpp \
    vmcheckedtableitem.cpp \
    newprojectdialog.cpp \
    vmlistwidget.cpp \
    vmtableutils.cpp \
    vmlistitem.cpp \
    vmlinefilter.cpp \
    keychain/gnomekeyring.cpp \
    keychain/keychain_unix.cpp \
    keychain/keychain.cpp \
    keychain/kwallet_interface.cpp \
    keychain/passwordsessionmanager.cpp \
    htmldb.cpp

HEADERS += dbrecord.h \
	vmnumberformats.h \
	client.h \
	job.h \
	payment.h \
	purchases.h \
	vivenciadb.h \
	data.h \
	supplierrecord.h \
	companypurchases.h \
	companypurchasesui.h \
	dbsupplies.h \
	inventory.h \
	inventoryui.h \
	dbsuppliesui.h \
	estimates.h \
	generaltable.h \
	userrecord.h \
	db_image.h \
	global.h \
	suppliersdlg.h \
	textdb.h \
	quickproject.h \
	separatewindow.h \
	completers.h \
	texteditwithcompleter.h \
	calculator.h \
	simplecalculator.h \
	vmlist.h \
	global_db.h \
	global_enums.h \
	spellcheck.h \
	reportgenerator.h \
	standardprices.h \
	vmwidgets.h \
	logindialog.h \
	vmcompress.h \
	backupdialog.h \
	todolist.h \
	fileops.h \
	configops.h \
	documenteditor.h \
	documenteditorwindow.h \
	texteditor.h \
	usermanagement.h \
	quickprojectui.h \
	emailconfigdialog.h \
	vmnotify.h \
	crashrestore.h \
	fixdatabase.h \
	fixdatabaseui.h \
	mainwindow.h \
    tristatetype.h \
    heapmanager.h \
    cleanup.h \
    wordhighlighter.h \
    vmwidget.h \
    stringrecord.h \
    vmtaskpanel.h \
    vmactiongroup.h \
    actionpanelscheme.h \
    configdialog.h \
    completerrecord.h \
    dbcalendar.h \
    searchui.h \
    usermanagementui.h \
    contactsmanagerwidget.h \
    machinesrecord.h \
    machinesdlg.h \
    vmtablewidget.h \
    vmtableitem.h \
    vmcheckedtableitem.h \
    fast_library_functions.h \
    newprojectdialog.h \
    singleapp.h \
    vmlistwidget.h \
    vmtableutils.h \
    vmlistitem.h \
    vmlinefilter.h \
    keychain/gnomekeyring_p.h \
    keychain/keychain_p.h \
    keychain/keychain.h \
    keychain/kwallet_interface.h \
    keychain/passwordsessionmanager.h \
    htmldb.h

FORMS += mainwindow.ui \
	backupdialog.ui \
    companypurchasesui.ui \
    configdialog.ui \
    simplecalculator.ui

RESOURCES += \
    resources.qrc
LIBS += -L/usr/lib -lhunspell -lbz2
DEFINES += DEBUG QT_USE_QSTRINGBUILDER QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS
INCLUDEPATH += -I/usr/include
QMAKE_CXXFLAGS_RELEASE += -march=native -fomit-frame-pointer -funroll-loops -Ofast
#QMAKE_CXXFLAGS_DEBUG += -g -fvar-tracking-assignments-toggle
QMAKE_CXXFLAGS_DEBUG += -g
#QMAKE_CXXFLAGS += -g -fvar-tracking-assignments-toggle
QMAKE_CXXFLAGS += -Werror -Wall -Wextra -pedantic -std=c++14
TRANSLATIONS += i18n/VivenciaManager_pt_BR.ts

OTHER_FILES +=

DISTFILES += \
    files/spreadsheet.xlsx \
    project_files/spreadsheet.xlsx \
    files/project.docx \
    project_files/project.docx \
    qtkeychain/org.kde.KWallet.xml \
    files/vivencia.jpg \
    project_files/vivencia.jpg \
    project_files/vivencia_report_logo.jpg \
    files/splash.png \
    resources/run.png \
    resources/useradd.png \
    resources/vm-logo.png \
    resources/circle_black.svg \
    files/User_pt_BR.dic \
    Optimization_and_code_sanity.txt \
    i18n/VivenciaManager_pt_BR.ts
