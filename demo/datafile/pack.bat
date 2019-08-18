@echo off
set LIME_UTIL=..\..\lime\bin\x64\Release\lime.exe
set RESOURCE_MANIFEST=resources.manifest
set OUTPUT_FILE=demo.dat
if not exist %LIME_UTIL% (
	echo. && echo Can't locate the Lime utility! Did you forget to build it?
) else (
	%LIME_UTIL% %RESOURCE_MANIFEST% %OUTPUT_FILE% -clevel=9 -head="Lime Demo" -chksum=adler32
)
