#define CATCH_CONFIG_RUNNER
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QtWidgets/QApplication>

#include <context.h>
#include <fakeit.hpp>

#include <memory>
#include <log.h>

#define private public
#include <canrawview.h>
#include <gui/crvguiinterface.h>
#include <crvfactoryinterface.h>
#undef private

std::shared_ptr<spdlog::logger> kDefaultLogger;
using namespace fakeit;




TEST_CASE("Init table", "[canrawview]")
{

    
    
    Mock<CRVGuiInterface> crvMock;
    Mock<CRVFactoryInterface> crvFactoryMock;
    

    Fake(Dtor(crvMock));


    Fake(Method(crvMock, setClearCbk));
    Fake(Method(crvMock, setDockUndockCbk));
    Fake(Method(crvMock, setSectionClikedCbk));
    Fake(Method(crvMock, setFilterCbk));
    Fake(Method(crvMock, setModel));
    Fake(Method(crvMock, initTableView));


    When(Method(crvMock, isViewFrozen)).Return(false);
    Fake(Method(crvMock, scrollToBottom));
    Fake(Method(crvMock, getSortOrder));
    Fake(Method(crvMock, getClickedColumn));
    Fake(Method(crvMock, setSorting));
    Fake(Method(crvMock, getWindowTitle));
    Fake(Method(crvMock, isColumnHidden));

    CanRawView canRawView{ CanRawViewCtx(&crvMock.get()) };
    CHECK(canRawView.initTableView("", "") == false);
    
}



TEST_CASE("Start simulation", "[canrawview]")
{
    Mock<CRVGuiInterface> crvMock;
    Mock<CRVFactoryInterface> crvFactoryMock;

    Fake(Dtor(crvMock));

    Fake(Method(crvMock, setClearCbk));
    Fake(Method(crvMock, setDockUndockCbk));
    Fake(Method(crvMock, setSectionClikedCbk));
    Fake(Method(crvMock, setFilterCbk));
    Fake(Method(crvMock, setModel));
    Fake(Method(crvMock, initTableView));

    Fake(Method(crvMock, startSimulation));

    CanRawView canRawView{ CanRawViewCtx(&crvMock.get()) };
    CHECK(canRawView.d_ptr->_simStarted == true)
}

int main(int argc, char* argv[])
{
    bool haveDebug = std::getenv("CDS_DEBUG") != nullptr;
    kDefaultLogger = spdlog::stdout_color_mt("cds");
    if (haveDebug) {
        kDefaultLogger->set_level(spdlog::level::debug);
    }
    cds_debug("Staring canrawsender unit tests");
    QApplication a(argc, argv); // QApplication must exist when contructing QWidgets TODO check QTest
    return Catch::Session().run(argc, argv);
}
