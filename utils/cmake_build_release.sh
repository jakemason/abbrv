cd ../
# Copy over our libraries to the final directory
mkdir -p abbrv
# cp ./bin/zlib1.dll ./abbrv/.
# cp ./bin/SDL2.dll ./abbrv/.
# cp ./bin/glew32.dll ./abbrv/.


# NOTE: Set our cmake options flags, these are NOT the same as the definitions in
# C++, despite having the same name. Refer to either CMakeLists.txt for more
# information.
cmake -DEDITOR_MODE=OFF -DENABLE_DEBUG=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1
(cmake --build . || $SHELL) | tee cgame.txt 
