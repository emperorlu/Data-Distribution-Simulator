/* 
 * File:   compare.h
 * Author: fermat
 *
 * Created on 14. April 2010, 13:47
 */

#ifndef _COMPARE_H
#define	_COMPARE_H

#ifdef	__cplusplus
extern "C" {
#endif

    void *runCompareTest(void *args);

    struct runCompareTestParms {
        int64_t firstID;
        int64_t numExtents;
        int32_t threadID;
    };


#ifdef	__cplusplus
}
#endif

#endif	/* _COMPARE_H */

