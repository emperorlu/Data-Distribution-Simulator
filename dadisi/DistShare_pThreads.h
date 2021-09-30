/* 
 * File:   DistShare_pThreads.h
 * Author: fermat
 *
 * Created on 9. April 2010, 08:00
 */

#ifndef _DISTSHARE_PTHREADS_H
#define	_DISTSHARE_PTHREADS_H

extern "C" {

    void* VDRIVE_DistShare_runTest(void* args);

    struct VDRIVE_DistShare_runTest_parm {
        uint64_t dist_id;
        uint16_t thread_id;
    };
    
}
#endif	/* _DISTSHARE_PTHREADS_H */

