#include "test_series.hpp"
#include "test_series_dialog.hpp"
#include "spreadsheet.hpp"
#include "qcustomplot.h"

#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDoubleValidator>


testDataItemDialog::testDataItemDialog(int test_number, double defaultR, QWidget *parent ):
    QDialog( parent )
{
    test_series_.set_id( QString("Crack Growth %1").arg( test_number ) );

    name_line_edit = new QLineEdit( test_series_.id() );

    // Cycle through styles
    auto color = QColorDialog::standardColor( test_number  % 48  );
    test_series_.marker.set_color( color );
    test_series_.marker.set_shape( test_series_t::indexToScatterShape( test_number  % 13 ));

    test_series_.data.R = defaultR;

    connect( name_line_edit, &QLineEdit::textChanged, [this](const QString new_text )
    {
        test_series_.set_id( new_text );
    });

    marker_shape_combo_box = new QComboBox;

    update_combo_box();

    auto grid = new QGridLayout;

    int r = 0;
    grid->addWidget( new QLabel(tr("Name: ")), ++r,0,1,1 );
    grid->addWidget( name_line_edit, r,1,1,1 );
    connect( name_line_edit, &QLineEdit::textChanged, [this]( const QString &new_text )
    {
        test_series_.set_id( new_text );
    });

    grid->addWidget( new QLabel(tr("Symbol: ")), ++r,0,1,1 );
    grid->addWidget( marker_shape_combo_box, r,1,1,1 );

    connect( marker_shape_combo_box,
             qOverload<int>(&QComboBox::currentIndexChanged), [ this ](int newIndex)
    {
        test_series_.marker.set_shape( test_series_t::indexToScatterShape( newIndex ) );
        update_plot();
    });

    grid->addWidget( new QLabel(tr("Color: ") ), ++r,0,1,1 );
    auto colorPick = new QPushButton( tr("Change color") );
    grid->addWidget( colorPick, r, 1, 1, 1);

    connect( colorPick, &QPushButton::clicked, [this]()
    {
        QColorDialog color;
        color.exec();

        if ( color.result() == QDialog::Accepted )
        {
            test_series_.marker.set_color( color.selectedColor() );

            update_combo_box();

            update_plot();
        }
    });

    auto validator = new QDoubleValidator( this );
    validator->setRange(-1.0, 1.0, 3 );

    R_line_edit = new QLineEdit( QString("%1").arg(defaultR) );
    R_line_edit->setValidator( validator );
    connect( R_line_edit, &QLineEdit::textChanged, [this]( const QString &new_text )
    {
        test_series_.data.R = new_text.toDouble();
    });

    grid->addWidget( new QLabel(tr("R ratio: ")), ++r,0,1,1 );
    grid->addWidget( R_line_edit, r,1,1,1 );

    data_table = new spreadsheet;
    grid->addWidget( data_table, ++r, 0, 1, 2);

    data_table->setHorizontalHeaderLabels(QStringList() << "ΔK" << "da / dN");
    data_table->setMinimumWidth( 300 );

    grid->setColumnStretch( 1, 1 );
    grid->setColumnStretch( 2, 5 );

    {
        plot = new QCustomPlot;

        plot->setMinimumWidth( 300 );
        plot->yAxis2->setVisible(true);
        plot->xAxis2->setVisible(true);
        plot->xAxis2->setTicks(false);
        plot->yAxis2->setTicks(false);
        plot->xAxis2->setTickLabels(false);
        plot->yAxis2->setTickLabels(false);

        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        plot->yAxis->setTicker(logTicker);
        plot->yAxis->setScaleType(QCPAxis::stLogarithmic);

        plot->addGraph();
        QPen pen(Qt::red);
        pen.setWidthF( 1.5f );
        plot->graph(0)->setPen( pen );
        plot->graph(0)->setLineStyle( QCPGraph::LineStyle::lsNone );

        plot->xAxis->setLabel( "ΔK" );
        plot->yAxis->setLabel( "da / dN" );
    }

    grid->addWidget( plot, 0, 2, 6, 4 );

    connect(data_table, &spreadsheet::modified, [this]()
    {
        bool ok = false;

        test_series_.data.points = data_table->get_data( ok ).points;

        if ( ok )
        {
            update_plot();
        }

    });

    auto buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

    grid->addWidget( buttonBox, ++r, 0, 1,1 );

    this->setLayout( grid );
}

testDataItemDialog::testDataItemDialog(const test_series_t &test_data, QWidget *parent):
    testDataItemDialog(0, test_data.data.R, parent)
{
    test_series_ = test_data;

    name_line_edit->setText( test_series_.id() );

    R_line_edit->setText( QString("%1").arg( test_series_.data.R) );

    data_table->set_data( test_series_.data );

    update_combo_box();

    update_plot();
}

const test_series_t &testDataItemDialog::test_data() const
{
    return test_series_;
}


void testDataItemDialog::update_combo_box()
{
    marker_shape_combo_box->blockSignals( true );
    marker_shape_combo_box->clear();

    auto tmp_test_data = test_series_;

    for ( auto i = 0 ; i != 13; i ++ )
    {
        tmp_test_data.marker.set_shape( test_series_t::indexToScatterShape( i ) );

        auto shape = test_series_t::indexToScatterShape( i );
        auto text  = test_series_t::ScatterShapeToString( shape );

        auto icon = tmp_test_data.generate_scatter_icon();

        marker_shape_combo_box->addItem( icon, text );
    }

    marker_shape_combo_box->setCurrentIndex( test_series_t::ScatterShapeToIndex( test_series_.marker.shape()) );

    marker_shape_combo_box->blockSignals( false );
}

void testDataItemDialog::update_plot()
{
    const auto &data_points = test_series_.data.points;

    auto begin  = data_points.begin();
    auto end    = data_points.end();
    if ( begin == end )
    {
        return;
    }

    double minx = std::numeric_limits<double>::max();
    double maxx = std::numeric_limits<double>::lowest();
    double miny = std::numeric_limits<double>::max();
    double maxy = std::numeric_limits<double>::lowest();

    QVector<double> xs;
    QVector<double> ys;

    for ( const auto &data_point : data_points )
    {
        xs.push_back( data_point.DeltaK );
        ys.push_back( data_point.dadN);

        if ( data_point.DeltaK < minx ) minx = data_point.DeltaK;
        if ( data_point.DeltaK > maxx ) maxx = data_point.DeltaK;
        if ( data_point.dadN   < miny ) miny = data_point.dadN;
        if ( data_point.dadN   > maxy ) maxy = data_point.dadN;
    }
    const bool already_sorted = false;

    plot->graph(0)->setData( xs, ys , already_sorted );
    plot->graph(0)->setPen( test_series_.marker.get_QCPScatterStyle().pen());
    plot->graph(0)->setScatterStyle( test_series_.marker.get_QCPScatterStyle() );
    plot->xAxis->setRange( std::max( 0.0, minx - 1.0), 1.1*maxx);
    plot->yAxis->setRange( std::max( 1e-20, 0.5*miny), 2*maxy);

    plot->replot();
}
