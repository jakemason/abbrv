# forces cmakelists to refresh because we use GLOB
cd ../

# IMPORTANT:
# Make sure cl.exe is in your path as such:
# C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.14.26428\bin\Hostx64\x64
#
# #  Also, make sure ninja.exe is in your path as such:
# C:\Ninja

#Ninja Build:
(cmake . -G Ninja -DCMAKE_CXX_COMPILER="cl.exe" -DCMAKE_C_COMPILER="cl.exe" || $SHELL)

# MSVC Build Equivalent:
# (cmake . -DCMAKE_GENERATOR_PLATFORM=x64)
