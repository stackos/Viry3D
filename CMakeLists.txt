cmake_minimum_required(VERSION 3.7.2)

project(Project)

get_filename_component(VIRY3D_LIB_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/lib/src
                       ABSOLUTE)

get_filename_component(VIRY3D_APP_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/app/src
                       ABSOLUTE)

function(string_ends_with str suffix var)
    string(LENGTH "${str}" str_length)
    string(LENGTH "${suffix}" suffix_length)
    set(value FALSE)
    if (NOT ${str_length} LESS ${suffix_length})
        math(EXPR str_offset "${str_length} - ${suffix_length}")
        string(SUBSTRING "${str}" ${str_offset} ${suffix_length} str_suffix)
        if (str_suffix STREQUAL suffix)
            set(value TRUE)
        endif ()
    endif ()
    set(${var} ${value} PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE VIRY3D_ASSETS ${VIRY3D_APP_SRC_DIR}/../bin/Assets/*.*)
foreach (item ${VIRY3D_ASSETS})
    string_ends_with(${item} ".cache" is_cache)
    if (${is_cache})
        list(REMOVE_ITEM VIRY3D_ASSETS ${item})
    endif ()
endforeach ()

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -DFT2_BUILD_LIBRARY -DAL_LIBTYPE_STATIC -DAL_ALEXT_PROTOTYPES -DFPM_DEFAULT -DSIZEOF_INT=4")

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -DFT2_BUILD_LIBRARY -DAL_LIBTYPE_STATIC -DAL_ALEXT_PROTOTYPES -DFPM_DEFAULT -DSIZEOF_INT=4")

file(GLOB VIRY3D_DEP_SRCS
     ${VIRY3D_LIB_SRC_DIR}/crypto/md5/md5.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/autofit/autofit.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftbase.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftbbox.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftbitmap.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftdebug.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftfntfmt.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftfstype.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftgasp.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftglyph.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftgxval.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftinit.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftlcdfil.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftmm.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftotval.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftpatent.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftpfr.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftstroke.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftsynth.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftsystem.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/fttype1.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/base/ftwinfnt.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/bdf/bdf.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/cache/ftcache.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/cff/cff.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/cid/type1cid.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/gzip/ftgzip.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/lzw/ftlzw.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/pcf/pcf.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/pfr/pfr.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/psaux/psaux.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/pshinter/pshinter.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/psnames/psmodule.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/raster/raster.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/sfnt/sfnt.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/smooth/smooth.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/truetype/truetype.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/type1/type1.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/type42/type42.c
     ${VIRY3D_LIB_SRC_DIR}/freetype/src/winfonts/winfnt.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jaricom.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcapimin.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcapistd.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcarith.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jccoefct.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jccolor.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcdctmgr.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jchuff.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcinit.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcmainct.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcmarker.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcmaster.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcomapi.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcparam.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcprepct.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jcsample.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jctrans.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdapimin.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdapistd.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdarith.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdatadst.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdatasrc.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdcoefct.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdcolor.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jddctmgr.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdhuff.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdinput.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdmainct.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdmarker.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdmaster.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdmerge.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdpostct.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdsample.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jdtrans.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jerror.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jfdctflt.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jfdctfst.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jfdctint.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jidctflt.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jidctfst.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jidctint.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jmemmgr.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jmemnobs.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jquant1.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jquant2.c
     ${VIRY3D_LIB_SRC_DIR}/jpeg/jutils.c
     ${VIRY3D_LIB_SRC_DIR}/jsoncpp/src/lib_json/json_reader.cpp
     ${VIRY3D_LIB_SRC_DIR}/jsoncpp/src/lib_json/json_value.cpp
     ${VIRY3D_LIB_SRC_DIR}/jsoncpp/src/lib_json/json_writer.cpp
     ${VIRY3D_LIB_SRC_DIR}/png/png.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngerror.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngget.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngmem.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngpread.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngread.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngrio.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngrtran.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngrutil.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngset.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngtrans.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngwio.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngwrite.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngwtran.c
     ${VIRY3D_LIB_SRC_DIR}/png/pngwutil.c
     ${VIRY3D_LIB_SRC_DIR}/xml/tinyxml2.cpp
     ${VIRY3D_LIB_SRC_DIR}/zlib/ioapi.c
     ${VIRY3D_LIB_SRC_DIR}/zlib/unzip.c
     )

source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS})

add_library(Viry3DDep STATIC ${VIRY3D_DEP_SRCS})

target_include_directories(Viry3DDep PRIVATE
                           ${VIRY3D_LIB_SRC_DIR}
                           ${VIRY3D_LIB_SRC_DIR}/freetype/include
                           ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                           )

file(GLOB VIRY3D_LIB_SRCS
     ${VIRY3D_LIB_SRC_DIR}/*.h
     ${VIRY3D_LIB_SRC_DIR}/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/2d/*.h
     ${VIRY3D_LIB_SRC_DIR}/2d/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/animation/*.h
     ${VIRY3D_LIB_SRC_DIR}/animation/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/audio/*.h
     ${VIRY3D_LIB_SRC_DIR}/audio/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/graphics/*.h
     ${VIRY3D_LIB_SRC_DIR}/graphics/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/io/*.h
     ${VIRY3D_LIB_SRC_DIR}/io/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/math/*.h
     ${VIRY3D_LIB_SRC_DIR}/math/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/memory/*.h
     ${VIRY3D_LIB_SRC_DIR}/memory/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/string/*.h
     ${VIRY3D_LIB_SRC_DIR}/string/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/thread/*.h
     ${VIRY3D_LIB_SRC_DIR}/thread/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/time/*.h
     ${VIRY3D_LIB_SRC_DIR}/time/*.cpp
     ${VIRY3D_LIB_SRC_DIR}/ui/*.h
     ${VIRY3D_LIB_SRC_DIR}/ui/*.cpp
     )

source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_LIB_SRCS})

add_library(Viry3D STATIC ${VIRY3D_LIB_SRCS})

target_include_directories(Viry3D PRIVATE
                           ${VIRY3D_LIB_SRC_DIR}
                           ${VIRY3D_LIB_SRC_DIR}/freetype/include
                           ${VIRY3D_LIB_SRC_DIR}/mp3/mad
                           ${VIRY3D_LIB_SRC_DIR}/openal/include
                           ${VIRY3D_LIB_SRC_DIR}/openal/android
                           ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/Include
                           ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src
                           ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                           )

file(GLOB VIRY3D_APP_SRCS
     ${VIRY3D_APP_SRC_DIR}/*.h
     ${VIRY3D_APP_SRC_DIR}/*.cpp
     )

source_group(TREE ${VIRY3D_APP_SRC_DIR}/.. FILES ${VIRY3D_APP_SRCS})

# lua
if ( FALSE )

    file(GLOB VIRY3D_DEP_SRCS_LUA
         ${VIRY3D_LIB_SRC_DIR}/lua/lpeg/lpcap.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lpeg/lpcode.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lpeg/lpprint.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lpeg/lptree.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lpeg/lpvm.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lapi.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lauxlib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lbaselib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lbitlib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lcode.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lcorolib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lctype.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ldblib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ldebug.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ldo.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ldump.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lfunc.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lgc.c
         ${VIRY3D_LIB_SRC_DIR}/lua/linit.c
         ${VIRY3D_LIB_SRC_DIR}/lua/liolib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/llex.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lmathlib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lmem.c
         ${VIRY3D_LIB_SRC_DIR}/lua/loadlib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lobject.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lopcodes.c
         ${VIRY3D_LIB_SRC_DIR}/lua/loslib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lparser.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lstate.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lstring.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lstrlib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ltable.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ltablib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/ltm.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lundump.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lutf8lib.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lvm.c
         ${VIRY3D_LIB_SRC_DIR}/lua/lzio.c
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_LUA})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_LUA})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/lua
                               )

endif ()

# noise
if ( FALSE )

    file(GLOB VIRY3D_DEP_SRCS_NOISE
         ${VIRY3D_LIB_SRC_DIR}/noise/model/cylinder.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/model/line.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/model/plane.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/model/sphere.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/abs.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/add.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/billow.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/blend.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/cache.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/checkerboard.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/clamp.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/const.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/curve.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/cylinders.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/displace.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/exponent.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/invert.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/max.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/min.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/modulebase.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/multiply.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/perlin.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/power.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/ridgedmulti.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/rotatepoint.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/scalebias.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/scalepoint.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/select.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/spheres.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/terrace.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/translatepoint.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/turbulence.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/module/voronoi.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/latlon.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/noisegen.cpp
         ${VIRY3D_LIB_SRC_DIR}/noise/noiseutils.cpp
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_NOISE})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_NOISE})

endif ()

# physics
if ( FALSE )

    file(GLOB VIRY3D_DEP_SRCS_PHYSICS
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btDbvt.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btGhostObject.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/btUnionFind.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btBox2dShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btBoxShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btCollisionShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btCompoundShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConcaveShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConeShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvex2dShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvexShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btCylinderShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btEmptyShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btShapeHull.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btSphereShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTriangleBuffer.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/Character/btKinematicCharacterController.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btFixedConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btGearConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/Dynamics/btRigidBody.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btAlignedAllocator.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btConvexHull.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btConvexHullComputer.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btGeometryUtil.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btPolarDecomposition.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btQuickprof.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btSerializer.cpp
         ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src/LinearMath/btVector3.cpp
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_PHYSICS})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_PHYSICS})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/physics/bullet/src
                               )

endif ()

# zlib
if (${Target} MATCHES "Windows" OR
    ${Target} MATCHES "UWP" OR
    ${Target} MATCHES "WASM"
    )

    file(GLOB VIRY3D_DEP_SRCS_ZLIB
         ${VIRY3D_LIB_SRC_DIR}/zlib/adler32.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/compress.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/crc32.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/deflate.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/gzclose.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/gzlib.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/gzread.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/gzwrite.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/infback.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/inffast.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/inflate.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/inftrees.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/trees.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/uncompr.c
         ${VIRY3D_LIB_SRC_DIR}/zlib/zutil.c
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_ZLIB})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_ZLIB})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/zlib
                               )

endif ()

# mp3
if (${Target} MATCHES "Windows" OR
    ${Target} MATCHES "UWP" OR
    ${Target} MATCHES "Android" OR
    ${Target} MATCHES "Mac" OR
    ${Target} MATCHES "iOS"
    )

    file(GLOB VIRY3D_DEP_SRCS_MP3
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/bit.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/decoder.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/fixed.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/frame.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/huffman.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/compat.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/crc.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/field.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/frametype.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/genre.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/id3_debug.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/id3_frame.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/id3_version.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/latin1.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/parse.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/render.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/tag.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/ucs4.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/utf8.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/utf16.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/id3tag/util.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/layer3.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/layer12.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/mad_stream.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/mad_timer.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/synth.c
         ${VIRY3D_LIB_SRC_DIR}/mp3/mad/version.c
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_MP3})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_MP3})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/mp3/mad
                               )

endif ()

# openal
if (${Target} MATCHES "Windows" OR ${Target} MATCHES "UWP" OR ${Target} MATCHES "Android")

    file(GLOB VIRY3D_DEP_SRCS_OPENAL
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/backends/loopback.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/backends/null.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/backends/thread_msg_queue_cpp11.cpp
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/ALc.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/ALc_cpp11.cpp
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcConfig.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcDedicated.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcEcho.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcModulator.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcReverb.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcRing.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcThread.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/ALu.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/bs2b.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/helpers.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/hrtf.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/mixer.c
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/panning.c
         ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/alError.c
         ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/alExtension.c
         ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/alListener.c
         ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/alSource.c
         ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/alThunk.c
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_OPENAL})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_OPENAL})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/openal/include
                               ${VIRY3D_LIB_SRC_DIR}/openal/OpenAL32/Include
                               )

endif ()

# spirv_cross
if (${Target} MATCHES "Windows" OR ${Target} MATCHES "Android" OR ${Target} MATCHES "Mac" OR ${Target} MATCHES "iOS")

    file(GLOB VIRY3D_DEP_SRCS_SPIRV_CROSS
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_cfg.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_cpp.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_cross.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_cross_util.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_glsl.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_hlsl.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/spirv_cross/spirv_msl.cpp
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_SPIRV_CROSS})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_SPIRV_CROSS})

endif ()

# vulkan shader compiler
if (${Target} MATCHES "Windows" OR ${Target} MATCHES "Android")

    file(GLOB VIRY3D_LIB_SRCS_VULKAN
         ${VIRY3D_LIB_SRC_DIR}/vulkan/*.h
         ${VIRY3D_LIB_SRC_DIR}/vulkan/*.cpp
         )

    target_sources(Viry3D PRIVATE ${VIRY3D_LIB_SRCS_VULKAN})

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_LIB_SRCS_VULKAN})

endif ()

if (${Target} MATCHES "Windows")

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -DVR_WINDOWS -DVR_VULKAN=1 -DVR_GLES=0 -DVK_USE_PLATFORM_WIN32_KHR")
    set(EXECUTABLE_OUTPUT_PATH
        ${PROJECT_BINARY_DIR}/bin)
    add_definitions(-W3 -D_CRT_SECURE_NO_WARNINGS -D_CRT_RAND_S -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES -DGLEW_STATIC)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

    file(GLOB VIRY3D_DEP_SRCS_WIN
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/backends/xaudio2.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/GenericCodeGen/CodeGen.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/GenericCodeGen/Link.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/attribute.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/Constant.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/glslang_tab.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/InfoSink.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/Initialize.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/Intermediate.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/intermOut.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/IntermTraverse.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/iomapper.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/limits.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/linkValidate.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/parseConst.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/ParseContextBase.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/ParseHelper.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/PoolAlloc.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/propagateNoContraction.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/reflection.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/RemoveTree.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/Scan.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/ShaderLang.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/SymbolTable.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/MachineIndependent/Versions.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/glslang/OSDependent/Windows/ossource.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/OGLCompilersDLL/InitializeDll.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/disassemble.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/doc.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/GlslangToSpv.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/InReadableOrder.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/Logger.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/SpvBuilder.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/SpvPostProcess.cpp
         ${VIRY3D_LIB_SRC_DIR}/vulkan/glslang/SPIRV/SPVRemapper.cpp
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_WIN})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_WIN})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/openal/win
                               )

    target_include_directories(Viry3D PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               ${VIRY3D_LIB_SRC_DIR}/zlib
                               )

    source_group(src FILES ${VIRY3D_APP_SRC_DIR}/../project/win/Main.cpp)

    add_executable(Viry3DApp
                   ${VIRY3D_APP_SRCS}
                   ${VIRY3D_APP_SRC_DIR}/../project/win/Main.cpp
                   )

    target_include_directories(Viry3DApp PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_APP_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               )

    target_link_libraries(Viry3DApp
                          Viry3D Viry3DDep
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/vulkan_sdk/lib/${Arch}/vulkan-1.lib
                          opengl32.lib
                          winmm.lib
                          Xaudio2.lib
                          )

    set_property(TARGET Viry3DApp PROPERTY LINK_FLAGS "/SUBSYSTEM:WINDOWS")

    string(REPLACE "/" "\\" ASSETS_LINK_TARGET ${CMAKE_SOURCE_DIR}/app/bin/Assets)
    string(REPLACE "/" "\\" ASSETS_LINK_LINK ${EXECUTABLE_OUTPUT_PATH}/$(Configuration)/Assets)
    string(REPLACE "/" "\\" ICON_SRC ${CMAKE_SOURCE_DIR}/app/bin/icon.ico)
    string(REPLACE "/" "\\" ICON_DST ${EXECUTABLE_OUTPUT_PATH}/$(Configuration)/icon.ico)
    add_custom_command(TARGET Viry3DApp
                       POST_BUILD
                       COMMAND if not exist ${ASSETS_LINK_LINK} (mklink /d ${ASSETS_LINK_LINK} ${ASSETS_LINK_TARGET})
                       COMMAND copy /Y ${ICON_SRC} ${ICON_DST}
                       )

    add_executable(CubeMapToSphericalPolynomial
                   ${VIRY3D_APP_SRC_DIR}/../project/CubeMapToSphericalPolynomial/CubeMapToSphericalPolynomial.cpp
                   )

    target_include_directories(CubeMapToSphericalPolynomial PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               )

    target_link_libraries(CubeMapToSphericalPolynomial
                          Viry3D Viry3DDep
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/vulkan_sdk/lib/${Arch}/vulkan-1.lib
                          winmm.lib
                          Xaudio2.lib
                          )

    add_executable(CubeMapCompress
                   ${VIRY3D_APP_SRC_DIR}/../project/CubeMapCompress/CubeMapCompress.cpp
                   )

    target_include_directories(CubeMapCompress PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               ${VIRY3D_APP_SRC_DIR}/../project/CubeMapCompress/Compressonator/include
                               )

    target_link_libraries(CubeMapCompress
                          Viry3D Viry3DDep
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/vulkan_sdk/lib/${Arch}/vulkan-1.lib
                          winmm.lib
                          Xaudio2.lib
                          ${VIRY3D_APP_SRC_DIR}/../project/CubeMapCompress/Compressonator/lib/VS2015/${Arch}/Compressonator_MD_DLL.lib
                          )

    string(REPLACE "/" "\\" COMP_DLL_SRC ${VIRY3D_APP_SRC_DIR}/../project/CubeMapCompress/Compressonator/lib/VS2015/${Arch}/Compressonator_MD_DLL.dll)
    string(REPLACE "/" "\\" COMP_DLL_DST ${EXECUTABLE_OUTPUT_PATH}/$(Configuration)/Compressonator_MD_DLL.dll)
    add_custom_command(TARGET CubeMapCompress
                       POST_BUILD
                       COMMAND copy /Y ${COMP_DLL_SRC} ${COMP_DLL_DST}
                       )

elseif (${Target} MATCHES "UWP")

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -DVR_UWP -DVR_VULKAN=0 -DVR_GLES=1")
    add_definitions(-W3 -D_CRT_SECURE_NO_WARNINGS -D_CRT_RAND_S -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES -DGLEW_STATIC)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

    file(GLOB VIRY3D_DEP_SRCS_UWP
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/backends/xaudio2.cpp
         ${VIRY3D_LIB_SRC_DIR}/openal/Alc/alcThreadCpp11.cpp
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_DEP_SRCS_UWP})

    target_sources(Viry3DDep PRIVATE ${VIRY3D_DEP_SRCS_UWP})

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/openal/win
                               )

    target_include_directories(Viry3D PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/zlib
                               ${VIRY3D_LIB_SRC_DIR}/gles/angle/include
                               )

    set(UWP_ASSETS
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/LockScreenLogo.scale-200.png
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/SplashScreen.scale-200.png
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/Square44x44Logo.scale-200.png
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/Square44x44Logo.targetsize-24_altform-unplated.png
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/Square150x150Logo.scale-200.png
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/StoreLogo.png
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Assets/Wide310x150Logo.scale-200.png
        )
    set(UWP_CONTENT
        ${VIRY3D_ASSETS}
        ${UWP_ASSETS}
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/Package.appxmanifest
        ${VIRY3D_LIB_SRC_DIR}/gles/angle/lib/${Arch}/libEGL.dll
        ${VIRY3D_LIB_SRC_DIR}/gles/angle/lib/${Arch}/libGLESv2.dll
        )
    set(UWP_RESOURCE_FILES
        ${UWP_CONTENT}
        ${VIRY3D_APP_SRC_DIR}/../project/uwp/app_TemporaryKey.pfx)

    foreach (item ${VIRY3D_ASSETS})
        get_filename_component(dir ${item} DIRECTORY)
        string(LENGTH ${VIRY3D_APP_SRC_DIR}/../bin/ root_length)
        string(SUBSTRING ${dir} ${root_length} -1 local_dir)
        set_property(SOURCE ${item} PROPERTY VS_DEPLOYMENT_LOCATION ${local_dir})
    endforeach ()
    set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
    set_property(SOURCE ${UWP_CONTENT} PROPERTY VS_DEPLOYMENT_CONTENT 1)
    source_group("Resource Files" FILES ${UWP_RESOURCE_FILES})
    source_group(TREE ${VIRY3D_APP_SRC_DIR}/../bin FILES ${VIRY3D_ASSETS})
    source_group(TREE ${VIRY3D_APP_SRC_DIR}/../project/uwp FILES ${UWP_ASSETS})

    source_group(src FILES ${VIRY3D_APP_SRC_DIR}/../project/uwp/Main.cpp)

    add_executable(Viry3DApp
                   ${VIRY3D_APP_SRCS}
                   ${VIRY3D_APP_SRC_DIR}/../project/uwp/Main.cpp
                   ${UWP_RESOURCE_FILES}
                   )

    target_include_directories(Viry3DApp PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_LIB_SRC_DIR}/gles/angle/include
                               ${VIRY3D_APP_SRC_DIR}
                               )

    target_link_libraries(Viry3DApp
                          Viry3D Viry3DDep
                          ${VIRY3D_LIB_SRC_DIR}/gles/angle/lib/${Arch}/libEGL.lib
                          ${VIRY3D_LIB_SRC_DIR}/gles/angle/lib/${Arch}/libGLESv2.lib
                          mincore.lib
                          )

    set_property(TARGET Viry3DApp PROPERTY LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_property(TARGET Viry3DApp PROPERTY VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.10240.0")
    set_property(TARGET Viry3DApp PROPERTY VS_WINRT_COMPONENT TRUE)

elseif (${Target} MATCHES "Android")

    get_filename_component(SHADERC_SRC
                           ${ANDROID_NDK}/sources/third_party/shaderc
                           ABSOLUTE)
    set(GLSLANG_SPIRV_INCLUDE_DIR ${SHADERC_SRC}/third_party/glslang)
    add_library(shaderc_lib STATIC IMPORTED)
    set_target_properties(shaderc_lib PROPERTIES IMPORTED_LOCATION
                          ${SHADERC_SRC}/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

    set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -DVR_ANDROID -DVR_VULKAN=1 -DVR_GLES=0 -DVK_USE_PLATFORM_ANDROID_KHR -fexceptions -frtti")
    add_definitions(-Wall -DIOAPI_NO_64 -DHAVE_GCC_DESTRUCTOR)

    target_sources(Viry3DDep PRIVATE
                   ${VIRY3D_LIB_SRC_DIR}/openal/Alc/backends/opensl.c
                   )

    target_include_directories(Viry3DDep PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/openal/android
                               )

    target_sources(Viry3D PRIVATE
                   ${VIRY3D_LIB_SRC_DIR}/vulkan/vulkan_wrapper/vulkan_wrapper.cpp
                   )

    target_include_directories(Viry3D PRIVATE
                               ${ANDROID_NDK}/sources/third_party/shaderc/include
                               ${SHADERC_SRC}/third_party/spirv-tools/include
                               ${SHADERC_SRC}/third_party/spirv-tools/include/spirv-tools
                               )

    add_library(Viry3DApp SHARED
                ${VIRY3D_APP_SRCS}
                ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
                ${CMAKE_SOURCE_DIR}/app/project/android/app/src/main/jni/jni.cpp
                )

    target_include_directories(Viry3DApp PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_APP_SRC_DIR}
                               ${ANDROID_NDK}/sources/android/native_app_glue
                               )

    target_link_libraries(Viry3DApp
                          Viry3D Viry3DDep
                          android log z OpenSLES EGL GLESv3
                          shaderc_lib       # for vulkan shader compile
                          )

    # build shaderc
    # ndk-bundle/sources/third_party/shaderc
    # ../../../ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk APP_STL:=c++_shared APP_ABI=armeabi-v7a APP_PLATFORM=android-18 libshaderc_combined

elseif (${Target} MATCHES "Mac")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -DVR_MAC -DVR_VULKAN=1 -DVR_GLES=0")
    add_definitions(-Wall -DHAVE_ASSERT_H -fobjc-arc)

    set_source_files_properties(${VIRY3D_LIB_SRC_DIR}/Debug.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${VIRY3D_LIB_SRC_DIR}/Application.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

    target_include_directories(Viry3D PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/MoltenVKShaderConverter/MoltenVKGLSLToSPIRVConverter/macOS/MoltenVKGLSLToSPIRVConverter.framework/Headers
                               )

    set_source_files_properties(${VIRY3D_APP_SRC_DIR}/../project/mac/app/Assets.xcassets
                                ${VIRY3D_APP_SRC_DIR}/../bin/Assets
                                PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources
                                )

    add_executable(Viry3DApp
                   ${VIRY3D_APP_SRCS}
                   ${VIRY3D_APP_SRC_DIR}/../project/mac/Main.m
                   ${VIRY3D_APP_SRC_DIR}/../project/mac/AppDelegate.m
                   ${VIRY3D_APP_SRC_DIR}/../project/mac/ViewController.mm
                   ${VIRY3D_APP_SRC_DIR}/../project/mac/VkView.m
                   ${VIRY3D_APP_SRC_DIR}/../project/mac/app/Assets.xcassets
                   ${VIRY3D_APP_SRC_DIR}/../bin/Assets
                   )

    target_include_directories(Viry3DApp PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_APP_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               )

    target_link_libraries(Viry3DApp
                          Viry3D Viry3DDep
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/macOS/libMoltenVK.a
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/MoltenVKShaderConverter/MoltenVKGLSLToSPIRVConverter/macOS/MoltenVKGLSLToSPIRVConverter.framework
                          "-framework OpenAL"
                          "-framework AppKit"
                          "-framework Metal"
                          "-framework QuartzCore"
                          "-framework IOKit"
                          z.1.2.8
                          )

    set_target_properties(Viry3DApp PROPERTIES
                          MACOSX_BUNDLE TRUE
                          MACOSX_BUNDLE_INFO_PLIST ${VIRY3D_APP_SRC_DIR}/../project/mac/app/Info.plist
                          XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
                          )

elseif (${Target} MATCHES "iOS")

    set(XCODE_IOS_PLATFORM iphoneos)
    execute_process(COMMAND xcodebuild -version -sdk ${XCODE_IOS_PLATFORM} Path
        OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "found ios sdk: ${CMAKE_OSX_SYSROOT}")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "")
    set(CMAKE_OSX_ARCHITECTURES arm64)

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -DVR_IOS")
    add_definitions(-Wall -DVR_VULKAN=1 -DVR_GLES=0 -DHAVE_ASSERT_H -fobjc-arc)

    set_source_files_properties(${VIRY3D_LIB_SRC_DIR}/Debug.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${VIRY3D_LIB_SRC_DIR}/Application.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${VIRY3D_LIB_SRC_DIR}/ios/ARScene.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")

    file(GLOB VIRY3D_LIB_SRCS_IOS
         ${VIRY3D_LIB_SRC_DIR}/ios/*.h
         ${VIRY3D_LIB_SRC_DIR}/ios/*.mm
         )

    source_group(TREE ${VIRY3D_LIB_SRC_DIR}/.. FILES ${VIRY3D_LIB_SRCS_IOS})

    target_sources(Viry3D PRIVATE ${VIRY3D_LIB_SRCS_IOS})

    target_include_directories(Viry3D PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/MoltenVKShaderConverter/MoltenVKGLSLToSPIRVConverter/iOS/MoltenVKGLSLToSPIRVConverter.framework/Headers
                               )

    set_source_files_properties(${VIRY3D_APP_SRC_DIR}/../project/ios/app/Assets.xcassets
                                ${VIRY3D_APP_SRC_DIR}/../project/ios/app/Base.lproj/LaunchScreen.storyboard
                                ${VIRY3D_APP_SRC_DIR}/../bin/Assets
                                PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources
                                )

    add_executable(Viry3DApp
                   ${VIRY3D_APP_SRCS}
                   ${VIRY3D_APP_SRC_DIR}/../project/ios/Main.m
                   ${VIRY3D_APP_SRC_DIR}/../project/ios/AppDelegate.m
                   ${VIRY3D_APP_SRC_DIR}/../project/ios/ViewController.mm
                   ${VIRY3D_APP_SRC_DIR}/../project/ios/VkView.m
                   ${VIRY3D_APP_SRC_DIR}/../project/ios/app/Assets.xcassets
                   ${VIRY3D_APP_SRC_DIR}/../project/ios/app/Base.lproj/LaunchScreen.storyboard
                   ${VIRY3D_APP_SRC_DIR}/../bin/Assets
                   )

    target_include_directories(Viry3DApp PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_APP_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/include
                               )

    target_link_libraries(Viry3DApp
                          Viry3D Viry3DDep
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/iOS/libMoltenVK.a
                          ${VIRY3D_LIB_SRC_DIR}/vulkan/MoltenVK/MoltenVKShaderConverter/MoltenVKGLSLToSPIRVConverter/iOS/MoltenVKGLSLToSPIRVConverter.framework
                          "-framework OpenAL"
                          "-framework UIKit"
                          "-framework Metal"
                          "-framework QuartzCore"
                          "-framework IOKit"
                          "-framework ARKit"
                          "-framework CoreVideo"
                          z.1.2.8
                          )

    set_target_properties(Viry3DApp PROPERTIES
                          MACOSX_BUNDLE TRUE
                          MACOSX_BUNDLE_INFO_PLIST ${VIRY3D_APP_SRC_DIR}/../project/ios/app/Info.plist
                          XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
                          )

elseif (${Target} MATCHES "WASM")

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -DVR_WASM -DVR_VULKAN=0 -DVR_GLES=1 -std=c++11")
    add_definitions(-Wall -s WASM=1 -O3)

    target_include_directories(Viry3D PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}/zlib
                               )

    add_library(Viry3DApp SHARED
                ${VIRY3D_APP_SRCS}
                ${VIRY3D_APP_SRC_DIR}/../project/web/Main.cpp
                )

    target_include_directories(Viry3DApp PRIVATE
                               ${VIRY3D_LIB_SRC_DIR}
                               ${VIRY3D_LIB_SRC_DIR}/jsoncpp/include
                               ${VIRY3D_APP_SRC_DIR}
                               )

    target_link_libraries(Viry3DApp
                          Viry3D Viry3DDep
                          )

endif ()
