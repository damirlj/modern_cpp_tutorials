/*
 * TestDependencyInjection.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_TESTDEPENDENCYINJECTION_HXX_
#define DI_TESTDEPENDENCYINJECTION_HXX_

namespace test::di
{
    /**
     * Call within main as
     *
     * int main()
     * {
     *     return test::di::testDI();
     * }
     * @return Error indication, 0 - on success
     */
    int testDI();
}



#endif /* DI_TESTDEPENDENCYINJECTION_HXX_ */
