#include "marker.hpp"


QString ScatterShapeToString( const QCPScatterStyle::ScatterShape &ss )
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

QCPScatterStyle::ScatterShape stringToScatterShape( const QString &shape_string )
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

marker_t::marker_t()
{
    scatter_style_.setShape( QCPScatterStyle::ScatterShape::ssCircle );
    scatter_style_.setSize( 1.5 * 6.0 );

    QPen pen;
    pen.setColor( Qt::black );
    pen.setWidthF( 1.5 );

    scatter_style_.setPen( pen );
}

marker_t::marker_t(const QJsonObject &json_object)
{
    read( json_object );
}

QCPScatterStyle::ScatterShape marker_t::shape() const
{
    return scatter_style_.shape();
}

void marker_t::set_shape(const QCPScatterStyle::ScatterShape &shape)
{
    scatter_style_.setShape( shape );
}

void marker_t::set_shape(const QString &shape_name)
{
    set_shape( stringToScatterShape( shape_name ) );
}

qreal marker_t::line_width() const
{
    return scatter_style_.pen().widthF();
}

void marker_t::set_line_width(const qreal &w )
{
    auto pen = scatter_style_.pen();
    pen.setWidthF( w );

    scatter_style_.setPen( pen );
}

double marker_t::size( ) const
{
    return scatter_style_.size();
}

void marker_t::set_size( const int &s )
{
    scatter_style_.setSize( s );
}

QColor marker_t::color() const
{
    return scatter_style_.pen().color();
}

void marker_t::set_color(const QColor &c)
{
    auto pen = scatter_style_.pen();
    pen.setColor( c );

    scatter_style_.setPen( pen );
}

const QCPScatterStyle &marker_t::get_QCPScatterStyle() const
{
    return scatter_style_;
}

QJsonObject marker_t::to_json_object() const
{
    auto pen = scatter_style_.pen();
    auto color = pen.color();

    QJsonObject color_object;
    color_object["red"] = color.red();
    color_object["green"] = color.green();
    color_object["blue"] = color.blue();

    QJsonObject json_object;
    json_object["shape"] = ScatterShapeToString( scatter_style_.shape() );
    json_object["color"] = color_object;
    json_object["line width"] = pen.widthF();
    json_object["size"] = scatter_style_.size();

    return json_object;

}

void marker_t::read(const QJsonObject &json_object)
{
    if (!(json_object.contains("shape") && json_object["shape"].isString()))
    {
        throw std::runtime_error("Incorrect shape designation in json.");
    }

    set_shape( json_object["shape"].toString() );

    if (!(json_object.contains("color") && json_object["color"].isObject()))
    {
        throw std::runtime_error("Incorrect color designation in json.");
    }

    auto color_object = json_object["color"].toObject();

    auto get_color_component =[ &color_object ](const QString &key)
    {
        int c = 0;
        if (!( color_object.contains(key) && color_object[key].isDouble() ) )
        {
            throw std::runtime_error("Incorrect color component designation in json.");
        }
        else
        {
            c = color_object[key].toDouble();
            if ( c<0 || c > 255)
            {
                throw std::runtime_error("Incorrect color compenent value in json.");
            }
        }

        return c;
    };

    auto r = get_color_component("red");
    auto g = get_color_component("green");
    auto b = get_color_component("blue");

    set_color( QColor(r, g, b) );

    if (!(json_object.contains("line width") && json_object["line width"].isDouble() ))
    {
        throw std::runtime_error("Incorrect line width in json.");
    }

    if (json_object["line width"].toDouble() < 0.0 )
    {
        throw std::runtime_error("Incorrect line width designation in json.");
    }
    set_line_width( json_object["line width"].toDouble() );


    if (!(json_object.contains("size") && json_object["size"].isDouble() ))
    {
        throw std::runtime_error("Incorrect size designation in json.");
    }

    set_size( json_object["size"].toDouble() );
}
