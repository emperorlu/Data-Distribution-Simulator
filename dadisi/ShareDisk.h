/* 
 * File:   ShareDisk.h
 * Author: fermat
 *
 * Created on 26. März 2010, 17:59
 */

#ifndef _SHAREDISK_H
#define	_SHAREDISK_H

#include<stdint.h>

namespace VDRIVE {

    class ShareDisk {
    public:
        ShareDisk(const ShareDisk& orig) {
            this->id = orig.id;
            this->end = orig.end;
            this->rounds = orig.end;
            this->start = orig.start;
        };

        virtual ~ShareDisk() {
        };

        ShareDisk(int64_t id, uint64_t start, uint64_t end, int32_t rounds, int32_t copy) {
            this->id = id;
            this->start = start;
            this->rounds = rounds;
            this->end = end;
            this->copy = copy;
        }

        int64_t getID() {
            return id;
        }

        int64_t getStart() {
            return start;
        }

        int64_t getEnd() {
            return end;
        }

        int32_t getCopy() {
            return copy;
        }

        bool coversPoint(uint64_t point) {
            if (rounds > 0)
                return true;
            if (start < end) {
                if ((point > start) && (point < end))
                    return true;
                return false;
            }
            if (point < end)
                return true;
            if (point > start)
                return true;
            return false;
        }

        int32_t countCoversPoint(uint64_t point) {
            if (start < end) {
                if ((point > start) && (point < end))
                    return rounds + 1;
                return rounds;
            }
            if (point < end)
                return rounds + 1;
            if (point > start)
                return rounds + 1;
            return rounds;
        }
    private:
        int64_t id;
        uint64_t start;
        uint64_t end;
        int32_t rounds;
        int32_t copy;
    };
}
#endif	/* _SHAREDISK_H */

