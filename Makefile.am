ACLOCAL_AMFLAGS = -I m4

## Define a libtool archive target "libdadisi-@API_VERSION@.la", with
## @API_VERSION@ substituted into the generated Makefile at configure
## time.
## The libtool archive file (.la) will be installed into the directory 
## named by the predefined variable $(bindir), along with the actual 
## shared library file (.so).
lib_LTLIBRARIES=libdadisi-@API_VERSION@.la

libdadisi_@API_VERSION@_la_CPPFLAGS = $(GCRYPT_CPPFLAGS) $(GMP3_CPPFLAGS) $(XERCES_CPPFLAGS) $(SQLITE3_CPPFLAGS) $(BOOST_CPPFLAGS)

libdadisi_@API_VERSION@_la_CXXFLAGS = -fPIC  -MMD -MP -MF

## Instruct libtool to include ABI version information in the generated shared
## library file (.so).  The library ABI version is defined in configure.ac, so
## that all version information is kept in one place.
libdadisi_@API_VERSION@_la_LDFLAGS = -version-info $(SO_VERSION)

libdadisi_@API_VERSION@_la_LIBDADD = $(GCRYPT_LIBS) $(GMP3_LIBS) $(XERCES_LIBS) $(SQLITE3_LIBS) -pthread


dadisi_includedir=$(includedir)/libdadisi-$(API_VERSION)
nobase_dadisi_include_HEADERS = dadisi/Disk.h                     \
                                dadisi/DistCRUSH.h                \
                                dadisi/DistFastRedundantShare.h   \
                                dadisi/DistNearestNeighbour.h     \
                                dadisi/DistRedundantShare.h       \
                                dadisi/DistRoundRobin.h           \
                                dadisi/DistShare.h                \
                                dadisi/DistShare_pThreads.h       \
                                dadisi/Distributor.h              \
                                dadisi/ImprovedMap.h              \
                                dadisi/RSCache.h                  \
                                dadisi/RSDisk.h                   \
                                dadisi/SHA.h                      \
                                dadisi/ShareDisk.h                \
                                dadisi/helper.h

libdadisi_@API_VERSION@_la_SOURCES = dadisi/Builder.cpp                  \
                                     dadisi/Builder.h                    \
                                     dadisi/Prime.cpp                    \
                                     dadisi/Prime.h                      \
                                     dadisi/crush.cpp                    \
                                     dadisi/crush.h                      \
                                     dadisi/crushwrapper.h               \
                                     dadisi/grammar.h                    \
                                     dadisi/hash.cpp                     \
                                     dadisi/hash.h                       \
                                     dadisi/mapper.cpp                   \
                                     dadisi/mapper.h                     \
                                     dadisi/test.cpp                     \
                                     dadisi/types.h                      \
                                     dadisi/Disk.cpp                     \
                                     dadisi/DistCRUSH.cpp                \
                                     dadisi/DistFastRedundantShare.cpp   \
                                     dadisi/DistNearestNeighbour.cpp     \
                                     dadisi/DistRandSlice.h              \
                                     dadisi/DistRandSlice.cpp            \
                                     dadisi/DistRUSHp.cpp                \
                                     dadisi/DistRUSHp.h                  \
                                     dadisi/DistRUSHr.cpp                \
                                     dadisi/DistRUSHr.h                  \
                                     dadisi/DistRUSHt.cpp                \
                                     dadisi/DistRUSHt.h                  \
                                     dadisi/DistRedundantShare.cpp       \
                                     dadisi/DistRoundRobin.cpp           \
                                     dadisi/DistShare.cpp                \
                                     dadisi/Distributor.cpp              \
                                     dadisi/flat_segment_tree.hpp        \
                                     dadisi/flat_segment_tree_node.hpp   \
                                     dadisi/ImprovedMap.cpp              \
                                     dadisi/RSCache.cpp                  \
                                     dadisi/RSDisk.cpp                   \
                                     dadisi/SHA.cpp                      \
                                     dadisi/helper.cpp

bin_PROGRAMS = CPPDistTest CPPDistCompare fileDistTester test_rand_slice_grow

CPPFLAGS_TESTS = -Wall -Wno-deprecated -funroll-loops $(libdadisi_@API_VERSION@_la_CPPFLAGS)
LDADD_TESTS = -ldadisi-$(API_VERSION) $(libdadisi_@API_VERSION@_la_LIBDADD)

CPPDistTest_CPPFLAGS = $(CPPFLAGS_TESTS)
CPPDistTest_SOURCES = perf_eval/test.cpp       \
                      perf_eval/test.h         \
                      perf_eval/test_private.h
CPPDistTest_LDADD = $(LDADD_TESTS)


CPPDistCompare_CPPFLAGS = $(CPPFLAGS_TESTS)
CPPDistCompare_SOURCES = perf_eval/compare.cpp \
                         perf_eval/compare.h
CPPDistCompare_LDADD = $(LDADD_TESTS)

fileDistTester_CPPFLAGS = $(CPPFLAGS_TESTS)
fileDistTester_SOURCES = perf_eval/fileDistTester.cpp
fileDistTester_LDADD = $(LDADD_TESTS)

test_rand_slice_grow_CPPFLAGS = $(CPPFLAGS_TESTS)
test_rand_slice_grow_SOURCES = perf_eval/test_rand_slice_grow.cpp
test_rand_slice_grow_LDADD = $(LDADD_TESTS)

                           
dist_noinst_SCRIPTS = autogen.sh

EXTRA_DIST = perf_eval/results.tar.bz2 doc
