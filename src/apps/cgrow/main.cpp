#include <QApplication>

#include "mainwindow.hpp"

#include "test_series.hpp"

void register_qt_meta_types()
{
    qRegisterMetaType<test_data_t>("test_data_t");
    qRegisterMetaType<std::vector<test_data_t>>("std::vector<test_data_t>");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName(("U.S. Naval Research Lab"));
    QApplication::setApplicationName( ("Hartman-Schijve fit"));

    register_qt_meta_types( );

    mainWindow window;
    window.show();

    return QApplication::exec();
}
