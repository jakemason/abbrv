cd ../
# NOTE: Set our cmake options flags, these are NOT the same as the definitions in
# C++, despite having the same name. Refer to either CMakeLists.txt for more
# information.
cmake -DEDITOR_MODE=ON -DENABLE_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1
(cmake --build . || $SHELL) | tee cgame.txt 
