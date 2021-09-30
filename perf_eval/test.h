/* 
 * File:   test.h
 * Author: fermat
 *
 * Created on 31. März 2010, 15:56
 */

#ifndef _TEST_H
#define	_TEST_H

#ifdef	__cplusplus
extern "C" {
#endif

    void *runTest(void *args);

    struct runTestParms {
        int64_t firstID;
        int64_t numExtents;
        int32_t threadID;
    };
#ifdef	__cplusplus
}
#endif

#endif	/* _TEST_H */

