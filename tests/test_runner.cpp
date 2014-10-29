#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>

using namespace CPPUNIT_NS;

int main (int argc, char* argv[])
{
    // informs test-listener about testresults
    TestResult testresult;

    // register listener for collecting the test-results
    TestResultCollector collectedresults;
    testresult.addListener( &collectedresults );

    // register listener for per-test progress output
    BriefTestProgressListener progress;
    testresult.addListener( &progress );

    // insert test-suite at test-runner by registry
    TestRunner testrunner;
    testrunner.addTest( TestFactoryRegistry::getRegistry().makeTest() );
    testrunner.run( testresult );

    // output results in compiler-format
    CompilerOutputter compileroutputter( &collectedresults, std::cerr );
    compileroutputter.write();

    // return 0 if tests were successful
    return collectedresults.wasSuccessful() ? 0 : 1;
}
