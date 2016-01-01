#include <QProcess>
#include <QLabel>
#include <QScrollBar>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollArea>

#include "global.h"
#include "db_image.h"
#include "fileops.h"
#include "data.h"

static QStringList emptyQStringList;
static const QDir::Filters filter ( QDir::AllDirs | QDir::Files | QDir::Readable | QDir::NoSymLinks | QDir::NoDotAndDotDot );

inline static bool operator> ( const QSize& size1, const QSize& size2 )
{
    return ( size1.width () > size2.width () ) | ( size2.height () > size1.height () );
}

DB_Image::DB_Image ( QWidget* parent )
	: QFrame ( parent ), imageViewer ( nullptr ), mouse_ex ( 0 ), mouse_ey ( 0 ),
	  mb_fitToWindow ( true ), mb_maximized ( false ), scaleFactor ( 1.0 ), images_array ( 50 ),
	  funcImageRequested ( nullptr ), funcShowMaximized ( nullptr )
{
	imageViewer = new QLabel;
	imageViewer->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding ); //( QSizePolicy::Ignored, QSizePolicy::Ignored );
	imageViewer->setScaledContents ( true );
	imageViewer->setFocusPolicy ( Qt::WheelFocus );
	imageViewer->setToolTip ( "Zoom in (+)\nZoom out (-)\nToggle on/off Fit to Window (F)" );

	scrollArea = new QScrollArea ( this );
	scrollArea->setBackgroundRole ( QPalette::Dark );
	scrollArea->setWidget ( imageViewer );
	scrollArea->setMouseTracking ( true );
	scrollArea->setGeometry ( 0, 0, width (), height () );

	installEventFilter ( imageViewer );
	installEventFilter ( this );

	name_filters.insert ( 0, QStringLiteral ( "*.jpg" ) );
	name_filters.insert ( 1, QStringLiteral ( "*.JPG" ) );
	name_filters.insert ( 2, QStringLiteral ( "*.png" ) );
	name_filters.insert ( 4, QStringLiteral ( "*.PNG" ) );
}

DB_Image::~DB_Image ()
{
	images_array.clear ( true );
}

void DB_Image::showImage ( const int rec_id, const QString& path )
{
	bool b_exists ( false );
	if ( rec_id != -1 ) {
		if ( ( b_exists = findImagesByID ( rec_id ) ) ) {
			if ( images_array.current ()->path != path )
				removeDir (); // in editing, the image path changed
		}
	}
	else if ( !path.isEmpty () )
		b_exists = findImagesByPath ( path );
	if ( !b_exists ) {
		if ( !hookUpDir ( rec_id, path ) )
			images_array.setCurrent ( -1 );
	}
	showFirstImage ();
}

void DB_Image::showImageInternal ( const QString& filename )
{
	QPixmap pixmap;
	if ( filename.isEmpty () ) {
		pixmap.load ( QStringLiteral ( ":/resources/no_image.jpg" ) );
		mstr_FileName = emptyString;
	}
	else {
		mstr_FileName = images_array.current ()->path + filename;
		mouse_ex = mouse_ey = 0;
		pixmap.load ( mstr_FileName );
	}
	showImageInternal ( pixmap );
}

void DB_Image::showImageInternal ( const QPixmap& pixmap )
{
	const bool need_resize ( imageViewer->pixmap () != nullptr ?
							 pixmap.size () != imageViewer->pixmap ()->size () :
							 true
						   );

	imageViewer->setPixmap ( pixmap );
	if ( need_resize ) {
		if ( mb_fitToWindow )
			fitToWindow ();
		else
			normalSize ();
	}
}

bool DB_Image::findImagesByID ( const int rec_id )
{
	for ( uint i ( 0 ); i < images_array.count (); ++i ) {
		if ( static_cast<RECORD_IMAGES*> ( images_array.at ( i ) )->rec_id == rec_id ) {
			images_array.setCurrent ( i );
			mb_fitToWindow = true;
			return true;
		}
	}
	return false;
}

bool DB_Image::findImagesByPath ( const QString& path )
{
	for ( uint i ( 0 ); i < images_array.count (); ++i ) {
		if ( static_cast<RECORD_IMAGES*> ( images_array.at ( i ) )->path == path ) {
			images_array.setCurrent ( i );
			mb_fitToWindow = true;
			return true;
		}
	}
	return false;
}

void DB_Image::reloadInternal ( RECORD_IMAGES* ri, const QString& path )
{
    if ( fileOps::isDir ( path ).isOn () ) {
		QDir dir ( path );
		ri->files = dir.entryList ( name_filters, QDir::Files, QDir::Name );
	}
}

bool DB_Image::hookUpDir ( const int rec_id, const QString& path )
{
    if ( !fileOps::exists ( path ).isOn () )
		return false;

	RECORD_IMAGES* ri ( new RECORD_IMAGES );
	ri->rec_id = ( rec_id != -1 ) ? rec_id : -2;
	ri->path = path;
	reloadInternal ( ri, path );
	images_array.setCurrent ( images_array.count () );
	images_array.append ( ri );
	return true;
}

void DB_Image::reload ()
{
	if ( images_array.current () ) {
		images_array.current ()->files.clear ();
		reloadInternal ( static_cast<RECORD_IMAGES*> ( images_array.current () ),
						 static_cast<RECORD_IMAGES*> ( images_array.current () )->path );
		showFirstImage ();
	}
}

// called when deleting a record, when changing the dir path of a record or when changing a dir path for a single image
void DB_Image::removeDir ( const bool b_deleteview )
{
	if ( images_array.current () ) {
		images_array.current ()->files.clear ();
		if ( b_deleteview )
			images_array.remove ( images_array.currentIndex (), true );
	}
}

const QStringList& DB_Image::imagesList () const
{
	if ( images_array.currentIndex () >= 0 )
		return images_array.current ()->files;
	return emptyQStringList;
}

// Only one caller for this function and it uses the same QStringList we use. No need to check for the validity of index
// Note: sometimes, when least expecting it, if I choose a random pic from the combo the program will abort for index will
// be out of range. The rude thing is that it is not reproductible and if I choose the same picture in another session, it will
// be displayed correctly.
void DB_Image::showSpecificImage ( const int index )
{
	if ( index >= 0 && index < images_array.current ()->files.count () ) {
		if ( images_array.currentIndex () >= 0 ) {
			images_array.current ()->cur_image = index;
			if ( images_array.current ()->files.count () > index ) {
				showImageInternal ( images_array.current ()->files.at ( index ) );
				return;
			}
		}
	}
	showImageInternal ( emptyString );
}

bool DB_Image::showFirstImage ()
{
	if ( images_array.currentIndex () >= 0 ) {
		images_array.current ()->cur_image = 0;
		if ( images_array.current ()->files.count () > 0 ) {
			showImageInternal ( images_array.current ()->files.at ( 0 ) );
			funcImageRequested ( 0 ); // do not check if not nullptr. In this app, it will be set
			return true;
		}
	}
	showImageInternal ( emptyString );
	return false;
}

bool DB_Image::showLastImage ()
{
	if ( images_array.currentIndex () >= 0 ) {
		const int last_idx ( images_array.current ()->files.count () - 1 );
		images_array.current ()->cur_image = last_idx;
		if ( last_idx >= 0 ) {
			showImageInternal ( images_array.current ()->files.at ( last_idx ) );
			funcImageRequested ( last_idx );
			return true;
		}
	}
	showImageInternal ( emptyString );
	return false;
}

int DB_Image::showPrevImage ()
{
	if ( images_array.currentIndex () >= 0 ) {
		RECORD_IMAGES* ri ( images_array.current () );
		if ( ri->cur_image >= 1 ) {
			--( ri->cur_image );
			if ( images_array.current ()->files.count () > ri->cur_image ) {
				showImageInternal ( ri->files.at ( ri->cur_image ) );
				funcImageRequested ( ri->cur_image );
				return ri->cur_image;
			}
		}
	}
	showImageInternal ( emptyString );
	return -1;
}

int DB_Image::showNextImage ()
{
	if ( images_array.currentIndex () >= 0 ) {
		RECORD_IMAGES *ri ( images_array.current () );
		if ( ri->cur_image < ( ri->files.count () - 1 ) ) {
			++( ri->cur_image );
			if ( images_array.current ()->files.count () > ri->cur_image ) {
				showImageInternal ( ri->files.at ( ri->cur_image ) );
				funcImageRequested ( ri->cur_image );
				return ri->cur_image;
			}
		}
	}
	showImageInternal ( emptyString );
	return -1;
}

int DB_Image::rename ( const QString& newName )
{
	if ( images_array.currentIndex () >= 0 ) {
		if ( newName != images_array.current ()->files.at ( images_array.current ()->cur_image ) ) {
			QString new_fileName ( fileOps::dirFromPath ( mstr_FileName ) + newName );
			const QString ext ( CHR_DOT + fileOps::fileExtension ( mstr_FileName ) );
			if ( !newName.endsWith ( ext ) )
				new_fileName.append ( ext );

            if ( fileOps::rename ( mstr_FileName, new_fileName ).isOn () ) {
				mstr_FileName = new_fileName;
				images_array.current ()->files.removeAt ( images_array.current ()->cur_image );
				images_array.current ()->cur_image =
					Data::insertStringListItem ( images_array.current ()->files, newName );
				return images_array.current ()->cur_image;
			}
		}
	}
	return -1;
}

void DB_Image::scrollBy ( const int x, const int y )
{
	QScrollBar* h ( scrollArea->horizontalScrollBar () );
	QScrollBar* v ( scrollArea->verticalScrollBar () );
	h->setValue ( h->value() + x );
	v->setValue ( v->value() + y );
}

void DB_Image::scaleImage ( const float factor )
{
	scaleFactor *= factor;
	int img_width ( scaleFactor * imageViewer->pixmap ()->size ().width () );
	int img_height ( scaleFactor * imageViewer->pixmap ()->size ().height () );
	imageViewer->resize ( img_width, img_height );
	scrollArea->setGeometry ( ( ( width () - img_width ) / 2 ),
							  ( ( height () - img_height ) / 2 ), qMin ( width () , img_width ), qMax ( height (), img_height )
							);
	// Try to avoid scrollbars
	if ( ( img_width + 10 ) > scrollArea->width () )
		img_width = scrollArea->width () - 10;
	if ( ( img_height + 10 ) > scrollArea->height () )
		img_height = scrollArea->height () - 10;
	imageViewer->resize ( img_width, img_height );

	//if ( ( img_width > scrollArea->width () ) || ( img_height > scrollArea->height () ) )
	//adjustScrollBar ( factor );

}

void DB_Image::adjustScrollBar ( const float factor )
{
	QScrollBar* h = scrollArea->horizontalScrollBar ();
	QScrollBar* v = scrollArea->verticalScrollBar ();
	h->setValue ( static_cast<int> ( factor * h->value () + ( ( factor - 1 ) * h->pageStep () / 2 ) ) );
	v->setValue ( static_cast<int> ( factor * v->value () + ( ( factor - 1 ) * v->pageStep () / 2 ) ) );
}

void DB_Image::zoomIn ()
{
	if ( !scrollArea->widgetResizable () )
		scaleImage ( 1.10 );
}

void DB_Image::zoomOut ()
{
	if ( !scrollArea->widgetResizable () )
		scaleImage ( 0.9 );
}

void DB_Image::normalSize ()
{
	scaleFactor = 1.0;
	imageViewer->adjustSize ();
	if ( imageViewer->pixmap ()->size () > size () ) {
		scrollArea->setGeometry ( 0, 0, width (), height () );
		adjustScrollBar ( 1.0 );
	}
	else
		scrollArea->setGeometry ( ( width () - imageViewer->width () ) / 2,
								  ( height () - imageViewer->height () ) / 2,
								  qMin ( width (), imageViewer->width () ),
								  qMin ( height (), imageViewer->height () )
								);
	//if ( ( imageViewer->width () > scrollArea->width () ) || ( imageViewer->height () > scrollArea->height () ) )
	//	adjustScrollBar ( 1.0 );

}

void DB_Image::fitToWindow ()
{
	scaleFactor = 1.0;
	float factor ( 1.0 );
	const float imgWidth ( static_cast<float> ( imageViewer->pixmap ()->size ().width () ) );
	const float imgHeight ( static_cast<float> ( imageViewer->pixmap ()->size ().height () ) );
	const float f_width ( static_cast<float> ( this->width () ) );
	const float f_height ( static_cast<float> ( this->height () ) );

	if ( imgWidth > f_width )
		factor = f_width / imgWidth;

	if ( imgHeight > f_height ) {
		const float factor2 ( f_height / imgHeight );
		if ( factor2 < factor )
			factor = factor2;
	}

	scaleImage ( factor );
}

bool DB_Image::eventFilter ( QObject* o, QEvent* e )
{
	switch ( e->type () ) {
		default:
			return false;
		case QEvent::Resize:
			if ( o == this ) {
				scrollArea->setGeometry ( ( width () - imageViewer->width () ) / 2,
									  ( height () - imageViewer->height () ) / 2, imageViewer->width (),
									  imageViewer->height ()
									);
				if ( mb_fitToWindow )
					fitToWindow ();
				else
					normalSize ();
			}
			break;
		case QEvent::KeyPress:
		{
			const QKeyEvent* k ( static_cast<QKeyEvent*> ( e ) );
			switch  ( k->key () ) {
				default:
					return false;
				case Qt::Key_F:
					mb_fitToWindow = !mb_fitToWindow;
					if ( mb_fitToWindow )
						fitToWindow ();
					else
						normalSize ();
				break;
				case Qt::Key_Minus:
					zoomOut ();
				break;
				case Qt::Key_Plus:
				case Qt::Key_Equal:
					zoomIn ();
				break;
				case Qt::Key_A:
				case Qt::Key_S:
					showPrevImage ();
				break;
				case Qt::Key_D:
				case Qt::Key_W:
					showNextImage ();
				break;
				case Qt::Key_Home:
					showFirstImage ();
				break;
				case Qt::Key_End:
					showLastImage ();
				break;
			}
		}
		break;
		case  QEvent::MouseButtonPress:
			if ( images_array.currentIndex () >= 0 ) {
				const QMouseEvent* m ( static_cast<QMouseEvent*> ( e ) );
				if ( m->button () == Qt::LeftButton )  {
					setCursor ( Qt::ClosedHandCursor );
					mouse_ex = m->x ();
					mouse_ey = m->y ();
				}
			}
			break;
		case QEvent::MouseButtonRelease:
			setCursor ( Qt::ArrowCursor );
		break;
		case QEvent::MouseButtonDblClick:
			funcShowMaximized ( mb_maximized = !mb_maximized );
		break;
		case QEvent::MouseMove:
			if ( images_array.currentIndex () >= 0 ) {
				const QMouseEvent* m ( static_cast<QMouseEvent*> ( e ) );
				if ( m->buttons () & Qt::LeftButton ) {
					const int mx = m->x ();
					const int my = m->y ();
					scrollBy ( mouse_ex - mx,mouse_ey - my );
					mouse_ex = mx;
					mouse_ey = my;
				}
			}
		break;
	}
	e->accept ();
	return true;
}
