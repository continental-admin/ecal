@echo off

pushd %~dp0

if not exist "_build\sdk" mkdir "_build\sdk"

set "CMAKE_PREFIX_PATH=C:/Qt/6.6.1/msvc2019_64"

pushd %~dp0\_build\sdk
cmake ../.. -G "Visual Studio 17 2022" -T v142 -A x64^
        -DHAS_HDF5=ON ^
        -DHAS_QT=ON ^
        -DHAS_CURL=OFF ^
        -DHAS_CAPNPROTO=OFF ^
        -DHAS_FTXUI=ON ^
        -DBUILD_DOCS=OFF ^
        -DBUILD_APPS=OFF ^
        -DBUILD_SAMPLES=OFF ^
        -DBUILD_TIME=ON ^
        -DBUILD_PY_BINDING=OFF ^
        -DBUILD_CSHARP_BINDING=OFF ^
        -DBUILD_ECAL_TESTS=OFF ^
        -DECAL_INCLUDE_PY_SAMPLES=OFF ^
        -DECAL_INSTALL_SAMPLE_SOURCES=OFF ^
        -DECAL_JOIN_MULTICAST_TWICE=OFF ^
        -DECAL_NPCAP_SUPPORT=ON ^
        -DECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS=ON ^
        -DECAL_THIRDPARTY_BUILD_PROTOBUF=ON ^
        -DECAL_THIRDPARTY_BUILD_SPDLOG=ON ^
        -DECAL_THIRDPARTY_BUILD_TINYXML2=ON ^
        -DECAL_THIRDPARTY_BUILD_FINEFTP=OFF ^
        -DECAL_THIRDPARTY_BUILD_CURL=OFF ^
        -DECAL_THIRDPARTY_BUILD_GTEST=OFF ^
        -DECAL_THIRDPARTY_BUILD_HDF5=ON ^
        -DECAL_THIRDPARTY_BUILD_RECYCLE=ON ^
        -DECAL_THIRDPARTY_BUILD_TCP_PUBSUB=ON ^
        -DECAL_THIRDPARTY_BUILD_QWT=OFF ^
        -DECAL_THIRDPARTY_BUILD_YAML-CPP=OFF ^
        -DECAL_THIRDPARTY_BUILD_UDPCAP=ON ^
        -DBUILD_SHARED_LIBS=OFF ^
        -DCPACK_PACK_WITH_INNOSETUP=OFF
popd 

popd

pause