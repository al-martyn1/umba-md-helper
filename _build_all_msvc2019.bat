@call ~call_cmake_msvc_impl.bat msvc2019 x86 GENERATE
@call ~call_cmake_msvc_impl.bat msvc2019 x64 GENERATE
@rem 
@call ~call_cmake_msvc_impl.bat msvc2019 x86 BUILD
@call ~call_cmake_msvc_impl.bat msvc2019 x64 BUILD
