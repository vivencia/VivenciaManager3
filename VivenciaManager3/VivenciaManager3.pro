# -------------------------------------------------
# Project created by QtCreator 2010-03-22T17:08:20
# -------------------------------------------------
QT += sql \
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
	listitems.cpp \
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
	servicesofferedui.cpp \
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
    passwordmanager.cpp \
    usermanagementui.cpp \
    contactsmanagerwidget.cpp \
    machinesrecord.cpp \
    machinesdlg.cpp \
    vmtablewidget.cpp \
    vmtableitem.cpp \
    vmcheckedtableitem.cpp \
    newprojectdialog.cpp

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
	servicesofferedui.h \
	generaltable.h \
	userrecord.h \
	listitems.h \
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
    passwordmanager.h \
    usermanagementui.h \
    contactsmanagerwidget.h \
    machinesrecord.h \
    machinesdlg.h \
    vmtablewidget.h \
    vmtableitem.h \
    vmcheckedtableitem.h \
    fast_library_functions.h \
    newprojectdialog.h
FORMS += mainwindow.ui \
	backupdialog.ui \
    companypurchasesui.ui \
    configdialog.ui
RESOURCES += resources.qrc
LIBS += -L/usr/lib -lhunspell -lbz2
DEFINES += DEBUG QT_USE_QSTRINGBUILDER QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS TRANSITION_PERIOD
INCLUDEPATH += -I/usr/include
#QMAKE_CXXFLAGS_RELEASE += -O3 -march=core2 -mtune=core2 -fomit-frame-pointer -funroll-loops
#QMAKE_CXXFLAGS_DEBUG += -g -fvar-tracking-assignments-toggle
QMAKE_CXXFLAGS_DEBUG += -g -std=c++14
#QMAKE_CXXFLAGS += -g -fvar-tracking-assignments-toggle
QMAKE_CXXFLAGS += -g -std=c++0x -Werror -Wextra -Wall -pedantic
TRANSLATIONS += i18n/VivenciaManager_pt_BR.ts

OTHER_FILES +=
