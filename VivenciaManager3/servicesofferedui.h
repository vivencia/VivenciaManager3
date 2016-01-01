#ifndef SERVICESOFFEREDUI_H
#define SERVICESOFFEREDUI_H

#include <QDialog>

class vmTableWidget;
class servicesOffered;

class servicesOfferedUI : public QDialog
{

    Q_OBJECT

    friend class VivenciaDB;

public:
    ~servicesOfferedUI ();

    static const QString init () {
        if ( !s_instance )
            s_instance = new servicesOfferedUI;
        return QString::null;
    }

    void showWindow ();
    void setupUI ();

    inline vmTableWidget* table () const {
        return m_table;
    }

public slots:
    void readRowData ( const int, const int, const int, const int );
    void tableChanged ( const int row, const int col );
    void rowRemoved ( const uint row = 0 );

private:
    explicit servicesOfferedUI ();
    friend servicesOfferedUI* SERVICES ();
    friend void deleteServicesInstance ();
    static servicesOfferedUI* s_instance;

    servicesOffered* so_rec;
    vmTableWidget* m_table;
};

inline servicesOfferedUI* SERVICES ()
{
    return servicesOfferedUI::s_instance;
}

#endif // SERVICESOFFEREDUI_H
