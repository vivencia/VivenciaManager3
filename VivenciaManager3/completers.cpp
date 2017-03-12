#include "completers.h"
#include "global.h"
#include "data.h"
#include "stringrecord.h"
#include "dbrecord.h"
#include "cleanup.h"
#include "completerrecord.h"
#include "heapmanager.h"
#include "vmwidgets.h"

#include <QStandardItemModel>

vmCompleters* vmCompleters::s_instance ( nullptr );

void deleteCompletersInstance ()
{
	heap_del ( vmCompleters::s_instance );
}

vmCompleters::vmCompleters ()
	: cr_rec ( nullptr ), completersList ( COMPLETERS_COUNT + 1 )
{
	QCompleter* completer ( nullptr );
	QStandardItemModel* item_model ( nullptr );
	for ( uint i ( 0 ); i < COMPLETERS_COUNT; ++i )
	{
		completer = new QCompleter;
		completer->setCompletionMode ( QCompleter::PopupCompletion );
		completer->setCaseSensitivity ( Qt::CaseInsensitive );
		completer->setProperty ( "type", QVariant ( i ) );
		// Initialize SERVICES with two columns. The first holds a stringRecord with the pertiment
		// information; the second, an index for faster searches
		item_model = new QStandardItemModel ( 0, i != PRODUCT_OR_SERVICE ? 1 : 2, completer );
		completer->setModel ( item_model );
		//item_model->appendRow ( new QStandardItem ( QStringLiteral ( "N/A" ) ) );
		completersList.insert ( i, completer );
	}
	loadCompleters ();
	addPostRoutine ( deleteCompletersInstance );
}

vmCompleters::~vmCompleters ()
{
	heap_del ( cr_rec );
	completersList.clear ( true );
}

void vmCompleters::loadCompleters ()
{
	cr_rec = new completerRecord;
	QStringList completer_strings;
	QStandardItemModel* item_model ( nullptr );
	QStandardItemModel* model_all ( static_cast<QStandardItemModel*> ( completersList.at ( ALL_CATEGORIES )->model () ) );

	for ( int i ( 2 ), x ( 0 ), str_count ( 0 ); i < static_cast<int>(COMPLETERS_COUNT); ++i )
	{
		cr_rec->loadCompleterStrings ( completer_strings, static_cast<COMPLETER_CATEGORIES> ( i ) );
		str_count = completer_strings.count ();
		if ( str_count > 0 )
		{
			item_model = static_cast<QStandardItemModel*> ( completersList.at ( i )->model () );
			for ( x = 0; x < str_count; ++x )
			{
				item_model->appendRow ( new QStandardItem ( completer_strings.at ( x ) ) );
				model_all->appendRow ( new QStandardItem ( completer_strings.at ( x ) ) );
			}
			completer_strings.clear ();
		}
	}

	QStringList completer_strings_2;
	cr_rec->loadCompleterStringsForProductsAndServices ( completer_strings, completer_strings_2 );
	const int str_count ( static_cast<int> ( qMin ( completer_strings.count (), completer_strings_2.count () ) ) );
	if ( str_count > 0 )
	{
		item_model = static_cast<QStandardItemModel*> ( completersList.at ( PRODUCT_OR_SERVICE )->model () );
		for ( int x ( 0 ); x < str_count; ++x )
		{
			item_model->insertRow ( x, QList<QStandardItem *> () <<
									new QStandardItem ( completer_strings.at ( x ) ) <<
									new QStandardItem ( completer_strings_2.at ( x ) ) );
		}
	}
}

void vmCompleters::setCompleter ( vmLineEdit* line, const COMPLETER_CATEGORIES type ) const
{
	if ( line != nullptr && type != COMPLETER_CATEGORIES::NONE )
		line->setCompleter ( completersList.at ( static_cast<int> ( type ) ) );
}

// type must be >= SUPPLIER. Rely on caller to reduce number of checks
void vmCompleters::updateCompleter ( const QString& str, const COMPLETER_CATEGORIES type )
{
	if ( str.isEmpty () || str == QStringLiteral ( "N/A" ) || type == COMPLETER_CATEGORIES::NONE )
		return;

	const QCompleter* completer ( completersList.at ( static_cast<int> ( type ) ) );
	QStandardItemModel* item_model ( static_cast<QStandardItemModel*> ( completer->model () ) );

	if ( inList ( str, type ) == -1 )
	{
		item_model->appendRow ( new QStandardItem ( str ) );
		QStandardItemModel* model_all ( static_cast<QStandardItemModel*> ( completersList.at ( ALL_CATEGORIES )->model () ) );
		model_all->appendRow ( new QStandardItem ( str ) );
		cr_rec->updateTable ( type, str );
	}
}

void vmCompleters::fillList ( const vmCompleters::COMPLETER_CATEGORIES type, QStringList& list ) const
{
	const QStandardItemModel* model ( static_cast<QStandardItemModel*> ( completersList.at ( static_cast<int> ( type ) )->model () ) );
	if ( model )
	{
		const uint n_items ( static_cast<uint> ( model->rowCount () ) );
		if ( n_items > 0 )
		{
			const QModelIndex index ( model->index ( 0, 0 ) );
			list.clear ();
			for ( uint i_row ( 0 ); i_row < n_items; ++i_row )
				static_cast<void>(Data::insertStringListItem ( list, model->data ( index.sibling ( static_cast<int>(i_row), 0 ) ).toString () ));
		}
	}
}

int vmCompleters::inList ( const QString& str, const vmCompleters::COMPLETER_CATEGORIES type ) const
{
	const QStandardItemModel* model ( static_cast<QStandardItemModel*> ( completersList.at ( static_cast<int> ( type ) )->model () ) );
	const QModelIndex index ( model->index ( 0, 0 ) );
	const int n_items ( model->rowCount () );
	for ( int i_row ( 0 ); i_row < n_items; ++i_row )
	{
		if ( str.compare ( model->data ( index.sibling ( i_row, 0 ) ).toString (), Qt::CaseInsensitive ) == 0 )
			return i_row;
	}
	return -1;
}

vmCompleters::COMPLETER_CATEGORIES vmCompleters::completerType ( QCompleter* completer, const QString& completion ) const
{
	vmCompleters::COMPLETER_CATEGORIES ret ( static_cast<vmCompleters::COMPLETER_CATEGORIES> ( completer->property ( "type" ).toInt () ) );
	if ( ret == vmCompleters::ALL_CATEGORIES )
	{
		if ( !completion.isEmpty () )
		{
			for ( uint i = 0; i <= vmCompleters::JOB_TYPE; ++i )
			{
				if ( i != static_cast<int> ( vmCompleters::ALL_CATEGORIES ) )
				{
					if ( inList ( completion, static_cast<vmCompleters::COMPLETER_CATEGORIES> ( i ) ) != -1 )
					{
						ret = static_cast<vmCompleters::COMPLETER_CATEGORIES> ( i );
						break;
					}
				}

			}
		}
	}
	return ret;
}

void vmCompleters::encodeCompleterISRForSpreadSheet ( const DBRecord* dbrec )
{
	if ( !dbrec->completerUpdated () )
	{
		stringRecord info;
		QStandardItemModel* model ( static_cast<QStandardItemModel*> ( completersList.at ( PRODUCT_OR_SERVICE )->model () ) );

		const QString compositItemName (	dbrec->isrValue ( ISR_NAME ) + QLatin1String ( " (" ) +
											dbrec->isrValue ( ISR_UNIT ) + QLatin1String ( ") " ) +
											dbrec->isrValue ( ISR_BRAND )
									   );

		for ( uint i ( ISR_NAME ); i <= ISR_DATE; ++i )
			info.fastAppendValue ( dbrec->isrValue ( static_cast<ITEMS_AND_SERVICE_RECORD> ( i ) ) );

		const int row ( inList ( compositItemName, PRODUCT_OR_SERVICE ) );
		if ( row == -1 )
		{
			model->appendRow ( new QStandardItem ( compositItemName ) );
			model->setItem ( model->rowCount () - 1, 1, new QStandardItem ( info.toString () ) );
		}
		else
		{
			const QModelIndex index ( model->index ( row, 1 ) );
			model->setData ( index, info.toString () );
		}
		cr_rec->setAction ( row == -1 ? ACTION_ADD : ACTION_EDIT );
		setRecValue ( cr_rec, FLD_CR_PRODUCT_OR_SERVICE_1, compositItemName );
		setRecValue ( cr_rec, FLD_CR_PRODUCT_OR_SERVICE_2, info.toString () );
		
		const_cast<DBRecord*> ( dbrec )->setCompleterUpdated ( cr_rec->saveRecord () );
	}
}
