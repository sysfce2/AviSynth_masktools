md release
cd release
md with_dual_filter_signatures
cd with_dual_filter_signatures
md x64
md x64_xp
md x86
md x86_xp
cd ..
md x64
md x64_clang
md x64_xp
md x86
md x86_clang
md x86_xp
cd ..

xcopy /y x64\build\release-boost\masktools2.dll release\x64\
xcopy /y x64\build\release-LLVM-clangCL\masktools2.dll release\x64_clang\
xcopy /y x64\build\release-no-boost\masktools2.dll release\x64_xp\
xcopy /y Win32\build\release-boost\masktools2.dll release\x86\
xcopy /y Win32\build\release-LLVM-clangCL\masktools2.dll release\x86_clang\
xcopy /y Win32\build\release-no-boost\masktools2.dll release\x86_xp\
xcopy /y x64\build\release-boost-dualsign\masktools2.dll release\with_dual_filter_signatures\x64\
xcopy /y x64\build\release-no-boost-dualsign\masktools2.dll release\with_dual_filter_signatures\x64_xp\
xcopy /y Win32\build\release-boost-dualsign\masktools2.dll release\with_dual_filter_signatures\x86\
xcopy /y Win32\build\release-no-boost-dualsign\masktools2.dll release\with_dual_filter_signatures\x86_xp\