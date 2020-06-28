rm -r build-win
cmake -G "Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=${1:-Debug} -DSDL2_PATH:STRING="$PWD/vendor/SDL2-2.0.12" -DSDL2_TTF_PATH:STRING="$PWD/vendor/SDL2_ttf-2.0.15" -DGLEW_LOCATION="$PWD/vendor/glew-1.13.0" -A Win32 -S . -B build-win
