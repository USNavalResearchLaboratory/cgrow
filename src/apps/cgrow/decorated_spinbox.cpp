#include "decorated_spinbox.hpp"

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QLabel>

#include <QDebug>

decorated_double_spinbox::decorated_double_spinbox( const QString& name,
                                                    const double&  value,
                                                    const double&  lower,
                                                    const double&  upper,
                                                    const QPixmap& icon,
                                                    QWidget*       parent ) :

  QWidget( parent )
{
  auto grid = new QGridLayout;

  grid->setContentsMargins( 2, 2, 2, 2 );
  grid->setSpacing( 2 );
  grid->setMargin( 2 );

  int r = -1;
  {
    spinbox = new QDoubleSpinBox;
    spinbox->setValue( value );
    spinbox->setRange( lower, upper );
    spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    spinbox->setMaximumWidth( 50 );
    if ( !icon.isNull( ) )
    {
      grid->addWidget( spinbox, ++r, 1, 1, 1 );
    }
    else
    {
      grid->addWidget( spinbox, ++r, 0, 1, 2 );
    }

    connect( spinbox, SIGNAL( valueChanged( double ) ), this, SIGNAL( value_changed( double ) ) );
  }

  {
    if ( !icon.isNull( ) )
    {
      auto icon_label = new QLabel;
      icon_label->setPixmap( icon );
      grid->addWidget( icon_label, r, 0, 1, 1 );
    }
  }

  if ( !name.isEmpty( ) )
  {
    auto name_label = new QLabel( name );
    name_label->setAlignment( Qt::AlignCenter );

    grid->addWidget( name_label, ++r, 0, 1, 2 );
  }

  grid->addWidget( new QWidget );
  grid->setRowStretch( grid->rowCount( ) - 1, 1 );

  this->setLayout( grid );
}

void decorated_double_spinbox::set_single_step( double step )
{
  spinbox->setSingleStep( step );
}

decorated_int_spinbox::decorated_int_spinbox( const QString& name,
                                              const int&     value,
                                              const int&     lower,
                                              const int&     upper,
                                              const QPixmap& icon,
                                              QWidget*       parent ) :
  QWidget( parent )
{
  auto grid = new QGridLayout;

  grid->setContentsMargins( 2, 2, 2, 2 );
  grid->setSpacing( 2 );
  grid->setMargin( 2 );

  int r = -1;
  {
    spinbox = new QSpinBox;
    spinbox->setValue( value );
    spinbox->setRange( lower, upper );
    spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    spinbox->setMaximumWidth( 50 );
    if ( !icon.isNull( ) )
    {
      grid->addWidget( spinbox, ++r, 1, 1, 1 );
    }
    else
    {
      grid->addWidget( spinbox, ++r, 0, 1, 2 );
    }

    connect( spinbox, SIGNAL( valueChanged( int ) ), this, SIGNAL( value_changed( int ) ) );
  }

  {
    if ( !icon.isNull( ) )
    {
      auto icon_label = new QLabel;
      icon_label->setPixmap( icon );
      grid->addWidget( icon_label, r, 0, 1, 1 );
    }
  }

  if ( !name.isEmpty( ) )
  {
    auto name_label = new QLabel( name );
    name_label->setAlignment( Qt::AlignCenter );

    grid->addWidget( name_label, ++r, 0, 1, 2 );
  }

  grid->addWidget( new QWidget );
  grid->setRowStretch( grid->rowCount( ) - 1, 1 );

  this->setLayout( grid );
}
