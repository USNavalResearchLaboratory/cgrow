#include "spreadsheet.hpp"

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QKeyEvent>

#include <QDebug>

Cell::Cell()
{

}

QTableWidgetItem *Cell::clone() const
{
    return new Cell(*this);
}

void Cell::setData(int role, const QVariant &value)
{
    QTableWidgetItem::setData(role, value);
}

QVariant Cell::data(int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (value().isValid())
        {
            return value().toString();
        }
        else
        {
            return "####";
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if (value().type() == QVariant::String)
        {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else
        {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    else
    {
        return QTableWidgetItem::data(role);
    }
}

void Cell::setFormula(const QString &formula)
{
    setData(Qt::EditRole, formula);
}

QString Cell::formula() const
{
    return data(Qt::EditRole).toString();
}

//const QVariant Invalid;
QVariant Cell::value() const
{
    return formula();
}

spreadsheet::spreadsheet( QWidget *parent )
    : QTableWidget(parent)
{
    setItemPrototype(new Cell);

    setSelectionMode(ContiguousSelection);

    connect(this, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(somethingChanged()));

    clear();
}

QString spreadsheet::currentLocation() const
{
    return QChar('A' + currentColumn())
            + QString::number(currentRow() + 1);
}

QTableWidgetSelectionRange spreadsheet::selectedRange() const
{
    QList<QTableWidgetSelectionRange> ranges = selectedRanges();

    if (ranges.isEmpty())
    {
        return QTableWidgetSelectionRange();
    }

    return ranges.first();
}

void spreadsheet::clear()
{
    setRowCount(0);

    setColumnCount(0);

    setRowCount(RowCount);

    setColumnCount(ColumnCount);

    for (int i = 0; i < ColumnCount; ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem;

        item->setText(QString(QChar('A' + i)));

        setHorizontalHeaderItem(i, item);
    }

    setCurrentCell(0, 0);
}

void spreadsheet::set_data( const test_data_t &data)
{
    const auto &data_points = data.points;

    if ( data_points.size() > RowCount )
    {
        throw std::out_of_range("Too many data points.");
    }

    this->blockSignals( true );

    for ( std::size_t i = 0; i != data_points.size(); i++ )
    {
        setFormula( i, 0, QString::number( data_points[i].DeltaK ));
        setFormula( i, 1, QString::number( data_points[i].dadN ));
    }

    this->blockSignals( false );
}

test_data_t spreadsheet::get_data( bool &ok ) const
{
    test_data_t ret;

    for ( auto i = 0; i != RowCount; i++ )
    {
        auto x = formula( i, 0 );
        auto y = formula( i, 1 );

        if ( !x.isEmpty( ) )
        {
            bool xconvertedOk = false;
            double DeltaK_v = x.toDouble( &xconvertedOk );

            if ( xconvertedOk )
            {
                if ( y.isEmpty( ))
                {
                    ok = false;
                    return ret;
                }
                bool yconvertedOk = false;

                double dadN_v = y.toDouble( &yconvertedOk );

                if ( yconvertedOk )
                {
                    ret.points.push_back( {DeltaK_v, dadN_v } );
                }
                else
                {
                    ok = false;
                    return ret;
                }
            }
            else
            {
                ok = false;
                return ret;
            }
        }
        else if ( !y.isEmpty( ) )
        {
            ok = false;
            return ret;
        }
    }

    ok = true;
    return ret;
}

//bool spreadsheet::get_data(QVector<double> &DeltaK, QVector<double> &dadN) const
//{
//    for ( auto i = 0; i != RowCount; i++ )
//    {
//        auto x = formula( i, 0 );
//        auto y = formula( i, 1 );

//        if ( !x.isEmpty( ) )
//        {
//            bool xconvertedOk = false;
//            double DeltaK_v = x.toDouble( &xconvertedOk );

//            if ( xconvertedOk )
//            {
//                if ( y.isEmpty( ))
//                {
//                    return false;
//                }
//                bool yconvertedOk = false;

//                double dadN_v = y.toDouble( &yconvertedOk );

//                if ( yconvertedOk )
//                {
//                    DeltaK.push_back( DeltaK_v);
//                    dadN.push_back( dadN_v);
//                }
//                else
//                {
//                    return false;
//                }
//            }
//            else
//            {
//                return false;
//            }
//        }
//        else if ( !y.isEmpty( ) )
//        {
//            return false;
//        }
//    }

//    return true;
//}

void spreadsheet::keyPressEvent(QKeyEvent *event)
{
    if ( event->matches(QKeySequence::Copy) )
    {
        copy();
    }
    else if ( event->matches(QKeySequence::Paste) )
    {
        paste();
    }
    else if ( event->matches(QKeySequence::Delete) )
    {
        del();
    }
    else if ( event->matches(QKeySequence::Cut) )
    {
        cut();
    }
    else
    {
        QTableView::keyPressEvent(event);
    }
}


void spreadsheet::cut()
{
    copy();

    del();
}

void spreadsheet::copy()
{
    QTableWidgetSelectionRange range = selectedRange();

    QString str;

    for (int i = 0; i < range.rowCount(); ++i)
    {
        if (i > 0)
        {
            str += "\n";
        }

        for (int j = 0; j < range.columnCount(); ++j)
        {
            if (j > 0)
            {
                str += "\t";
            }

            str += formula(range.topRow() + i, range.leftColumn() + j);
        }
    }

    QApplication::clipboard()->setText(str);
}

void spreadsheet::paste()
{
    QTableWidgetSelectionRange range = selectedRange();

    QString str = QApplication::clipboard()->text();

    QStringList rows = str.split('\n');

    int numRows = rows.count();

    int numColumns = rows.first().count('\t') + 1;


    if (range.rowCount() * range.columnCount() != 1
            && (range.rowCount() != numRows
                || range.columnCount() != numColumns))
    {
        QMessageBox::information(this, tr("Spreadsheet"),
                                 tr("The information cannot be pasted because the copy "
                                    "and paste areas aren't the same size."));
        return;
    }

    for (int i = 0; i < numRows; ++i)
    {
        if ( !rows[i].isEmpty() )
        {
            QStringList columns = rows[i].split('\t');

            for (int j = 0; j < numColumns; ++j)
            {
                int row = range.topRow() + i;

                int column = range.leftColumn() + j;

                if (row < RowCount && column < ColumnCount)
                {
                    setFormula(row, column, columns[j]);
                }
            }
        }
    }
    somethingChanged();
}

void spreadsheet::del()
{
    QList<QTableWidgetItem *> items = selectedItems();
    if (!items.isEmpty()) {
        foreach (QTableWidgetItem *item, items)
            delete item;
        somethingChanged();
    }
}

void spreadsheet::selectCurrentRow()
{
    selectRow(currentRow());
}

void spreadsheet::selectCurrentColumn()
{
    selectColumn(currentColumn());
}

void spreadsheet::findNext(const QString &str, Qt::CaseSensitivity cs)
{
    int row = currentRow();

    int column = currentColumn() + 1;

    while (row < RowCount)
    {
        while (column < ColumnCount)
        {
            if (text(row, column).contains(str, cs))
            {
                clearSelection();
                setCurrentCell(row, column);
                activateWindow();
                return;
            }
            ++column;
        }
        column = 0;
        ++row;
    }

    QApplication::beep();
}

void spreadsheet::findPrevious(const QString &str,
                               Qt::CaseSensitivity cs)
{
    int row = currentRow();

    int column = currentColumn() - 1;

    while (row >= 0)
    {
        while (column >= 0)
        {
            if (text(row, column).contains(str, cs))
            {
                clearSelection();
                setCurrentCell(row, column);
                activateWindow();
                return;
            }
            --column;
        }
        column = ColumnCount - 1;
        --row;
    }

    QApplication::beep();
}

void spreadsheet::somethingChanged()
{
    qDebug() << "something chagned";
    emit modified();
}

Cell *spreadsheet::cell(int row, int column) const
{
    return static_cast<Cell *>(item(row, column));
}

QString spreadsheet::text(int row, int column) const
{
    Cell *c = cell(row, column);
    if (c)
    {
        return c->text();
    }
    else
    {
        return "";
    }
}

void spreadsheet::setFormula(int row, int column,
                             const QString &formula)
{
    Cell *c = cell(row, column);
    if (!c)
    {
        c = new Cell;
        setItem(row, column, c);
    }
    c->setFormula(formula);
}

QString spreadsheet::formula(int row, int column) const
{
    Cell *c = cell(row, column);
    if (c)
    {
        return c->formula();
    }
    else
    {
        return "";
    }
}
