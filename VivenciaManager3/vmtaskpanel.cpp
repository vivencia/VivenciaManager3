#include "vmtaskpanel.h"
#include "vmactiongroup.h"
#include "actionpanelscheme.h"
#include "vmwidgets.h"

vmTaskPanel::vmTaskPanel ( const QString& title, QWidget* parent )
	: QFrame ( parent ), mTitle ( nullptr )
{
	setProperty ( "class", QStringLiteral ( "panel" ) );
	setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
	setScheme ( ActionPanelScheme::defaultScheme () );

	mLayout = new QVBoxLayout;
	mLayout->setMargin ( 5 );
	mLayout->setSpacing ( 5 );
	mLayout->setSizeConstraint ( QLayout::SetMinAndMaxSize );
	setLayout ( mLayout );

	if ( !title.isEmpty () )
	{
		mTitle = new vmActionLabel ( title );
		mLayout->insertWidget( 0, mTitle );
	}
}

vmTaskPanel::~vmTaskPanel () {}

void vmTaskPanel::setTitle ( const QString& new_title )
{
	if ( !mTitle )
	{
		mTitle = new vmActionLabel ( new_title );
		mLayout->insertWidget( 0, mTitle );
	}
	else
		mTitle->setText ( new_title );
}

void vmTaskPanel::setScheme ( ActionPanelScheme* scheme )
{
	if ( scheme )
	{
		mScheme = scheme;
		setStyleSheet ( mScheme->actionStyle );

		// set scheme for children
		QObjectList list ( children () );
		foreach ( QObject *obj, list )
		{
			if ( dynamic_cast<vmActionGroup*> ( obj ) )
			{
				( static_cast<vmActionGroup*>( obj ) )->setScheme ( scheme );
				continue;
			}
		}
		update ();
	}
}

void vmTaskPanel::addWidget ( QWidget* w )
{
	if ( w )
		mLayout->addWidget ( w );
}

void vmTaskPanel::addStretch ( const int s )
{
	mLayout->addStretch ( s );
}

vmActionGroup* vmTaskPanel::createGroup (const QString& title, const bool expandable,
		const bool stretchContents, const bool closable )
{
	vmActionGroup* box ( new vmActionGroup ( title, expandable, stretchContents, closable, this ) );
	addWidget ( box );
	return box;
}

vmActionGroup* vmTaskPanel::createGroup ( const QIcon& icon, const QString& title,
		const bool expandable, const bool stretchContents, const bool closable )
{
	vmActionGroup* box ( new vmActionGroup ( icon, title, expandable, stretchContents, closable, this ) );
	addWidget ( box );
	return box;
}

void vmTaskPanel::removeGroup ( vmActionGroup* group, const bool bDelete )
{
	if ( group )
	{
		mLayout->removeWidget ( group );
		if ( bDelete )
			delete group;
	}
}

QSize vmTaskPanel::minimumSizeHint ()  const
{
	return QSize ( 200, 150 );
}
