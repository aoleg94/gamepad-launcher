#ifndef APPSMODEL_H
#define APPSMODEL_H

#include <QAbstractListModel>
#include <QPixmap>

struct AppItem {
    QString name;
    QPixmap icon;
    QString iconpath;
    QString cmd;
};

QPixmap loadIcon(const QString &sourceFile);

#define CmdRole Qt::UserRole+1

class AppsModel : public QAbstractListModel
{
    Q_OBJECT
    QList<AppItem> m_items;
public:
    explicit AppsModel(QObject* parent = nullptr);

    // QAbstractItemModel interface
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void update();

    QList<AppItem>& items() { return m_items; }
    const QList<AppItem>& items() const { return m_items; }
};

#endif // APPSMODEL_H
