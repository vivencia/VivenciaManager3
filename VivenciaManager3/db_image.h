#ifndef DB_IMAGE_H
#define DB_IMAGE_H

#include "vmlist.h"
#include "global_enums.h"
#include "fileops.h"

#include <QFrame>

#include <functional>

class QLabel;
class QScrollArea;

class DB_Image : public QFrame
{

private:
	QLabel* imageViewer;
	QScrollArea* scrollArea;

	struct RECORD_IMAGES // OPT-2
	{ 
		PointersList<fileOps::st_fileInfo*> files;
		QString path;
		int rec_id;

		RECORD_IMAGES () : files ( 50 ), rec_id ( -1 ) { files.setAutoDeleteItem ( true ); }
	};

	bool findImagesByID ( const int rec_id );
	bool findImagesByPath ( const QString& path );
	void reloadInternal ( RECORD_IMAGES* ri, const QString& path );
	bool hookUpDir ( const int rec_id, const QString& path );

	void loadImage ( const QPixmap& );
	void loadImage ( const QString& );
	void scrollBy ( const int x, const int y );
	void adjustScrollBar ( const double factor );
	void scaleImage ( const double factor );

	QStringList name_filters;
	int mouse_ex, mouse_ey;
	bool mb_fitToWindow, mb_maximized;
	double scaleFactor;
	PointersList <RECORD_IMAGES*> images_array;

	std::function<void ( const int )> funcImageRequested;
	std::function<void ( const bool )> funcShowMaximized;

protected:
	bool eventFilter ( QObject* o, QEvent* e ) override;

public:
	explicit DB_Image ( QWidget* parent = nullptr );
	virtual ~DB_Image ();

	void showImage ( const int rec_id = -1, const QString& path = QString::null );

	const QString imageFileName () const;

	inline const QString imageCompletePath () const {
		return images_array.current () ? images_array.current ()->files.current ()->fullpath : QString::null;
	}
	inline const QString currentPath () const {
		return ( images_array.current () ? images_array.current ()->path : QString::null );
	}
	inline void setID ( const int id ) {
		if ( images_array.current () ) images_array.current ()->rec_id = id;
	}

	inline void setCallbackForshowImageRequested ( const std::function<void ( const int )>& func ) {
		funcImageRequested = func;
	}

	inline void setCallbackForshowMaximized ( const std::function<void ( const bool )>& func ) {
		funcShowMaximized = func;
	}

	QStringList imagesList () const;

	void showSpecificImage ( const int index );
	int showPrevImage ();
	int showNextImage ();
	int showFirstImage ();
	int showLastImage ();
	int rename ( const QString& newName );

	void zoomIn ();
	void zoomOut ();
	void normalSize ();
	void fitToWindow ();
	inline void setMaximized ( const bool maximized ) {
		mb_maximized = maximized;
	}

	void reload ( const QString& new_path = QString::null );
	void removeDir ( const bool b_deleteview = false );
};

#endif // DB_IMAGE_H
