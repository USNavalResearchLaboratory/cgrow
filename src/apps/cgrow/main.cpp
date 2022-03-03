#include <QApplication>

#include "mainwindow.hpp"

#include "test_series.hpp"

#include "fitting_worker.hpp"

void register_qt_meta_types()
{
    qRegisterMetaType<test_data_t>("test_data_t");
    qRegisterMetaType<std::vector<test_data_t>>("std::vector<test_data_t>");
    qRegisterMetaType<Hartman_Schijve_autoRange>("Hartman_Schijve_autoRange");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName(("U.S. Naval Research Lab"));
    QApplication::setApplicationName( ("CGROW - Hartman-Schijve"));

    register_qt_meta_types( );

    mainWindow window;
    window.show();

    return QApplication::exec();
}
