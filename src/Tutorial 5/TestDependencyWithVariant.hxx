/*
 * TestDependencyWithVariant.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_TESTDEPENDENCYWITHVARIANT_HXX_
#define DI_TESTDEPENDENCYWITHVARIANT_HXX_


namespace test::di::visitor
{
    /**
     * Call within main as
     *
     * int main()
     * {
     *     return test::di::visitor::test();
     * }
     * @return Error indication, 0 - on success
     */
    int test();
}


#endif /* DI_TESTDEPENDENCYWITHVARIANT_HXX_ */
