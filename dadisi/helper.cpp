#include <cstdio>
#include <sqlite3.h>

#include"helper.h"

int64_t readInt64_t(const char* value, int8_t base) {
    char* start = (char*) value;
    int64_t result = 0;
    char *last;
    bool neg = false;

    while (((*start) == ' ') && (*start <= 0))
        start++;
    if (*start == 0) {
        throw 1;
    }

    if (start[0] == '-') {
        neg = true;
        start++;
    }

    if (base == 0) {
        if (start[0] == '0') {
            if (start[1] == 'x') {
                base = 16;
                start += 2;
            } else {
                base = 8;
                start += 1;
            }
        } else {
            base = 10;
        }
    }

    switch (base) {
        case 2:
            last = start;
            while ((*last >= '0') && (*last <= '1')) {
                last--;
                result <<= 1;
                result += (*last - '0');
                last++;
            }
            break;
        case 8:
            last = start;
            while ((*last >= '0') && (*last <= '7')) {
                result <<= 3;
                result += (*last - '0');
                last++;
            }
            break;
        case 10:
            last = start;
            while ((*last >= '0') && (*last <= '9')) {
                result *= 10;
                result += (*last - '0');
                last++;
            }
            break;
        case 16:
            last = start;
            while (((*last >= '0') && (*last <= '9')) || ((*last >= 'a') && (*last <= 'f')) || ((*last >= 'A') && (*last <= 'F'))) {
                result <<= 4;
                if ((*last >= '0') && (*last <= '9')) {
                    result += (*last - '0');
                }
                if ((*last >= 'a') && (*last <= 'f')) {
                    result += (*last - 'a' + 10);
                }
                if ((*last >= 'A') && (*last <= 'F')) {
                    result += (*last - 'A' + 10);
                }
                last++;
            }
            break;
    }

    if (neg)
        result *= -1;
    return result;
}

uint64_t readUint64_t(const char* value, int8_t base) {
    char* start = (char*) value;
    uint64_t result = 0;
    char *last;

    while (((*start) == ' ') && (*start <= 0))
        start++;
    if (*start == 0) {
        throw 1;
    }

    if (start[0] == '-') {
        throw 1;
    }

    if (base == 0) {
        if (start[0] == '0') {
            if (start[1] == 'x') {
                base = 16;
                start += 2;
            } else {
                base = 8;
                start += 1;
            }
        } else {
            base = 10;
        }
    }

    switch (base) {
        case 2:
            last = start;
            while ((*last >= '0') && (*last <= '1')) {
                last--;
                result <<= 1;
                result += (*last - '0');
                last++;
            }
            break;
        case 8:
            last = start;
            while ((*last >= '0') && (*last <= '7')) {
                result <<= 3;
                result += (*last - '0');
                last++;
            }
            break;
        case 10:
            last = start;
            while ((*last >= '0') && (*last <= '9')) {
                result *= 10;
                result += (*last - '0');
                last++;
            }
            break;
        case 16:
            last = start;
            while (((*last >= '0') && (*last <= '9')) || ((*last >= 'a') && (*last <= 'f')) || ((*last >= 'A') && (*last <= 'F'))) {
                result <<= 4;
                if ((*last >= '0') && (*last <= '9')) {
                    result += (*last - '0');
                }
                if ((*last >= 'a') && (*last <= 'f')) {
                    result += (*last - 'a' + 10);
                }
                if ((*last >= 'A') && (*last <= 'F')) {
                    result += (*last - 'A' + 10);
                }
                last++;
            }
            break;
    }

    return result;
}

int64_t read_int64_t(mpq_class value) {
    return readInt64_t(value.get_str(10).c_str(), 10);
}

uint64_t read_uint64_t(mpz_class value) {
    return readUint64_t(value.get_str(10).c_str(), 10);
}

mpz_class create_mpz(int64_t num) {
    uint64_t tmp = num;
    tmp = tmp >> 32;
    uint32_t tmp32 = (uint32_t) tmp;
    mpz_class z(tmp32);
    z = z << 32;
    tmp = num;
    tmp = tmp << 32;
    tmp = tmp >> 32;
    tmp32 = (int32_t) tmp;
    z = z + tmp32;
    return z;
}

mpq_class create_mpq(int64_t num, int64_t den) {
    mpq_class q(create_mpz(num), create_mpz(den));
    return q;
}

mpq_class create_mpq_n(int64_t num) {
    mpq_class q(create_mpz(num));
    return q;
}

#ifndef no_sqlite
sqlite3 *loadDB(std::string filename) {
    sqlite3 *db;
    int error;

    error = sqlite3_open(filename.c_str(), &db); // Use :memory: as filename to get an inmemory database.
    if (error) {
        fprintf(stderr, "Could not open database file: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        throw "Could not open database file";
    }

    return db;
}

void execQuery(sqlite3 *db, std::string queryStr) {
    sqlite3_stmt *query;
    int error;

    error = sqlite3_prepare(db, queryStr.c_str(), -1, &query, 0);
    if (error) {
        fprintf(stderr, "Could not create query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(query);
        throw "Could not create query";
    }

    error = sqlite3_step(query);
    if (error != SQLITE_DONE) {
        fprintf(stderr, "Could not execute first query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(query);
        throw "Could not execute first query";
    }

    sqlite3_finalize(query);
}

sqlite3 *createDB(std::string filename) {
    sqlite3 *db = loadDB(filename);
    execQuery(db, "create table meta (key text, value text)");
    return db;
}
#endif
