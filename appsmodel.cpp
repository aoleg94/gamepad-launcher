#include "appsmodel.h"
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>

QPixmap loadIconWin(const QString &sourceFile);
QPixmap loadIconGeneric(const QString &sourceFile)
{
    QPixmap p(sourceFile);
    if(p.isNull())
    {
        QList<QByteArray> l = QImageReader::supportedImageFormats();
        QString imageFilter = "Executable (*.exe);;Images (";
        for(const QByteArray& ba : l)
            imageFilter += " *." + QString::fromLatin1(ba);
        imageFilter += ")";
        QString fn = QFileDialog::getOpenFileName(nullptr, "Select icon", QString(), imageFilter);
        if(!fn.isEmpty())
            p = loadIconWin(fn);
        if(p.isNull())
            p.load(fn);
    }
    return p;
}

#ifdef _WIN32
#include <windows.h>
#include <QtWinExtras>
QPixmap loadIconWin(const QString &sourceFile)
{
    QPixmap p;

    const QString nativeName = QDir::toNativeSeparators(sourceFile);
    const wchar_t *sourceFileC = reinterpret_cast<const wchar_t *>(nativeName.utf16());
    if (!ExtractIconEx(sourceFileC, -1, 0, 0, 0)) {
        qWarning("%s does not appear to contain icons.", qPrintable(sourceFile));
        return QPixmap();
    }

    HICON icon;
    if (!ExtractIconEx(sourceFileC, 0, &icon, 0, 1)) {
        qErrnoWarning("Failed to extract icons from %s", qPrintable(sourceFile));
        return QPixmap();
    }

    return QtWin::fromHICON(icon);
}

QPixmap loadIconNoResize(const QString &sourceFile)
{
    if(sourceFile.isEmpty())
        return QPixmap();
    QPixmap p;
    if(!sourceFile.startsWith(':'))
        p = loadIconWin(sourceFile);
    if(p.isNull())
        p = loadIconGeneric(sourceFile);
    return p;
}

#else
QPixmap loadIconNoResize(const QString &sourceFile)
{
    return loadIconGeneric(sourceFile);
}
#endif
QPixmap loadIcon(const QString &sourceFile)
{
    return loadIconNoResize(sourceFile); //.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

AppsModel::AppsModel(QObject *parent) : QAbstractListModel(parent)
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings s;
    int size = s.beginReadArray("items");
    for (int i = 0; i < size; ++i)
    {
        s.setArrayIndex(i);
        AppItem ai;
        ai.name = s.value("name").toString();
        ai.iconpath = s.value("iconpath").toString();
        ai.icon = loadIcon(ai.iconpath.isEmpty() ? ":/no-icon.png" : ai.iconpath);
        ai.cmd  = s.value("cmd").toString();
        m_items.append(ai);
    }
    s.endArray();
}

int AppsModel::rowCount(const QModelIndex &) const
{
    return m_items.count();
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case Qt::DisplayRole:
        return m_items.at(index.row()).name;
    case Qt::DecorationRole:
        return m_items.at(index.row()).icon;
    case CmdRole:
        return m_items.at(index.row()).cmd;
    default:
        return QVariant();
    }
}

void AppsModel::update()
{
    beginResetModel();
    QSettings s;
    s.remove("items");
    s.beginWriteArray("items", m_items.size());
    for (int i = 0; i < m_items.size(); ++i)
    {
        s.setArrayIndex(i);
        const AppItem& ai = m_items.at(i);
        s.setValue("name", ai.name);
        s.setValue("iconpath", ai.iconpath);
        s.setValue("cmd", ai.cmd);
    }
    s.endArray();
    s.sync();
    endResetModel();
}


bool AppsModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if(count != 1)
        return false;
    if(!beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild))
        return false;

    AppItem ai = m_items.takeAt(sourceRow);
    m_items.insert(destinationChild, ai);

    endMoveRows();

    update();
    return true;
}

Qt::ItemFlags AppsModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}
