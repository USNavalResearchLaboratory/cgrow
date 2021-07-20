#include "qcp_results_table.hpp"

qcp_results_table::qcp_results_table(QCustomPlot *plot)
{
    setVisible(false);

    plot->axisRect()->insetLayout()->addElement( this, Qt::AlignRight | Qt::AlignBottom );

    auto legend_layer = plot->layer("legend");

    plot->addLayer( QLatin1String("results background"), legend_layer, QCustomPlot::limAbove );
    this->setLayer( QLatin1String("results background") );

    plot->addLayer( QLatin1String("results"), this->layer(), QCustomPlot::limAbove );
    auto text_layer = plot->layer( QLatin1String("results") );

    set_border_pen( Qt::NoPen );
    set_brush( QColor( 255, 255, 255, 180) );

    D_          = new QCPTextElement(plot, "" );
    D_->setLayer( text_layer );

    p_          = new QCPTextElement(plot, "" );
    p_->setLayer( QLatin1String("results") );

    DeltaK_thr_ = new QCPTextElement(plot,"" );
    DeltaK_thr_->setLayer( text_layer );

    A_          = new QCPTextElement(plot, "" );
    A_->setLayer( text_layer );

    auto computed = new QCPTextElement(plot, tr("Computed:") );
    auto font = computed->font();
    font.setUnderline( true );
    computed->setFont( font );

    addElement( 0,0, computed );
    element(0,0)->setLayer( text_layer );

    addElement( 1,0, new QCPTextElement(plot, tr("D=") ) );
       element( 1,0)->setLayer( text_layer );
    addElement( 1,1, D_ );

    addElement(2,0,  new QCPTextElement(plot, tr("p=") ) );
    element( 2,0)->setLayer( text_layer );
    addElement(2,1,  p_ );

    addElement(3,0,  new QCPTextElement(plot, tr("ΔKₜₕᵣ=") ) );
    element( 3,0 )->setLayer( text_layer );
    addElement(3,1,  DeltaK_thr_ );

    addElement(4,0,  new QCPTextElement(plot, tr("A=") ) );
    element(4,0)->setLayer( text_layer );
    addElement(4,1,  A_ );

}

void qcp_results_table::set_visible(bool on)
{
    setVisible( on );
}

const QPen &qcp_results_table::border_pen( ) const
{
    return border_pen_;
}

void qcp_results_table::set_border_pen( const QPen pen )
{
    border_pen_ = pen;
}

const QBrush &qcp_results_table::brush( ) const
{
    return brush_;
}

void qcp_results_table::set_brush( const QBrush &brush )
{
    brush_ = brush;
}

const QFont &qcp_results_table::font() const
{
    return font_;
}

void qcp_results_table::set_font(const QFont font)
{
    font_ = font;
    for (int i=0; i != elementCount(); i++)
    {
      if (elementAt(i))
      {
          qobject_cast<QCPTextElement*>(elementAt(i))->setFont( font_ );
      }
    }
}

double qcp_results_table::D() const
{
    return D_->text().toDouble();
}

void qcp_results_table::set_D(const double &D)
{
    D_->setText(QString("%1").arg(D,8,'g',3));
}

double qcp_results_table::p() const
{
    return p_->text().toDouble();
}

void qcp_results_table::set_p(const double &p)
{
    p_->setText(QString("%1").arg(p,5,'f',3));
}

double qcp_results_table::DeltaK_thr() const
{
    return DeltaK_thr_->text().toDouble();
}

void qcp_results_table::set_DeltaK_thr(const double &DeltaK_thr)
{
    DeltaK_thr_->setText(QString("%1").arg(DeltaK_thr,5,'f',3));
}

double qcp_results_table::A() const
{
    return A_->text().toDouble();
}

void qcp_results_table::set_A(const double &A)
{
    A_->setText(QString("%1").arg(A,5,'f',3));
}

void qcp_results_table::draw( QCPPainter *painter )
{
    painter->setBrush( brush() );
    painter->setPen( border_pen() );
    painter->drawRect( mOuterRect );
}
