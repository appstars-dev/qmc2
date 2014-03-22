#ifndef _FILEICONPROVIDER_H_
#define _FILEICONPROVIDER_H_

#include <QIcon>
#include <QCache>
#include <QFileIconProvider>

class FileIconProvider
{
	public:
		static FileIconProvider *instance();
		static QIcon fileIcon(const QString &filename);
		static QIcon folderIcon();
		static QIcon defaultFileIcon();

		FileIconProvider();

	private:
		static FileIconProvider *self;
		QCache<QString, QIcon> iconCache;
		QFileIconProvider iconProvider;
};

#endif
