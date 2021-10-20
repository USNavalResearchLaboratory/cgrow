#include "test_series.hpp"

const QString &test_series_t::id() const
{
    return id_;
}

void test_series_t::set_id(const QString &new_id)
{
    id_ = new_id;
}

QString test_series_t::name() const
{
    return QString("%1 R=%2").arg( id_ ).arg( data.R );
}

void test_series_t::read(const QJsonObject &json)
{
    if (!(json.contains("name") && json["name"].isString() ))
    {
        throw std::runtime_error("Json series does not contain a proper name field.");
    }
    else
    {
        id_ = json["name"].toString();
    }

    if (!(json.contains("marker") && json["marker"].isObject() ))
    {
        throw std::runtime_error("Json series does not contain a proper marker field.");
    }
    else
    {
        marker.read( json["marker"].toObject() );
    }

    if (!(json.contains("R") && json["R"].isDouble() ))
    {
        throw std::runtime_error("Json series does not contain a proper R field.");
    }
    else
    {
        data.R = json["R"].toDouble();

        if ( data.R < -1.0 || data.R > 1.0 )
        {
            throw std::runtime_error("Json series does not contain a proper R value.");
        }
    }


    QVector<double> newx;

    if (json.contains("DeltaK") && json["DeltaK"].isArray() )
    {

        auto DeltaKs_array = json["DeltaK"].toArray();
        for ( const auto &v : DeltaKs_array )
        {
            if ( v.isDouble())
            {
                newx.push_back( v.toDouble() );
            }
            else
            {
                throw std::runtime_error("Incorrect json format.");
            }
        }
    }
    else
    {
        throw std::runtime_error("Incorrect json format.");
    }

    QVector<double> newy;
    if (json.contains("dadN") && json["dadN"].isArray() )
    {

        auto DeltaKs_array = json["dadN"].toArray();
        for ( const auto &v : DeltaKs_array )
        {
            if ( v.isDouble())
            {
                newy.push_back( v.toDouble() );
            }
            else
            {
                throw std::runtime_error("Incorrect json format.");
            }
        }
    }
    else
    {
        throw std::runtime_error("Incorrect json format.");
    }

    if ( newx.size() != newy.size() )
    {
       throw std::runtime_error("Size of DeltaKs and dadNs do not match in the json file.");
    }

    data.points.clear();
    for ( auto i = 0; i != newx.size(); i++ )
    {
        data.points.push_back( { newx[i], newy[i] } );
    }
}

void test_series_t::write(QJsonObject &json) const
{
    QJsonArray DeltaKs_array;
    QJsonArray dadNs_array;

    for (const auto& v : data.points )
    {
        DeltaKs_array.append( v.DeltaK );
        dadNs_array.append( v.dadN );
    }

    json["name"] = id_;
    json["marker"] = marker.to_json_object();
    json["R"] = data.R;
    json["DeltaK"] = DeltaKs_array;
    json["dadN"] = dadNs_array;

}

QIcon test_series_t::generate_scatter_icon() const
{

    const int padding = 4;
    const int icon_size = marker.size() + padding;

    auto pen = QPen();
    pen.setColor( marker.color() );
    pen.setWidthF( marker.line_width() );

    const QColor background_color( 255, 255, 255,0 );

    QImage image(icon_size, icon_size, QImage::Format_ARGB32);
    image.fill( background_color );

    QCPPainter painter( &image );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(0,0, icon_size, icon_size, QBrush(QColor(255,255,255,0)));
    painter.setPen( pen );
    painter.setBrush(QBrush(QColor(255,255,255,0)));

    marker.get_QCPScatterStyle().drawShape( &painter, double(icon_size) / 2.0, double(icon_size) / 2.0 );

    return QIcon( QPixmap::fromImage( image ) );
}

QCPScatterStyle::ScatterShape test_series_t::indexToScatterShape( int index )
{
    switch ( index )
    {
    case  0: return QCPScatterStyle::ssCross;
    case  1: return QCPScatterStyle::ssPlus;
    case  2: return QCPScatterStyle::ssCircle;
    case  3: return QCPScatterStyle::ssDisc;
    case  4: return QCPScatterStyle::ssSquare;
    case  5: return QCPScatterStyle::ssDiamond;
    case  6: return QCPScatterStyle::ssTriangle;
    case  7: return QCPScatterStyle::ssTriangleInverted;
    case  8: return QCPScatterStyle::ssCrossSquare;
    case  9: return QCPScatterStyle::ssPlusSquare;
    case 10: return QCPScatterStyle::ssCrossCircle;
    case 11: return QCPScatterStyle::ssPlusCircle;
    case 12: return QCPScatterStyle::ssPeace;
    default:
        throw std::runtime_error("Invalid index when converting to ScatterShape");
    }
}

int test_series_t::ScatterShapeToIndex(QCPScatterStyle::ScatterShape ss)
{
    switch ( ss )
    {
    case QCPScatterStyle::ssCross: return 0;
    case QCPScatterStyle::ssPlus: return 1;
    case QCPScatterStyle::ssCircle: return 2;
    case QCPScatterStyle::ssDisc: return 3;
    case QCPScatterStyle::ssSquare: return 4;
    case QCPScatterStyle::ssDiamond: return 5;
    case QCPScatterStyle::ssTriangle: return 6;
    case QCPScatterStyle::ssTriangleInverted: return 7;
    case QCPScatterStyle::ssCrossSquare: return 8;
    case QCPScatterStyle::ssPlusSquare: return 9;
    case QCPScatterStyle::ssCrossCircle: return 10;
    case QCPScatterStyle::ssPlusCircle: return 11;
    case QCPScatterStyle::ssPeace: return 12;
    default:
        throw std::runtime_error("Invalid ScatterShape");
    }
}

QString test_series_t::ScatterShapeToString( QCPScatterStyle::ScatterShape ss )
{
    switch ( ss )
    {
    case QCPScatterStyle::ssDot: return "Dot";
    case QCPScatterStyle::ssCross: return "Cross";
    case QCPScatterStyle::ssPlus: return "Plus";
    case QCPScatterStyle::ssCircle: return "Circle";
    case QCPScatterStyle::ssDisc: return "Disc";
    case QCPScatterStyle::ssSquare: return "Square";
    case QCPScatterStyle::ssDiamond: return "Diamond";
    case QCPScatterStyle::ssTriangle: return "Triangle";
    case QCPScatterStyle::ssTriangleInverted: return "InvertedTriangle";
    case QCPScatterStyle::ssCrossSquare: return "Cross Square";
    case QCPScatterStyle::ssPlusSquare: return "Plus Square";
    case QCPScatterStyle::ssCrossCircle: return "Cross Circle";
    case QCPScatterStyle::ssPlusCircle: return "Plus Circle";
    case QCPScatterStyle::ssPeace: return "Peace";
    default:
        throw std::runtime_error("Invalid ScatterShape");
    }
}

QCPScatterStyle::ScatterShape test_series_t::stringToScatterShape( const QString &shape_string )
{
    if ( shape_string == "Dot") { return QCPScatterStyle::ssDot; }
    if ( shape_string == "Cross") { return QCPScatterStyle::ssCross; }
    if ( shape_string == "Plus") { return QCPScatterStyle::ssPlus; }
    if ( shape_string == "Circle") { return QCPScatterStyle::ssCircle; }
    if ( shape_string == "Disc") { return QCPScatterStyle::ssDisc; }
    if ( shape_string == "Square") { return QCPScatterStyle::ssSquare; }
    if ( shape_string == "Diamond") { return QCPScatterStyle::ssDiamond; }
    if ( shape_string == "Triangle") { return QCPScatterStyle::ssTriangle; }
    if ( shape_string == "InvertedTriangle") { return QCPScatterStyle::ssTriangleInverted; }
    if ( shape_string == "Cross Square") { return QCPScatterStyle::ssCrossSquare; }
    if ( shape_string == "Plus Square") { return QCPScatterStyle::ssPlusSquare; }
    if ( shape_string == "Cross Circle") { return QCPScatterStyle::ssCrossCircle; }
    if ( shape_string == "Plus Circle") { return QCPScatterStyle::ssPlusCircle; }
    if ( shape_string == "Peace") { return QCPScatterStyle::ssPeace; }
    if ( shape_string == "Dot") { return QCPScatterStyle::ssDot; }
    if ( shape_string == "Dot") { return QCPScatterStyle::ssDot; }

    throw std::runtime_error("Unkown scatter shape string.");
}

